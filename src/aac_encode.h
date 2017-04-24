//
//  aac_encode.h
//  butt
//
//  Created by Melchor Garau Madrigal on 14/7/15.
//  Copyright (c) 2015 Daniel Nöthen. All rights reserved.
//

#ifndef __butt__aac_encode__
#define __butt__aac_encode__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <config.h>
#include <stdint.h>

#include <fdk-aac/aacenc_lib.h>

class buffer {
    size_t size, position;
public:
    uint8_t* buff;

    buffer(size_t size) {
        if(size != 0)
            buff = (uint8_t*) calloc(size, sizeof(short));
        this->size = size * sizeof(short);
        position = 0;
    }
    
    size_t append(void* ptr, size_t size) {
        if(position + size < this->size) {
            memcpy(&buff[position], ptr, size);
            position += size;
            return size;
        } else {
            size_t sobrante = this->size - position;
            memcpy(&buff[position], ptr, this->size - position);
            position = this->size;
            return sobrante;
        }
    }
    
    size_t insert(size_t pos, void* ptr, size_t siz) {
        if(pos + siz < size) {
            memcpy(&buff[pos], ptr, siz);
            position = pos + siz;
            return siz;
        } else {
            memcpy(&buff[pos], ptr, size - pos);
            position = size;
            return size - pos;
        }
    }
    
    template <typename T> T* get() {
        return (T*) buff;
    }
    
    void destroy() {
        if(size != 0)
            free(buff);
        size = position = 0;
        buff = NULL;
    }
};

struct aac_enc {
    HANDLE_AACENCODER enc;
    AACENC_InfoStruct info = {0};

    int bitrate;
    int samplerate;
    int channels;
    volatile int state;
    
    unsigned input_samples;
    unsigned max_output_bytes;
    buffer buff;
    
    aac_enc() : buff(0) {}
};

enum {
    AACPLUS_READY, AACPLUS_BUSY
};

int aac_enc_init(aac_enc *aacplus);
int aac_enc_encode(aac_enc *aacplus, short *pcm_buf, char* enc_buf, int samples, int channel);
int aac_enc_reinit(aac_enc *aacplus);
void aac_enc_close(aac_enc *aacplus);

#endif /* defined(__butt__aac_encode__) */
