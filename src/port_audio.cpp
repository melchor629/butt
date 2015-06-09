// portaudio functions for butt
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
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include <string.h>
#include <pthread.h>
#include <samplerate.h>

#ifdef _WIN32
 #include <windows.h>
#endif

#include "config.h"

#include "butt.h"
#include "cfg.h"
#include "port_audio.h"
#include "parseconfig.h"
#include "lame_encode.h"
#include "shoutcast.h"
#include "icecast.h"
#include "strfuncs.h"
#include "wav_header.h"
#include "ringbuffer.h"
#include "vu_meter.h"
#include "flgui.h"
#include "fl_funcs.h"


int pa_frames = 2048;

char* encode_buf;
short *pa_pcm_buf;
int buf_index;
int buf_pos;
int framepacket_size;

bool try_to_connect;
bool pa_new_frames;
bool reconnect;

bool next_file;
FILE *next_fd;

struct ringbuf rec_rb;
struct ringbuf stream_rb;

SRC_STATE *srconv_state_stream = NULL;
SRC_STATE *srconv_state_record = NULL;
SRC_DATA srconv_stream;
SRC_DATA srconv_record;

pthread_t rec_thread;
pthread_t stream_thread;
pthread_mutex_t stream_mut, rec_mut;
pthread_cond_t  stream_cond, rec_cond;

PaStream *stream;

int snd_init(void)
{
    char info_buf[256];

    PaError p_err;
    if((p_err = Pa_Initialize()) != paNoError)
    {
        snprintf(info_buf, sizeof(info_buf),
				"PortAudio init failed:\n%s\n",
				Pa_GetErrorText(p_err));

        ALERT(info_buf);
        return 1;
    }

    srconv_stream.data_in = (float*)malloc(pa_frames*2 * sizeof(float));
    srconv_stream.data_out = (float*)malloc(pa_frames*2 * 10 * sizeof(float));
    srconv_record.data_in = (float*)malloc(pa_frames*2 * sizeof(float));
    srconv_record.data_out = (float*)malloc(pa_frames*2 * 10 * sizeof(float));

    reconnect = 0;
    buf_index = 0;
    return 0;
}

void snd_reinit(void)
{
    snd_close();
    snd_init();
    snd_open_stream();
}

int snd_open_stream(void)
{

    int samplerate;
    char info_buf[256];

    PaDeviceIndex pa_dev_id;
    PaStreamParameters pa_params;
    PaError pa_err;
    const PaDeviceInfo *pa_dev_info;

    if(cfg.audio.dev_count == 0)
    {
        print_info("ERROR: no sound device with input channels found", 1);
        return 1;
    }


    pa_frames = (cfg.audio.buffer_ms/1000.0)*cfg.audio.samplerate;

    snd_reset_samplerate_conv(SND_STREAM);
    snd_reset_samplerate_conv(SND_REC);


    framepacket_size = pa_frames * cfg.audio.channel;

    pa_pcm_buf = (short*)malloc(16 * framepacket_size * sizeof(short));
    encode_buf = (char*)malloc(16 * framepacket_size * sizeof(char));

    rb_init(&rec_rb, 16 * framepacket_size * sizeof(short));
    rb_init(&stream_rb, 16 * framepacket_size * sizeof(short));

    samplerate = cfg.audio.samplerate;

    pa_dev_id = cfg.audio.pcm_list[cfg.audio.dev_num]->dev_id;

    pa_dev_info = Pa_GetDeviceInfo(pa_dev_id);
    if(pa_dev_info == NULL)
    {
        snprintf(info_buf, 127, "Error getting device Info (%d)", pa_dev_id);
        print_info(info_buf, 1);
        return 1;
    }

    pa_params.device = pa_dev_id;
    pa_params.channelCount = cfg.audio.channel;
    pa_params.sampleFormat = paInt16;
    pa_params.suggestedLatency = pa_dev_info->defaultHighInputLatency;
    pa_params.hostApiSpecificStreamInfo = NULL;

    pa_err = Pa_IsFormatSupported(&pa_params, NULL, samplerate);
    if(pa_err != paFormatIsSupported)
    {
        if(pa_err == paInvalidSampleRate)
        {
            snprintf(info_buf, sizeof(info_buf),
                    "Samplerate not supported: %dHz\n"
                    "Using default samplerate: %dHz",
                    samplerate, (int)pa_dev_info->defaultSampleRate);
            print_info(info_buf, 1);

            if(Pa_IsFormatSupported(&pa_params, NULL,
               pa_dev_info->defaultSampleRate) != paFormatIsSupported)
            {
                print_info("FAILED", 1);
                return 1;
            }
            else
            {
                samplerate = (int)pa_dev_info->defaultSampleRate;
                cfg.audio.samplerate = samplerate;
                update_samplerates();
            }
        }
        else
        {
            snprintf(info_buf, sizeof(info_buf), "PA: Format not supported: %s\n",
                    Pa_GetErrorText(pa_err));
            print_info(info_buf, 1);
            return 1;
        }
    }

    pa_err = Pa_OpenStream(&stream, &pa_params, NULL,
                            samplerate, pa_frames,
                            paNoFlag, snd_callback, NULL);

    if(pa_err != paNoError)
    {
        printf("error opening sound device: \n%s\n", Pa_GetErrorText(pa_err));
        return 1;
    }

    
    Pa_StartStream(stream);
    return 0;
}

void snd_start_stream(void)
{   
    pthread_mutex_init(&stream_mut, NULL);
    pthread_cond_init (&stream_cond, NULL);
    
    kbytes_sent = 0;
    streaming = 1;

    pthread_create(&stream_thread, NULL, snd_stream_thread, NULL);
}

void snd_stop_stream(void)
{
    connected = 0;
    streaming = 0;

    pthread_cond_signal(&stream_cond);

    pthread_mutex_destroy(&stream_mut);
    pthread_cond_destroy(&stream_cond);


    print_info("user disconnected\n", 0);
}

void *snd_stream_thread(void *data)
{
    int sent;
    int rb_bytes_read;
	int encode_bytes_read = 0;
    int bytes_to_read;

    char *enc_buf = (char*)malloc(stream_rb.size * sizeof(char)*10);
    char *audio_buf = (char*)malloc(stream_rb.size * sizeof(char)*10);

    int (*xc_send)(char *buf, int buf_len) = NULL;

    // Read always chunks of 960 frames from the audio ringbuffer to be
    // compatible with OPUS
    bytes_to_read = 960 * sizeof(short)*cfg.audio.channel;


    if(cfg.srv[cfg.selected_srv]->type == SHOUTCAST)
        xc_send = &sc_send;
    else //Icecast
        xc_send = &ic_send;

    while(connected)
    {

        pthread_cond_wait(&stream_cond, &stream_mut);
        if(!connected)
            break;

        if(!strcmp(cfg.audio.codec, "opus"))
        {
            while ((rb_filled(&stream_rb)) >= bytes_to_read)
            {
                rb_read_len(&stream_rb, audio_buf, bytes_to_read);

                encode_bytes_read = opus_enc_encode(&opus_stream, (short*)audio_buf,
                        enc_buf, bytes_to_read/(2*cfg.audio.channel));

                if((sent = xc_send(enc_buf, bytes_to_read)) == -1)
                {
                    connected = 0;
                }
                else
                    kbytes_sent += bytes_to_read/1024.0;

            }
        }
        else // ogg and mp3 need more data than opus in order to compress the audio data
        {
            if(rb_filled(&stream_rb) < framepacket_size*sizeof(short))
                continue;

            rb_bytes_read = rb_read(&stream_rb, audio_buf);
            if(rb_bytes_read == 0)
                continue;

            if(!strcmp(cfg.audio.codec, "mp3"))
                encode_bytes_read = lame_enc_encode(&lame_stream, (short*)audio_buf, enc_buf,
                        rb_bytes_read/(2*cfg.audio.channel), stream_rb.size*10);
            
            if(!strcmp(cfg.audio.codec, "ogg"))
                encode_bytes_read = vorbis_enc_encode(&vorbis_stream, (short*)audio_buf, 
                        enc_buf, rb_bytes_read/(2*cfg.audio.channel));

            if((sent = xc_send(enc_buf, encode_bytes_read)) == -1)
                connected = 0; 
            else
                kbytes_sent += encode_bytes_read/1024.0;

        }
    }

    free(enc_buf);
    free(audio_buf);

    return NULL;
}

void snd_start_rec(void)
{
    int error;
    next_file = 0;

    kbytes_written = 0;
    recording = 1;
    
    pthread_mutex_init(&rec_mut, NULL);
    pthread_cond_init (&rec_cond, NULL);

    pthread_create(&rec_thread, NULL, snd_rec_thread, NULL);

    print_info("recording to:", 0);
    print_info(cfg.rec.path, 0);
}

void snd_stop_rec(void)
{
    record = 0;
    recording = 0;
    

    pthread_cond_signal(&rec_cond);

    pthread_mutex_destroy(&rec_mut);
    pthread_cond_destroy(&rec_cond);

    print_info("recording stopped", 0);
}

//The recording stuff runs in its own thread
//this prevents dropouts in the recording in case the
//bandwidth is smaller than the selected streaming bitrate
void* snd_rec_thread(void *data)
{
    int error;
    int rb_bytes_read;
    int bytes_to_read;
    int ogg_header_written;
    int opus_header_written;
    int enc_bytes_read;
    char info_buf[256];
    
    char *enc_buf = (char*)malloc(rec_rb.size * sizeof(char)*10);
    char *audio_buf = (char*)malloc(rec_rb.size * sizeof(char)*10);
    
    bytes_to_read = 960 * sizeof(short)*cfg.audio.channel;
    ogg_header_written = 0;
    opus_header_written = 0;

    while(record)
    {
        pthread_cond_wait(&rec_cond, &rec_mut);

        if(next_file == 1)
        {
            if(!strcmp(cfg.rec.codec, "flac"))
                flac_enc_close(&flac_rec);
            else
                fclose(cfg.rec.fd);

            cfg.rec.fd = next_fd;
            next_file = 0;
            if(!strcmp(cfg.rec.codec, "ogg"))
            {
                vorbis_enc_reinit(&vorbis_rec);
                ogg_header_written = 0;
            }
            if(!strcmp(cfg.rec.codec, "opus"))
            {
                opus_enc_reinit(&opus_rec);
                opus_header_written = 0;
            }
            if(!strcmp(cfg.rec.codec, "flac"))
            {
                flac_enc_reinit(&flac_rec);
                flac_enc_init_FILE(&flac_rec, cfg.rec.fd);
            }
        }

        // Opus needs a special treatment
        // The encoder needs a predefined count of frames
        // Therefore we don't feed the encoder with all data we have in the
        // ringbuffer at once 
        if(!strcmp(cfg.rec.codec, "opus"))
        {
            while ((rb_filled(&rec_rb)) >= bytes_to_read)
            {
                rb_read_len(&rec_rb, audio_buf, bytes_to_read);

                if(!opus_header_written)
                {
                    opus_enc_write_header(&opus_rec);
                    opus_header_written = 1;
                }

                enc_bytes_read = opus_enc_encode(&opus_rec, (short*)audio_buf, 
                        enc_buf, bytes_to_read/(2*cfg.audio.channel));
                kbytes_written += fwrite(enc_buf, 1, enc_bytes_read, cfg.rec.fd)/1024.0;
            }
        }
        else
        {

            if(rb_filled(&rec_rb) < framepacket_size*sizeof(short))
                continue;

            rb_bytes_read = rb_read(&rec_rb, audio_buf);
            if(rb_bytes_read == 0)
                continue;


            if(!strcmp(cfg.rec.codec, "mp3"))
            {

                enc_bytes_read = lame_enc_encode(&lame_rec, (short*)audio_buf, enc_buf,
                        rb_bytes_read/(2*cfg.audio.channel), rec_rb.size*10);
                kbytes_written += fwrite(enc_buf, 1, enc_bytes_read, cfg.rec.fd)/1024.0;
            }

            if(!strcmp(cfg.rec.codec, "ogg"))
            {
                if(!ogg_header_written)
                {
                    vorbis_enc_write_header(&vorbis_rec);
                    ogg_header_written = 1;
                }

                enc_bytes_read = vorbis_enc_encode(&vorbis_rec, (short*)audio_buf, 
                        enc_buf, rb_bytes_read/(2*cfg.audio.channel));
                kbytes_written += fwrite(enc_buf, 1, enc_bytes_read, cfg.rec.fd)/1024.0;
            }

            if(!strcmp(cfg.rec.codec, "flac"))
            {
                flac_enc_encode(&flac_rec, (short*)audio_buf, rb_bytes_read/sizeof(short)/cfg.audio.channel, cfg.audio.channel);
                kbytes_written = flac_enc_get_bytes_written()/1024.0;
            }


            if(!strcmp(cfg.rec.codec, "wav"))
            {
                //this permanently updates the filesize value in the WAV header
                //so we still have a valid WAV file in case of a crash
                wav_write_header(cfg.rec.fd, cfg.audio.channel, cfg.audio.samplerate, 16);
                kbytes_written += fwrite(audio_buf, sizeof(char), rb_bytes_read, cfg.rec.fd)/1024.0;
            }
        }
    }

    if(!strcmp(cfg.rec.codec, "flac"))
        flac_enc_close(&flac_rec);
    else
        fclose(cfg.rec.fd);
    
    free(enc_buf);
    free(audio_buf);
    
    return NULL;
}

//this function is called by PortAudio when new audio data arrived
int snd_callback(const void *input,
                 void *output,
                 unsigned long frameCount,
                 const PaStreamCallbackTimeInfo* timeInfo,
                 PaStreamCallbackFlags statusFlags,
                 void *userData)
{
    int i;
    int error;
    int samplerate_out;
    bool convert_stream = false;
    bool convert_record = false;
    
    char stream_buf[16 * pa_frames*2 * sizeof(short)];
    char record_buf[16 * pa_frames*2 * sizeof(short)];


    memcpy(pa_pcm_buf, input, frameCount*cfg.audio.channel*sizeof(short));
    samplerate_out = cfg.audio.samplerate;

    if (cfg.main.gain != 1)
    {
        for(i = 0; i < framepacket_size; i++)
        {
            pa_pcm_buf[i] *= cfg.main.gain;
        }
    }
	
	if (streaming)
	{
        if ((!strcmp(cfg.audio.codec, "opus")) && (cfg.audio.samplerate != 48000))
        {
            convert_stream = true;
            samplerate_out = 48000;
        }

        if (convert_stream == true)
        {
            srconv_stream.end_of_input = 0;
            srconv_stream.src_ratio = (float)samplerate_out/cfg.audio.samplerate;
            srconv_stream.input_frames = frameCount;
            srconv_stream.output_frames = frameCount*cfg.audio.channel * (srconv_stream.src_ratio+1) * sizeof(float);

            src_short_to_float_array((short*)pa_pcm_buf, srconv_stream.data_in, frameCount*cfg.audio.channel);

            //The actual resample process
            src_process(srconv_state_stream, &srconv_stream);

            src_float_to_short_array(srconv_stream.data_out, (short*)stream_buf, srconv_stream.output_frames_gen*cfg.audio.channel);

            rb_write(&stream_rb, (char*)stream_buf, srconv_stream.output_frames_gen*sizeof(short)*cfg.audio.channel);
        }
        else
            rb_write(&stream_rb, (char*)pa_pcm_buf, frameCount*sizeof(short)*cfg.audio.channel);

		pthread_cond_signal(&stream_cond);
	}

	if(recording)
	{

        if ((!strcmp(cfg.rec.codec, "opus")) && (cfg.audio.samplerate != 48000))
        {
            convert_record = true;
            samplerate_out = 48000;
        }

        if (convert_record == true)
        {
            srconv_record.end_of_input = 0;
            srconv_record.src_ratio = (float)samplerate_out/cfg.audio.samplerate;
            srconv_record.input_frames = frameCount;
            srconv_record.output_frames = frameCount*cfg.audio.channel * (srconv_record.src_ratio+1) * sizeof(float);

            src_short_to_float_array((short*)pa_pcm_buf, srconv_record.data_in, frameCount*cfg.audio.channel);

            //The actual resample process
            src_process(srconv_state_record, &srconv_record);

            src_float_to_short_array(srconv_record.data_out, (short*)record_buf, srconv_record.output_frames_gen*cfg.audio.channel);

            rb_write(&rec_rb, (char*)record_buf, srconv_record.output_frames_gen*sizeof(short)*cfg.audio.channel);

        }
        else
            rb_write(&rec_rb, (char*)pa_pcm_buf, frameCount*sizeof(short)*cfg.audio.channel);

		pthread_cond_signal(&rec_cond);
	}
    
    //tell vu_update() that there is new audio data
    pa_new_frames = 1;

    return 0;
}

void snd_update_vu(void)
{
    int i;
    int lpeak = 0;
    int rpeak = 0;
    short *p;

    p = pa_pcm_buf;
    for(i = 0; i < framepacket_size; i += cfg.audio.channel)
    {
        if(abs(p[i]) > lpeak)
            lpeak = abs(p[i]);
        if(abs(p[i+(cfg.audio.channel-1)]) > rpeak)
            rpeak = abs(p[i+(cfg.audio.channel-1)]);
    }

    vu_meter(lpeak, rpeak);

    pa_new_frames = 0;
}

snd_dev_t **snd_get_devices(int *dev_count)
{
	int i, j;
    int devcount, sr_count, dev_num;
    bool sr_supported = 0;
    const PaDeviceInfo *p_di;
    char info_buf[256];
    PaStreamParameters pa_params;

    int sr[] = { 8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000 };

    snd_dev_t **dev_list;

	dev_num = 0;

    dev_list = (snd_dev_t**)malloc(100*sizeof(snd_dev_t*));

	//100 sound devices should be enough
    for(i = 0; i < 100; i++)
        dev_list[i] = (snd_dev_t*)malloc(sizeof(snd_dev_t));

    dev_list[dev_num]->name = (char*) malloc(strlen("Default PCM device (default)")+1);
    strcpy(dev_list[dev_num]->name, "Default PCM device (default)");
    dev_list[dev_num]->dev_id = Pa_GetDefaultInputDevice();
    dev_num++;


    devcount = Pa_GetDeviceCount();
    if(devcount < 0)
    {
        snprintf(info_buf, sizeof(info_buf), "PaError: %s", Pa_GetErrorText(devcount));
        print_info(info_buf, 1);
    }

    for(i = 0; i < devcount && i < 100; i++)
    {
        sr_count = 0;
        p_di = Pa_GetDeviceInfo(i);
        if(p_di == NULL)
        {
            snprintf(info_buf, sizeof(info_buf), "Error getting device Info (%d)", i);
            print_info(info_buf, 1);
            continue;
        }


        //Save only devices which have input Channels
        if(p_di->maxInputChannels <= 0)
            continue;

        pa_params.device = i;
        pa_params.channelCount = cfg.audio.channel;
        pa_params.sampleFormat = paInt16;
        pa_params.suggestedLatency = p_di->defaultHighInputLatency;
        pa_params.hostApiSpecificStreamInfo = NULL;

        //add the supported samplerates to the device structure
        for(j = 0; j < 9; j++)
        {
            if(Pa_IsFormatSupported(&pa_params, NULL, sr[j]) != paInvalidSampleRate)
            {
                dev_list[dev_num]->sr_list[sr_count] = sr[j];
                sr_count++;
                sr_supported = 1;
            }
        }
        //Go to the next device if this one doesn't support at least one of our samplerates
        if(!sr_supported)
            continue;
        
        dev_list[dev_num]->num_of_sr = sr_count;

        //Mark the end of the samplerate list for this device with a 0
        dev_list[dev_num]->sr_list[sr_count] = 0;

        dev_list[dev_num]->name = (char*) malloc(strlen(p_di->name)+1);
        strcpy(dev_list[dev_num]->name, p_di->name);
        dev_list[dev_num]->dev_id = i;

        //copy the sr_list from the device where the
        //virtual default device points to
        if(dev_list[0]->dev_id == dev_list[dev_num]->dev_id)
        {
            memcpy(dev_list[0]->sr_list, dev_list[dev_num]->sr_list,
                    sizeof(dev_list[dev_num]->sr_list));
            
            dev_list[0]->num_of_sr = sr_count;
        }


        //We need to escape every '/' in the device name
        //otherwise FLTK will add a submenu for every '/' in the dev list
        strrpl(&dev_list[dev_num]->name, (char*)"/", (char*)"\\/", MODE_ALL);

        dev_num++;
    }//for(i = 0; i < devcount && i < 100; i++)


    if(dev_num == 1)
        *dev_count = 0;
    else
        *dev_count = dev_num;

    return dev_list;
}

void snd_reset_samplerate_conv(int rec_or_stream)
{
    int error;
    
    if (rec_or_stream == SND_STREAM)
    { 
        if (srconv_state_stream != NULL)
        {
            src_delete(srconv_state_stream);
            srconv_state_stream = NULL;
        }

        srconv_state_stream = src_new(cfg.audio.resample_mode, cfg.audio.channel, &error);
        if (srconv_state_stream == NULL)
        {
            print_info("ERROR: Could not initialize samplerate converter", 0);
        }
    }

    if (rec_or_stream == SND_REC)
    { 
        if (srconv_state_record != NULL)
        {
            src_delete(srconv_state_record);
            srconv_state_record = NULL;
        }


        srconv_state_record = src_new(cfg.audio.resample_mode, cfg.audio.channel, &error);
        if (srconv_state_record == NULL)
        {
            print_info("ERROR: Could not initialize samplerate converter", 0);
        }
    }
}

void snd_close(void)
{
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    free(srconv_stream.data_in);
    free(srconv_stream.data_out);

    free(srconv_record.data_in);
    free(srconv_record.data_out);

    free(pa_pcm_buf);
    free(encode_buf);
}


