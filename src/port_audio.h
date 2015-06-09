// audio functions for butt
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

#ifndef PORT_AUDIO_H
#define PORT_AUDIO_H

#include <stdio.h>
#include <stdlib.h>

#include <portaudio.h>

#include "lame_encode.h"

typedef struct
{
    char *name;
    int dev_id;
    int sr_list[10]; //8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000, 0
    int num_of_sr;

}snd_dev_t;

enum {
    SND_STREAM = 0,
    SND_REC = 1
};


extern bool try_to_connect;
extern bool pa_new_frames;
extern bool reconnect;
extern bool next_file;
extern FILE *next_fd;

int *snd_get_samplerates(int *sr_count);
snd_dev_t **snd_get_devices(int *dev_count);
void *snd_rec_thread(void *data);
void *snd_stream_thread(void *data);

void snd_loop(void);
void snd_close(void);
void snd_update_vu(void);
void snd_start_stream(void);
void snd_stop_stream(void);
void snd_start_rec(void);
void snd_stop_rec(void);
void snd_reinit(void);

int snd_init(void);
int snd_open_stream(void);
int snd_write_buf(void);
void snd_reset_samplerate_conv(int rec_or_stream);

int snd_callback(const void *input,
                 void *output,
                 unsigned long frameCount,
                 const PaStreamCallbackTimeInfo* timeInfo,
                 PaStreamCallbackFlags statusFlags,
                 void *userData);
#endif

