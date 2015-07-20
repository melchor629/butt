//
//  aacplus_encode.cpp
//  butt
//
//  Created by Melchor Garau Madrigal on 14/7/15.
//  Copyright (c) 2015 Daniel Nöthen. All rights reserved.
//

#include "aacplus_encode.h"
#include "fl_funcs.h"
#include "util.h"
#include <cstring>
#include <stdlib.h>
#include <stdio.h>

int aacplus_enc_init(aacplus_enc *aacplus) {
    unsigned long s, b;
    aacplus->encoder = aacplusEncOpen(aacplus->samplerate, aacplus->channels, &s, &b);
    aacplus->input_samples = (unsigned) s;
    aacplus->max_output_bytes = (unsigned) b;
    aacplus->conf = aacplusEncGetCurrentConfiguration(aacplus->encoder);
    aacplus->conf->bitRate = aacplus->bitrate;
    aacplus->conf->bandWidth = 0;
    aacplus->conf->outputFormat = 1; //ADTS
    aacplus->conf->nChannelsOut = aacplus->channels;
    aacplus->conf->inputFormat = AACPLUS_INPUT_16BIT;
    
    int ret = 0;
    if(!aacplusEncSetConfiguration(aacplus->encoder, aacplus->conf)) {
        char info_buf[100];
        fprintf(stderr, "Mala configuracion de AAC+\n");
        snprintf(info_buf, sizeof(info_buf), "Mala configuracion de AAC+");
        print_info(info_buf, 1);
        ret = 1;
    }

    aacplus->buff = buffer(aacplus->input_samples); // Los samples en este codificador no son samples
    aacplus->state = AACPLUS_READY;
    return ret;
}

int aacplus_enc_encode(aacplus_enc *aacplus, short *pcm_buf, char* enc_buf, int samples, int size) {
    if(size == 0 || aacplus->encoder == NULL)
        return 0;
    int bytes = 0;
    size_t size_pcm_buf = samples * aacplus->channels * 2;
    size = aacplus->max_output_bytes;
    uint8_t* audio = (uint8_t*) pcm_buf;
    
    if(samples * aacplus->channels < aacplus->input_samples) {
        size_t bytes_copied = aacplus->buff.append(pcm_buf, size_pcm_buf);
        if(bytes_copied < size_pcm_buf) {
            bytes = aacplusEncEncode(aacplus->encoder, aacplus->buff.get<int>(), aacplus->input_samples, (uint8_t*) enc_buf, size);
            bytes_copied += aacplus->buff.insert(0, &audio[bytes_copied], size_pcm_buf - bytes_copied);
            if(size_pcm_buf != bytes_copied)
                printf("%lu\n", size_pcm_buf - bytes_copied);
        }
    } else {
        size_t bytes_copied = aacplus->buff.append(pcm_buf, size_pcm_buf);
        bytes = aacplusEncEncode(aacplus->encoder, aacplus->buff.get<int>(), aacplus->input_samples, (uint8_t*) enc_buf, size);
        bytes_copied += aacplus->buff.insert(0, &audio[bytes_copied], size_pcm_buf - bytes_copied);
        if(bytes_copied < size_pcm_buf) {
            bytes += aacplusEncEncode(aacplus->encoder, aacplus->buff.get<int>(), aacplus->input_samples, (uint8_t*)&enc_buf[bytes], size);
            bytes_copied += aacplus->buff.insert(0, &audio[bytes_copied], size_pcm_buf - bytes_copied);
        }
    }
    

    return bytes;
}

int aacplus_enc_reinit(aacplus_enc *aacplus) {
    if(aacplus != NULL) {
        aacplus_enc_close(aacplus);
        return aacplus_enc_init(aacplus);
    }
    return 0;
}

void aacplus_enc_close(aacplus_enc *aacplus) {
    while(aacplus->state == AACPLUS_BUSY);
    
    if(aacplus->encoder != NULL)
        aacplusEncClose(aacplus->encoder);
    
    aacplus->buff.destroy();
    aacplus->conf = NULL;
    aacplus->encoder = NULL;
}
