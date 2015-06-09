// config functions for butt
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

#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "config.h"

#include "cfg.h"
#include "butt.h"
#include "flgui.h"
#include "util.h"
#include "fl_funcs.h"

#ifdef _WIN32
 const char CONFIG_FILE[] = "buttrc";
#else
 const char CONFIG_FILE[] = ".buttrc";
#endif

config_t cfg;
char *cfg_path;
bool unsaved_changes;

int cfg_write_file(char *path)
{
    int i;
    FILE *cfg_fd;
    char info_buf[256];

    if(path == NULL)
        path = cfg_path;

    cfg_fd = fl_fopen(path, "wb");
    if(cfg_fd == NULL)
    {
        snprintf(info_buf, sizeof(info_buf), "Could not write to file: %s", path);
        print_info(cfg_path, 1);
        return 1;
    }

    fprintf(cfg_fd, "This is a configuration file for butt (broadcast using this tool)\n\n");
    fprintf(cfg_fd, "[main]\n");

    fprintf(cfg_fd, "bg_color = %d\n", cfg.main.bg_color);
    fprintf(cfg_fd, "txt_color = %d\n", cfg.main.txt_color);

    if(cfg.main.num_of_srv > 0)
        fprintf(cfg_fd, 
                "server = %s\n"
                "srv_ent = %s\n",
                cfg.main.srv,
                cfg.main.srv_ent
               );
    else
        fprintf(cfg_fd, 
                "server = \n"
                "srv_ent = \n"
               );

    if(cfg.main.num_of_icy > 0)
        fprintf(cfg_fd, 
                "icy = %s\n"
                "icy_ent = %s\n",
                cfg.main.icy,
                cfg.main.icy_ent
               );
    else
        fprintf(cfg_fd, 
                "icy = \n"
                "icy_ent = \n"
               );

    fprintf(cfg_fd,
            "num_of_srv = %d\n"
            "num_of_icy = %d\n",
            cfg.main.num_of_srv,
            cfg.main.num_of_icy
           );

    if(cfg.main.song_path != NULL)
        fprintf(cfg_fd, "song_path = %s\n", cfg.main.song_path);
    else
        fprintf(cfg_fd, "song_path = \n");

    fprintf(cfg_fd, "song_update = %d\n", cfg.main.song_update);

    fprintf(cfg_fd, "gain = %f\n", cfg.main.gain);

    fprintf(cfg_fd, "connect_at_startup = %d\n", cfg.main.connect_at_startup);

    if (cfg.main.log_file != NULL)
        fprintf(cfg_fd, "log_file = %s\n\n", cfg.main.log_file);
    else
        fprintf(cfg_fd, "log_file = \n\n");

    fprintf(cfg_fd,
            "[audio]\n"
            "device = %d\n"
            "samplerate = %d\n"
            "bitrate = %d\n"
            "channel = %d\n"
            "codec = %s\n"
            "resample_mode = %d\n"
            "buffer_ms = %d\n\n",
            cfg.audio.dev_num,
            cfg.audio.samplerate,
            cfg.audio.bitrate,
            cfg.audio.channel,
            cfg.audio.codec,
            cfg.audio.resample_mode,
            cfg.audio.buffer_ms
           );

    fprintf(cfg_fd, 
            "[record]\n"
            "bitrate = %d\n"
            "codec = %s\n"
            "start_rec = %d\n"
            "sync_to_hour  = %d\n"
            "split_time = %d\n"
            "filename = %s\n"
            "folder = %s\n\n",
            cfg.rec.bitrate,
            cfg.rec.codec,
            cfg.rec.start_rec,
            cfg.rec.sync_to_hour,
            cfg.rec.split_time,
            cfg.rec.filename,
            cfg.rec.folder
           );

    fprintf(cfg_fd,
            "[gui]\n"
            "attach = %d\n"
            "ontop = %d\n"
            "lcd_auto = %d\n\n",
            cfg.gui.attach,
            cfg.gui.ontop,
            cfg.gui.lcd_auto
           );

    for(i = 0; i < cfg.main.num_of_srv; i++)
    {
        fprintf(cfg_fd, 
                "[%s]\n"
                "address = %s\n"
                "port = %u\n"
                "password = %s\n"
                "type = %d\n",
                cfg.srv[i]->name,
                cfg.srv[i]->addr,
                cfg.srv[i]->port,
                cfg.srv[i]->pwd,
                cfg.srv[i]->type
               );

        if(cfg.srv[i]->type == ICECAST)
        {
            fprintf(cfg_fd, "mount = %s\n", cfg.srv[i]->mount);
            fprintf(cfg_fd, "usr = %s\n\n", cfg.srv[i]->usr);
        }
        else //Shoutcast has no mountpoint and user
        {
            fprintf(cfg_fd, "mount = (none)\n");
            fprintf(cfg_fd, "usr = (none)\n\n");
        }

    }

    for(i = 0; i < cfg.main.num_of_icy; i++)
    {
        fprintf(cfg_fd,
                "[%s]\n"
                "pub = %s\n",
                cfg.icy[i]->name,
                cfg.icy[i]->pub
               );

        if(cfg.icy[i]->desc != NULL)
            fprintf(cfg_fd, "description = %s\n", cfg.icy[i]->desc);
        else
            fprintf(cfg_fd, "description = \n");

        if(cfg.icy[i]->genre != NULL)
            fprintf(cfg_fd, "genre = %s\n", cfg.icy[i]->genre);
        else
            fprintf(cfg_fd, "genre = \n");

        if(cfg.icy[i]->url != NULL)
            fprintf(cfg_fd, "url = %s\n", cfg.icy[i]->url);
        else
            fprintf(cfg_fd, "url = \n");

        if(cfg.icy[i]->irc != NULL)
            fprintf(cfg_fd, "irc = %s\n", cfg.icy[i]->irc);
        else
            fprintf(cfg_fd, "irc = \n");

        if(cfg.icy[i]->icq != NULL)
            fprintf(cfg_fd, "icq = %s\n", cfg.icy[i]->icq);
        else
            fprintf(cfg_fd, "icq = \n");

        if(cfg.icy[i]->aim != NULL)
            fprintf(cfg_fd, "aim = %s\n\n", cfg.icy[i]->aim);
        else
            fprintf(cfg_fd, "aim = \n\n");

    }

    fclose(cfg_fd);

    snprintf(info_buf, sizeof(info_buf), "Config written to %s", path);
    print_info(info_buf, 0);

    unsaved_changes = 0;

    return 0;
}

int cfg_set_values(char *path)
{
    int i;
    char *srv_ent;
    char *icy_ent;
    char *strtok_buf = NULL;
    
    if(path == NULL)
        path = cfg_path;

    if(cfg_parse_file(path) == -1)
        return 1;

    unsaved_changes = 0;

    cfg.main.log_file    = cfg_get_str("main", "log_file");

    cfg.audio.dev_num    = cfg_get_int("audio", "device");
    cfg.audio.samplerate = cfg_get_int("audio", "samplerate");
    cfg.audio.resolution = 16;
    cfg.audio.bitrate    = cfg_get_int("audio", "bitrate");
    cfg.audio.channel    = cfg_get_int("audio", "channel");
    cfg.audio.codec      = cfg_get_str("audio", "codec");
    cfg.audio.buffer_ms  = cfg_get_int("audio", "buffer_ms");
    cfg.audio.pcm_list   = snd_get_devices(&cfg.audio.dev_count);

    cfg.audio.resample_mode  = cfg_get_int("audio", "resample_mode");
    
    if(cfg.audio.dev_num < 0)
        cfg.audio.dev_num = 0;

    if(cfg.audio.samplerate == -1)
        cfg.audio.samplerate = 44100;

    if(cfg.audio.bitrate == -1)
        cfg.audio.bitrate = 128;

    if(cfg.audio.channel == -1)
        cfg.audio.channel = 2;


    if(cfg.audio.codec == NULL)
    {
        cfg.audio.codec = (char*)malloc(5*sizeof(char));
        strcpy(cfg.audio.codec, "mp3");
    }
    else //Make sure that also "opus" and "flac" fits into the codec char array
        cfg.audio.codec = (char*)realloc((char*)cfg.audio.codec, 5*sizeof(char));
        
    if(cfg.audio.buffer_ms == -1)
        cfg.audio.buffer_ms = 50;


    if(cfg.audio.resample_mode == -1)
        cfg.audio.resample_mode = 0; //SRC_SINC_BEST_QUALITY

    // catch illegal audio device number
    if(cfg.audio.dev_num > cfg.audio.dev_count - 1)
        cfg.audio.dev_num = 0;

    cfg.rec.bitrate    = cfg_get_int("record", "bitrate");
    cfg.rec.start_rec  = cfg_get_int("record", "start_rec");
    cfg.rec.sync_to_hour = cfg_get_int("record", "sync_to_hour");
    cfg.rec.split_time = cfg_get_int("record", "split_time");
    cfg.rec.codec      = cfg_get_str("record", "codec");
    cfg.rec.filename   = cfg_get_str("record", "filename");
    cfg.rec.folder     = cfg_get_str("record", "folder");

    if(cfg.rec.bitrate == -1)
        cfg.rec.bitrate = 192;

    if(cfg.rec.start_rec == -1)
        cfg.rec.start_rec = 0;

    if(cfg.rec.sync_to_hour == -1)
        cfg.rec.sync_to_hour = 0;

    if(cfg.rec.split_time == -1)
        cfg.rec.split_time = 0;

    if(cfg.rec.codec == NULL)
    {
        cfg.rec.codec = (char*)malloc(5*sizeof(char));
        strcpy(cfg.rec.codec, "mp3");
    }
    else
        cfg.rec.codec = (char*)realloc(cfg.rec.codec, 5*sizeof(char));

    if(cfg.rec.filename == NULL)
    {
        cfg.rec.filename = (char*)malloc(strlen("rec_%Y%m%d-%H%M%S_%i.mp3")+1);
        strcpy(cfg.rec.filename, "rec_%Y%m%d-%H%M%S_%i.mp3");
    }

    if(cfg.rec.folder == NULL)
    {
        cfg.rec.folder = (char*)malloc(3*sizeof(char));
        strcpy(cfg.rec.folder, "./");
    }


    cfg.selected_srv = 0;

    cfg.main.num_of_srv = cfg_get_int("main", "num_of_srv");
    if(cfg.main.num_of_srv == -1)
    {
        fl_alert("error while parsing config. Illegal value (-1) for num_of_srv.\nbutt is going to close now");
        exit(1);
    }
    if(cfg.main.num_of_srv > 0)
    {
        cfg.main.srv     = cfg_get_str("main", "server");
        if(cfg.main.srv == NULL)
        {
            fl_alert("error while parsing config. Missing main/server entry.\nbutt is going to close now");
            exit(1);
        }

        cfg.main.srv_ent = cfg_get_str("main", "srv_ent");
        if(cfg.main.srv_ent == NULL)
        {
            fl_alert("error while parsing config. Missing main/srv_ent entry.\nbutt is going to close now");
            exit(1);
        }

        cfg.srv = (server_t**)malloc(sizeof(server_t*) * cfg.main.num_of_srv);

        for(i = 0; i < cfg.main.num_of_srv; i++)
            cfg.srv[i] = (server_t*)malloc(sizeof(server_t));

        strtok_buf = strdup(cfg.main.srv_ent);
        srv_ent = strtok(strtok_buf, ";");

        for(i = 0; srv_ent != NULL; i++)
        {
            cfg.srv[i]->name = (char*)malloc((strlen(srv_ent)+1) * sizeof(char));
            snprintf(cfg.srv[i]->name, strlen(srv_ent)+1, "%s", srv_ent);

            cfg.srv[i]->addr = cfg_get_str(srv_ent, "address");
            if(cfg.srv[i]->addr == NULL)
            {
                fl_alert("error while parsing config. Missing address entry for server \"%s\"."
                        "\nbutt is going to close now", srv_ent);
                exit(1);
            }

            cfg.srv[i]->port  = cfg_get_int(srv_ent, "port");
            if(cfg.srv[i]->port == -1)
            {
                fl_alert("error while parsing config. Missing port entry for server \"%s\"."
                        "\nbutt is going to close now", srv_ent);
                exit(1);
            }

            cfg.srv[i]->pwd   = cfg_get_str(srv_ent, "password");
            if(cfg.srv[i]->pwd == NULL)
            {
                fl_alert("error while parsing config. Missing password entry for server \"%s\"."
                        "\nbutt is going to close now", srv_ent);
                exit(1);
            }

            cfg.srv[i]->type  = cfg_get_int(srv_ent, "type");
            if(cfg.srv[i]->type == -1)
            {
                fl_alert("error while parsing config. Missing type entry for server \"%s\"."
                        "\nbutt is going to close now", srv_ent);
                exit(1);
            }

            cfg.srv[i]->mount = cfg_get_str(srv_ent, "mount");
            if((cfg.srv[i]->mount == NULL) &&  (cfg.srv[i]->type == ICECAST))
            {
                fl_alert("error while parsing config. Missing mount entry for server \"%s\"."
                        "\nbutt is going to close now", srv_ent);
                exit(1);
            }

            cfg.srv[i]->usr = cfg_get_str(srv_ent, "usr");
            if((cfg.srv[i]->usr == NULL) &&  (cfg.srv[i]->type == ICECAST))
            {
                cfg.srv[i]->usr = (char*)malloc(strlen("source")*sizeof(char)+1);
                strcpy(cfg.srv[i]->usr, "source");
            }

            if(!strcmp(cfg.srv[i]->name, cfg.main.srv))
                cfg.selected_srv = i;

            srv_ent = strtok(NULL, ";");
        }
    }// if(cfg.main.num_of_srv > 0)

    cfg.main.num_of_icy = cfg_get_int("main", "num_of_icy");
    if(cfg.main.num_of_icy == -1)
    {
        fl_alert("error while parsing config. Illegal value (-1) for num_of_icy.\nbutt is going to close now");
        exit(1);
    }
    
    cfg.selected_icy = 0;

    if(cfg.main.num_of_icy > 0)
    {
        cfg.main.icy     = cfg_get_str("main", "icy");
        if(cfg.main.icy == NULL)
        {
            fl_alert("error while parsing config. Missing main/icy entry.\nbutt is going to close now");
            exit(1);
        }
        cfg.main.icy_ent = cfg_get_str("main", "icy_ent");          //icy entries
        if(cfg.main.icy_ent == NULL)
        {
            fl_alert("error while parsing config. Missing main/icy_ent entry.\nbutt is going to close now");
            exit(1);
        }

        cfg.icy = (icy_t**)malloc(sizeof(icy_t*) * cfg.main.num_of_icy);

        for(i = 0; i < cfg.main.num_of_icy; i++)
            cfg.icy[i] = (icy_t*)malloc(sizeof(icy_t));

        strtok_buf = strdup(cfg.main.icy_ent);
        icy_ent = strtok(strtok_buf, ";");

        for(i = 0; icy_ent != NULL; i++)
        {
            cfg.icy[i]->name = (char*)malloc(strlen(icy_ent) * sizeof(char)+1);
            snprintf(cfg.icy[i]->name, strlen(icy_ent)+1, "%s", icy_ent);

            cfg.icy[i]->desc  = cfg_get_str(icy_ent, "description");
            cfg.icy[i]->genre = cfg_get_str(icy_ent, "genre");
            cfg.icy[i]->url   = cfg_get_str(icy_ent, "url");
            cfg.icy[i]->irc   = cfg_get_str(icy_ent, "irc");
            cfg.icy[i]->icq   = cfg_get_str(icy_ent, "icq");
            cfg.icy[i]->aim   = cfg_get_str(icy_ent, "aim");
            cfg.icy[i]->pub   = cfg_get_str(icy_ent, "pub");
            if(cfg.icy[i]->pub == NULL)
            {
                fl_alert("error while parsing config. Missing pub entry for icy \"%s\"."
                        "\nbutt is going to close now", icy_ent);
                exit(1);
            }

            if(!strcmp(cfg.icy[i]->name, cfg.main.icy))
                cfg.selected_icy = i;

            icy_ent = strtok(NULL, ";");
        }
    }//if(cfg.main.num_of_icy > 0)

    if (strtok_buf != NULL)
        free(strtok_buf);

    cfg.main.song_path = cfg_get_str("main", "song_path");

    cfg.main.song_update = cfg_get_int("main", "song_update");
    if(cfg.main.song_update == -1)
        cfg.main.song_update = 0; //song update from file is default set to off

	cfg.main.connect_at_startup = cfg_get_int("main", "connect_at_startup");
	if(cfg.main.connect_at_startup == -1)
		cfg.main.connect_at_startup = 0;

	cfg.main.gain = cfg_get_float("main", "gain");
	if((int)cfg.main.gain == -1)
		cfg.main.gain = 1.0;
    else if(cfg.main.gain > util_db_to_factor(24))
        cfg.main.gain = util_db_to_factor(24);
    else if(cfg.main.gain < 0)
        cfg.main.gain = util_db_to_factor(-24);


    //read GUI stuff 
    cfg.gui.attach = cfg_get_int("gui", "attach");
    if(cfg.gui.attach == -1)
        cfg.gui.attach = 0;

    cfg.gui.ontop = cfg_get_int("gui", "ontop");
    if(cfg.gui.ontop == -1)
        cfg.gui.ontop = 0;

    cfg.gui.lcd_auto = cfg_get_int("gui", "lcd_auto");
    if(cfg.gui.lcd_auto == -1)
        cfg.gui.lcd_auto = 0;


    //read FLTK related stuff
    cfg.main.bg_color = cfg_get_int("main", "bg_color");
    if(cfg.main.bg_color == -1)
        cfg.main.bg_color = 151540480; //dark blue

    cfg.main.txt_color = cfg_get_int("main", "txt_color");
    if(cfg.main.txt_color == -1)
        cfg.main.txt_color = -256; //white

    return 0;
}

int cfg_create_default(void)
{
    FILE *cfg_fd;
    char *p;
    char def_rec_folder[PATH_MAX];

    cfg_fd = fl_fopen(cfg_path, "wb");
    if(cfg_fd == NULL)
        return 1;


#ifdef _WIN32
    p = getenv("USERPROFILE");
    if (p != NULL)
        snprintf(def_rec_folder, PATH_MAX, "%s\\Music\\", p);
    else
        snprintf(def_rec_folder, PATH_MAX, "./");
#elif __APPLE__
    p = getenv("HOME");
    if (p != NULL)
        snprintf(def_rec_folder, PATH_MAX, "%s/Music/", p);
    else
        snprintf(def_rec_folder, PATH_MAX, "./");
#else //UNIX
    p = getenv("HOME");
    if (p != NULL)
        snprintf(def_rec_folder, PATH_MAX, "%s/", p);
    else
        snprintf(def_rec_folder, PATH_MAX, "./");
#endif


    fprintf(cfg_fd, "This is a configuration file for butt (broadcast using this tool)\n\n");
    fprintf(cfg_fd, 
            "[main]\n"
            "server =\n"
            "icy =\n"
            "num_of_srv = 0\n"
            "num_of_icy = 0\n"
            "srv_ent =\n"
            "icy_ent =\n"
            "song_path =\n"
            "song_update = 0\n"
            "log_file =\n"
            "gain = 1.0\n"
            "connect_at_startup = 0\n\n"
           );

    fprintf(cfg_fd,
            "[audio]\n"
            "device = default\n"
            "samplerate = 44100\n"
            "bitrate = 128\n"
            "channel = 2\n"
            "codec = mp3\n"
            "resample_mode = 0\n" //SRC_SINC_BEST_QUALITY
            "buffer_ms = 50\n\n"
           );

    fprintf(cfg_fd,
            "[record]\n"
            "samplerate = 44100\n"
            "bitrate = 192\n"
            "channel = 2\n"
            "codec = mp3\n"
            "start_rec = 0\n"
            "sync_to_hour = 0\n"
            "split_time = 0\n"
            "filename = rec_%%Y%%m%%d-%%H%%M%%S_%%i.mp3\n"
            "folder = %s\n\n", def_rec_folder
           );

    fprintf(cfg_fd, 
            "[gui]\n"
            "attach = 0\n"
            "ontop = 0\n"
            "lcd_auto = 0\n\n"
            );

    fclose(cfg_fd);

    return 0;
}

