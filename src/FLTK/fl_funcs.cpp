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


void fill_cfg_widgets(void)
{
    int i;

    int bitrate[] = { 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112,
                    128, 160, 192, 224, 256, 320, 0 };

  
    //fill the main section
    for(i = 0; i < cfg.audio.dev_count; i++)
        fl_g->choice_cfg_dev->add(cfg.audio.pcm_list[i]->name);

    fl_g->choice_cfg_dev->value(cfg.audio.dev_num);

    fl_g->choice_cfg_act_srv->clear();
    fl_g->choice_cfg_act_srv->redraw();
    for(i = 0; i < cfg.main.num_of_srv; i++)
        fl_g->choice_cfg_act_srv->add(cfg.srv[i]->name);

    if(cfg.main.num_of_srv > 0)
    {
        fl_g->button_cfg_edit_srv->activate();
        fl_g->button_cfg_del_srv->activate();
    }
    else
    {
        fl_g->button_cfg_edit_srv->deactivate();
        fl_g->button_cfg_del_srv->deactivate();
    }

    fl_g->choice_cfg_act_icy->clear();
    fl_g->choice_cfg_act_icy->redraw();
    for(i = 0; i < cfg.main.num_of_icy; i++)
        fl_g->choice_cfg_act_icy->add(cfg.icy[i]->name);

    if(cfg.main.num_of_icy > 0)
    {
        fl_g->button_cfg_edit_icy->activate();
        fl_g->button_cfg_del_icy->activate();
    }
    else
    {
        fl_g->button_cfg_edit_icy->deactivate();
        fl_g->button_cfg_del_icy->deactivate();
    }

    fl_g->choice_cfg_act_srv->value(cfg.selected_srv);
    fl_g->choice_cfg_act_icy->value(cfg.selected_icy);


    fl_g->check_cfg_connect->value(cfg.main.connect_at_startup);
    fl_g->input_log_filename->value(cfg.main.log_file);

    //fill the audio (stream) section
    if(!strcmp(cfg.audio.codec, "mp3"))
        fl_g->choice_cfg_codec->value(CHOICE_MP3);
    else if(!strcmp(cfg.audio.codec, "ogg"))
        fl_g->choice_cfg_codec->value(CHOICE_OGG);
    else if(!strcmp(cfg.audio.codec, "opus"))
        fl_g->choice_cfg_codec->value(CHOICE_OPUS);
    else if(!strcmp(cfg.audio.codec, "aac+"))
        fl_g->choice_cfg_codec->value(3);


    if(cfg.audio.channel == 1)
        fl_g->choice_cfg_channel->value(CHOICE_MONO);
    else
        fl_g->choice_cfg_channel->value(CHOICE_STEREO);

    for(i = 0; bitrate[i] != 0; i++)
        if(cfg.audio.bitrate == bitrate[i])
        {
            fl_g->choice_cfg_bitrate->value(i);
            break;
        }

    fl_g->input_cfg_buffer->value(cfg.audio.buffer_ms);

    fl_g->choice_cfg_resample_mode->value(cfg.audio.resample_mode);
    

    if(cfg.main.song_update)
        fl_g->check_song_update_active->value(1);
    else
        fl_g->check_song_update_active->value(0);

    fl_g->input_cfg_song_file->value(cfg.main.song_path);
    
    fl_g->use_app->value(cfg.main.app_update);
    fl_g->choice_app->value(cfg.main.app_update_service);

    //fill the record section
    fl_g->input_rec_filename->value(cfg.rec.filename);
    fl_g->input_rec_folder->value(cfg.rec.folder);
    fl_g->input_rec_split_time->value(cfg.rec.split_time);

    if(!strcmp(cfg.rec.codec, "mp3"))
        fl_g->choice_rec_codec->value(CHOICE_MP3);
    else if(!strcmp(cfg.rec.codec, "ogg"))
        fl_g->choice_rec_codec->value(CHOICE_OGG);
    else if(!strcmp(cfg.rec.codec, "opus"))
        fl_g->choice_rec_codec->value(CHOICE_OPUS);
    else if(!strcmp(cfg.rec.codec, "flac"))
    {
        fl_g->choice_rec_codec->value(CHOICE_FLAC);
        fl_g->choice_rec_bitrate->deactivate();
    }
    else if(!strcmp(cfg.rec.codec, "wav"))
    {
        fl_g->choice_rec_codec->value(CHOICE_WAV);
        fl_g->choice_rec_bitrate->deactivate();
    } else { //aac-aac+
        fl_g->choice_rec_codec->value(CHOICE_WAV + 1);
    }

    for(i = 0; bitrate[i] != 0; i++)
        if(cfg.rec.bitrate == bitrate[i])
            fl_g->choice_rec_bitrate->value(i);

    if(cfg.rec.start_rec)
        fl_g->check_cfg_rec->value(1);
    else
        fl_g->check_cfg_rec->value(0);

    if(cfg.rec.sync_to_hour)
        fl_g->check_sync_to_full_hour->value(1);
    else
        fl_g->check_sync_to_full_hour->value(0);


    update_samplerates();

    //fill the GUI section
    fl_g->button_gui_bg_color->color(cfg.main.bg_color,
            fl_lighter((Fl_Color)cfg.main.bg_color));
    fl_g->button_gui_text_color->color(cfg.main.txt_color,
            fl_lighter((Fl_Color)cfg.main.txt_color));
    fl_g->check_gui_attach->value(cfg.gui.attach);
    fl_g->check_gui_ontop->value(cfg.gui.ontop);
    if(cfg.gui.ontop)
    {
        fl_g->window_main->stay_on_top(1);
        fl_g->window_cfg->stay_on_top(1);
    }
    fl_g->check_gui_lcd_auto->value(cfg.gui.lcd_auto);


    //fill the DSP section
    fl_g->activateCompressorCheckButton->value(cfg.dsp.compressor);
    fl_g->compressorQuantitySlider->value(1.0f - cfg.dsp.compQuantity);

    extern char slider_compressor_label[];
    snprintf(slider_compressor_label, 100, "%s (%.0f%%)", _("Cantidad de compresión"), (1.0f - cfg.dsp.compQuantity) * 100);
    fl_g->compressorQuantitySlider->label(slider_compressor_label);
}

//Updates the samplerate drop down menu for the audio
//device the user has selected
void update_samplerates(void)
{
    int i;
    int *sr_list;
    char sr_asc[10];

    fl_g->choice_cfg_samplerate->clear();

    sr_list = cfg.audio.pcm_list[cfg.audio.dev_num]->sr_list;

    for(i = 0; sr_list[i] != 0; i++)
    {
        snprintf(sr_asc, 10, "%dHz", sr_list[i]);
        fl_g->choice_cfg_samplerate->add(sr_asc);

        if(cfg.audio.samplerate == sr_list[i])
            fl_g->choice_cfg_samplerate->value(i);
    }
    if(i == 0)
    {
        fl_g->choice_cfg_samplerate->add(_("dev. not supported"));
        fl_g->choice_cfg_samplerate->value(0);
    }
}

void print_info(const char* info, int info_type)
{
    char timebuf[10];
    char logtimestamp[21];
    char* infotxt;
    FILE *log_fd;
    size_t len;

    time_t test;
    struct tm  *mytime;
    static struct tm time_bak;

    infotxt = strdup(info);


    test = time(NULL);
    mytime = localtime(&test);

    if( (time_bak.tm_min != mytime->tm_min) || (time_bak.tm_hour != mytime->tm_hour) )
    {
        time_bak.tm_min = mytime->tm_min;
        time_bak.tm_hour = mytime->tm_hour;
        strftime(timebuf, sizeof(timebuf), "\n%H:%M:", mytime);
        fl_g->info_buffer->append(timebuf);
    }

    fl_g->info_buffer->append((const char*)"\n");
    fl_g->info_buffer->append((const char*)info);

    //always scroll to the last line
    fl_g->info_output->scroll(fl_g->info_buffer->count_lines(0,     //count the lines from char 0 to the last character
                            fl_g->info_buffer->length()),           //returns the number of characters in the buffer
                            0);

    // log to log_file if defined
    if ((cfg.main.log_file != NULL) && (strlen(cfg.main.log_file) > 0))
    {
        strftime(logtimestamp, sizeof(logtimestamp), "%Y-%m-%d %H:%M:%S", mytime);
        log_fd = fopen(cfg.main.log_file, "ab");
        if (log_fd != NULL) 
        {
            // Probably the most confusing part of the whole code ;)
            
            if(strchr(infotxt, ':'))
                strrpl(&infotxt, (char*)"\n", (char*)", ", MODE_ALL);
            else
                strrpl(&infotxt, (char*)"\n", (char*)" ", MODE_ALL);

            strrpl(&infotxt, (char*)":,", (char*)": ", MODE_ALL);
            strrpl(&infotxt, (char*)"\t", (char*)"", MODE_ALL);

            len = strlen(infotxt)-1;
            // remove trailing commas and spaces 
            while (infotxt[len] == ',' || infotxt[len] == ' ')
            {
                infotxt[len--] = '\0';
            }

            fprintf(log_fd, "%s %s\n", logtimestamp, infotxt);
            fclose(log_fd);
        }

    }
    
    free(infotxt);
}

void print_lcd(const char *text, int home, int clear)
{
    if(clear)
        fl_g->lcd->clear();

    fl_g->lcd->print((const uchar*) text, (int) strlen(text));

    if(home)
        fl_g->lcd->cursor_pos(0);
}


void expand_string(char **str)
{
    char m[3], d[3], y[5], hh[3], mi[3], ss[3];
    struct tm *date;
    const time_t t = time(NULL);

    date = localtime(&t);

    snprintf(ss,  3, "%02d", date->tm_sec);
    snprintf(mi,  3, "%02d", date->tm_min);
    snprintf(hh,  3, "%02d", date->tm_hour);
    snprintf(d,   3, "%02d", date->tm_mday);
    snprintf(m,   3, "%02d", date->tm_mon+1);
    snprintf(y,   5, "%d",   date->tm_year+1900);

    strrpl(str, (char*)"%S", ss, MODE_ALL);
    strrpl(str, (char*)"%M", mi, MODE_ALL);
    strrpl(str, (char*)"%H", hh, MODE_ALL);
    strrpl(str, (char*)"%d", d, MODE_ALL);
    strrpl(str, (char*)"%m", m, MODE_ALL);
    strrpl(str, (char*)"%y", y, MODE_ALL);
    strrpl(str, (char*)"%Y", y, MODE_ALL); // %Y and %y both work as year
}

void test_file_extension(void)
{
    char *current_ext;

    current_ext = util_get_file_extension(cfg.rec.filename);

    // Append extension i
    if(current_ext == NULL)
    {
        cfg.rec.filename = (char*)realloc(cfg.rec.filename, strlen(cfg.rec.filename)+strlen(cfg.rec.codec)+2);
        strcat(cfg.rec.filename, ".");
        strcat(cfg.rec.filename, cfg.rec.codec);
        fl_g->input_rec_filename->value(cfg.rec.filename);
    }
    // Replace extension
    else if(strcmp(current_ext, cfg.rec.codec))
    {
        strrpl(&cfg.rec.filename, current_ext, cfg.rec.codec, MODE_LAST);
        fl_g->input_rec_filename->value(cfg.rec.filename);
    }
}



void init_main_gui_and_audio(void)
{
    fl_g->slider_gain->value(util_factor_to_db(cfg.main.gain));
    fl_g->window_main->redraw();

    if(cfg.gui.ontop)
        fl_g->window_main->stay_on_top(1);

    lame_stream.channel = cfg.audio.channel;
    lame_stream.bitrate = cfg.audio.bitrate;
    lame_stream.samplerate = cfg.audio.samplerate;
    lame_enc_reinit(&lame_stream);

    lame_rec.channel = cfg.audio.channel;
    lame_rec.bitrate = cfg.rec.bitrate;
    lame_rec.samplerate = cfg.audio.samplerate;
    lame_enc_reinit(&lame_rec);

    vorbis_stream.channel = cfg.audio.channel;
    vorbis_stream.bitrate = cfg.audio.bitrate;
    vorbis_stream.samplerate = cfg.audio.samplerate;
    vorbis_enc_reinit(&vorbis_stream);

    vorbis_rec.channel = cfg.audio.channel;
    vorbis_rec.bitrate = cfg.rec.bitrate;
    vorbis_rec.samplerate = cfg.audio.samplerate;
    vorbis_enc_reinit(&vorbis_rec);
    
    opus_stream.channel = cfg.audio.channel;
    opus_stream.bitrate = cfg.audio.bitrate*1000;
    opus_stream.samplerate = cfg.audio.samplerate;
    opus_enc_reinit(&opus_stream);
    
    opus_rec.channel = cfg.audio.channel;
    opus_rec.bitrate = cfg.rec.bitrate*1000;
    opus_rec.samplerate = cfg.audio.samplerate;
    opus_enc_reinit(&opus_rec);

    flac_rec.channel = cfg.audio.channel;
    flac_rec.samplerate = cfg.audio.samplerate;
    flac_enc_reinit(&flac_rec);

    aacplus_stream.channels = cfg.audio.channel;
    aacplus_stream.bitrate = cfg.audio.bitrate * 1000;//(cfg.audio.bitrate > 64 ? 64 : cfg.audio.bitrate) * 1000;
    aacplus_stream.samplerate = cfg.audio.samplerate;
    aac_enc_reinit(&aacplus_stream);

    aacplus_rec.channels = cfg.audio.channel;
    aacplus_rec.bitrate = cfg.rec.bitrate * 1000;
    aacplus_rec.samplerate = cfg.audio.samplerate;
    aac_enc_reinit(&aacplus_rec);
}

void init_choice_app(void* ptr) {
    flgui* fl_g = (flgui*) ptr;
#if __APPLE__ && __MACH__
    fl_g->choice_app->add("iTunes", 0, (Fl_Callback*) &change_app_cb);
    fl_g->choice_app->add("Spotify", 0, (Fl_Callback*) &change_app_cb);
    fl_g->choice_app->add("VOX", 0, (Fl_Callback*) &change_app_cb);
#endif
}
