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

#ifndef CFG_H
#define CFG_H

#include "port_audio.h"
#include "parseconfig.h"

enum {

    SHOUTCAST = 0,
    ICECAST = 1
};

enum {
    CHOICE_STEREO = 0,
    CHOICE_MONO = 1
};

enum {
    CHOICE_MP3 = 0,
    CHOICE_OGG = 1,
    CHOICE_OPUS = 2,
    CHOICE_FLAC = 3,
    CHOICE_WAV = 4
};

extern const char CONFIG_FILE[];
typedef struct
{
    char *name;
    char *addr;
    char *pwd;
    char *mount;        //mountpoint for icecast server
    char *usr;          //user for icecast server
    int port;
    int type;           //SHOUTCAST or ICECAST

}server_t;


typedef struct
{
        char *name;
        char *desc; //description
        char *genre;
        char *url;
        char *irc;
        char *icq;
        char *aim;
        char *pub;

}icy_t;


typedef struct
{
    int selected_srv;
    int selected_icy;

    struct
    {
        char *srv;
        char *icy;
        char *srv_ent;
        char *icy_ent;
        char *song;
        char *song_path;
        FILE *song_fd;
        int song_update;   //1 = song info will be read from file
        int num_of_srv;
        int num_of_icy;
        int bg_color, txt_color;
	int connect_at_startup;
        float gain;
	char *log_file;

    }main;

    struct
    {
        int dev_count;
        int dev_num;
        snd_dev_t **pcm_list;
        int samplerate;
        int resolution;
        int channel;
        int bitrate;
        int buffer_ms;
        int resample_mode;
        char *codec;

    }audio;

    struct
    {
        int channel;
        int bitrate;
        int quality;
        int samplerate;
        char *codec;
        char *filename;
        char *folder;
        char *path;
        FILE *fd;
        int start_rec;
        int split_time;
        int sync_to_hour;

    }rec;

    struct
    {
        int attach;
        int ontop;
	int lcd_auto;
    }gui;

    server_t **srv;
    icy_t **icy;

}config_t;



extern char *cfg_path;      //Path to config file
extern config_t cfg;        //Holds config parameters
extern bool unsaved_changes;//is checked when closing butt and informs the user for unsaved changes

int cfg_write_file(char *path); //Writes current config_t struct to path or cfg_path if path is NULL
int cfg_set_values(char *path); //Reads config file from path or cfg_path if path is NULL and fills the config_t struct
int cfg_create_default(void);   //Creates a default config file, if there isn't one yet

#endif

