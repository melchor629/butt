// FLTK GUI related functions
//
// Copyright 2007-2008 by Daniel Noethen.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/stat.h>

#ifndef _WIN32
 #include <sys/wait.h>
#endif

#include "config.h"

#include "cfg.h"
#include "butt.h"
#include "util.h"
#include "port_audio.h"
#include "timer.h"
#include "flgui.h"
#include "fl_funcs.h"
#include "shoutcast.h"
#include "icecast.h"
#include "strfuncs.h"
#include "fl_timer_funcs.h"

#if __APPLE__ && __MACH__
#include "CurrentTrackOSX.h"
#endif

const char* (*current_track_app)(void);

void vu_meter_timer(void*)
{
    if(pa_new_frames)
        snd_update_vu();

    Fl::repeat_timeout(0.01, &vu_meter_timer);
}

void display_info_timer(void*)
{
    char lcd_text_buf[33];

    if(try_to_connect == 1)
    {
        Fl::repeat_timeout(0.1, &display_info_timer);
        return;
    }

    if(display_info == SENT_DATA)
    {
        sprintf(lcd_text_buf, _("Sent\n%0.2lfMB"),
                kbytes_sent / 1024);
        print_lcd(lcd_text_buf, 0, 1);
    }

    if(display_info == STREAM_TIME && timer_is_elapsed(&stream_timer))
    {
        sprintf(lcd_text_buf, _("Time\n%s"),
                timer_get_time_str(&stream_timer));
        print_lcd(lcd_text_buf, 0, 1);
    }

    if(display_info == REC_TIME && timer_is_elapsed(&rec_timer))
    {
        sprintf(lcd_text_buf, _("Time recording\n%s"),
                timer_get_time_str(&rec_timer));
        print_lcd(lcd_text_buf, 0, 1);
    }

    if(display_info == REC_DATA)
    {
        sprintf(lcd_text_buf, _("Size recorded\n%0.2lfMB"),
                kbytes_written / 1024);
        print_lcd(lcd_text_buf, 0, 1);
    }

    Fl::repeat_timeout(0.1, &display_info_timer);
}

void display_rotate_timer(void*) 
{

    if(!connected && !recording)
        goto exit;

    if (!cfg.gui.lcd_auto)
        goto exit;

    switch(display_info)
    {
        case STREAM_TIME:
            display_info = SENT_DATA;
            break;
        case SENT_DATA:
            if(recording)
                display_info = REC_TIME;
            else
                display_info = STREAM_TIME;
            break;
        case REC_TIME:
            display_info = REC_DATA;
            break;
        case REC_DATA:
            if(connected)
                display_info = STREAM_TIME;
            else
                display_info = REC_TIME;
            break;
        default:
            break;
    }

exit:
    Fl::repeat_timeout(5, &display_rotate_timer);

}

void is_connected_timer(void*)
{
    if(!connected)
    {
        print_info(_("ERROR: Connexion lost\nReconnecting..."), 1);
        if(cfg.srv[cfg.selected_srv]->type == SHOUTCAST)
            sc_disconnect();
        else
            ic_disconnect();

        Fl::remove_timeout(&display_info_timer);
        Fl::remove_timeout(&is_connected_timer);

        //reconnect
        button_connect_cb();

        return;
    }

    Fl::repeat_timeout(0.5, &is_connected_timer);
}

void cfg_win_pos_timer(void*)
{

#ifdef _WIN32
    fl_g->window_cfg->position(fl_g->window_main->x() +
                                fl_g->window_main->w()+15,
                                fl_g->window_main->y());
#else //UNIX
    fl_g->window_cfg->position(fl_g->window_main->x() +
                                fl_g->window_main->w(),
                                fl_g->window_main->y());
#endif

    Fl::repeat_timeout(0.1, &cfg_win_pos_timer);
}

void split_recording_timer(void *mode)
{
    int i;
    int button_clicked = 0;
    static int with_repeat = 0;
    char *insert_pos;
    char *path;
    char *ext;
    char file_num_str[12];
    char *path_for_index_loop;
    struct tm *local_time;
    const time_t t = time(NULL);

    if(recording == 0)
        return;

    if(*((int*)mode) == 1)
        with_repeat = 1;
    else
        button_clicked = 1;

    // Values < 0 are not allowed
    if(fl_g->input_rec_split_time->value() < 0)
    {
        fl_g->input_rec_split_time->value(0);
        return;
    }

    path = strdup(cfg.rec.path_fmt);
    expand_string(&path);
    ext = util_get_file_extension(cfg.rec.filename);
    if(ext == NULL)
    {
        print_info(_("Cannot detect file extension\n"
                "Splitting file is deactivated"), 0);
        free(path);
        return;
    }


    path_for_index_loop = strdup(path);
    
    //check if the file already exists
    if((next_fd = fl_fopen(path, "rb")) != NULL)
    {
        fclose(next_fd);
        
        //increment the index until we find a filename that doesn't exist yet
        for(i = 1; /*inf*/; i++) // index_loop
        {
            
            free(path);
            path = strdup(path_for_index_loop);
            
            //find begin of the file extension
            insert_pos = strrstr(path, ext);
            
            //Put index between end of file name end beginning of extension
            snprintf(file_num_str, sizeof(file_num_str), "-%d", i);
            strinsrt(&path, file_num_str, insert_pos-1);
            
            if((next_fd = fl_fopen(path, "rb")) == NULL)
                break; // found valid file name

            if(i == 0x7FFFFFFF) // 2^31-1
            {
                free(path);
                free(path_for_index_loop);
                print_info(_("Could not find a valid filename for next file"
                             "\nbutt keeps recording to current file"), 0);
                return;
            }
        }
    }

    if((next_fd = fl_fopen(path, "wb")) == NULL)
    {
        fl_alert(_("Could not open:\n%s"), path);
        free(path);
        return;
    }

    print_info(_("Recording to: "), 0);
    print_info(path, 0);

    next_file = 1;
    free(path);

    if(with_repeat == 1 && button_clicked == 0)
    {
        local_time = localtime(&t);

        // Make sure that the 60 minutes boundary is not violated in case sync_to_hour == 1
        if((cfg.rec.sync_to_hour == 1) && ((local_time->tm_min + cfg.rec.split_time) > 60))
            Fl::repeat_timeout(60*(60 - local_time->tm_min), &split_recording_timer, &with_repeat);
        else
            Fl::repeat_timeout(60*cfg.rec.split_time, &split_recording_timer, &with_repeat);
    }

}

void songfile_timer(void*)
{
    long len;
    char song[501];
    char msg[100];
    float repeat_time = 1;

    struct stat s;
    static time_t old_t;


    if(cfg.main.song_path == NULL)
        goto exit;

    if(stat(cfg.main.song_path, &s) != 0)
    {

        // File was probably locked by another application
        // retry in 5 seconds
        repeat_time = 5;
        goto exit;
    }

    if(old_t == s.st_mtime) //file hasn't changed
        goto exit;

   if((cfg.main.song_fd = fl_fopen(cfg.main.song_path, "rb")) == NULL)
   {
	   snprintf(msg, sizeof(msg), _("Could not open: %s.\nWill try in 5s"),
					   cfg.main.song_path); 

       print_info(msg, 1);
       repeat_time = 5;
       goto exit;
   }

    old_t = s.st_mtime;

   if(fgets(song, 500, cfg.main.song_fd) != NULL)
   {
       len = strlen(song);
       //remove newline character
       if(song[len-2] == '\r') // Windows
           song[len-2] = '\0';
       else if(song[len-1] == '\n') // OSX, Linux
           song[len-1] = '\0';

       if(cfg.main.song == NULL || strcmp(cfg.main.song, song)) {
           cfg.main.song = (char*) realloc(cfg.main.song, strlen(song) +1);
           strcpy(cfg.main.song, song);
           button_cfg_song_go_cb();
       }
   }

   fclose(cfg.main.song_fd);

exit:
    Fl::repeat_timeout(repeat_time, &songfile_timer);
}

void app_timer(void*) {
    if(!app_timeout_running) {
        Fl::remove_timeout(&app_timer);
        return;
    }

    if(current_track_app != NULL) {
        const char* track = current_track_app();
        if(track) {
            if(cfg.main.song == NULL || strcmp(cfg.main.song, track)) {
                cfg.main.song = (char*) realloc(cfg.main.song, strlen(track) + 1);
                strcpy(cfg.main.song, track);
                button_cfg_song_go_cb();
            }
            free((void*) track);
        } else {
            if(cfg.main.song != NULL && strcmp(cfg.main.song, "")) {
                cfg.main.song = (char*) realloc(cfg.main.song, 1);
                strcpy(cfg.main.song, "");
                button_cfg_song_go_cb();
            }
        }
    }

    Fl::repeat_timeout(5, &app_timer);
}
