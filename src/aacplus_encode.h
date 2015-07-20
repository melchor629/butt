//
//  aacplus_encode.h
//  butt
//
//  Created by Melchor Garau Madrigal on 14/7/15.
//  Copyright (c) 2015 Daniel Nöthen. All rights reserved.
//

#ifndef __butt__aacplus_encode__
#define __butt__aacplus_encode__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <aacplus.h>

class buffer {
    size_t size, position;
public:
    uint8_t* buff;

    buffer(size_t size) {
        if(size != 0)
            buff = (uint8_t*) calloc(size, sizeof(short));
        this->size = size * sizeof(short);
        position = 0;
        //printf("[BUFF] Tamaño: %lu\n", this->size);
    }
    
    size_t append(void* ptr, size_t size) {
        if(position + size < this->size) {
            memcpy(&buff[position], ptr, size);
            position += size;
            //printf("[BUFF] Añadiendo %luB, usado %luB\n", size, position);
            return size;
        } else {
            size_t sobrante = this->size - position;
            memcpy(&buff[position], ptr, this->size - position);
            position = this->size;
            //printf("[BUFF] Añadiendo %luB, usado %luB (todo), sobrantes %luB\n", size, position, size-sobrante);
            return sobrante;
        }
    }
    
    size_t insert(size_t pos, void* ptr, size_t siz) {
        if(pos + siz < size) {
            memcpy(&buff[pos], ptr, siz);
            position = pos + siz;
            //printf("[BUFF] Insertando %luB en la posicion %lu, usado %lu\n", siz, pos, position);
            return siz;
        } else {
            memcpy(&buff[pos], ptr, size - pos);
            position = size;
            //printf("[BUFF] Insertando %luB en la posicion %lu, usado %lu (todo), sin copiar %luB\n", siz, pos, position, size-pos);
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

struct aacplus_enc {
    aacplusEncConfiguration* conf;
    aacplusEncHandle encoder;
    int bitrate;
    int samplerate;
    int channels;
    volatile int state;
    
    unsigned input_samples;
    unsigned max_output_bytes;
    buffer buff;
    
    aacplus_enc() : buff(0) {}
};

enum {
    AACPLUS_READY, AACPLUS_BUSY
};

int aacplus_enc_init(aacplus_enc *aacplus);
int aacplus_enc_encode(aacplus_enc *aacplus, short *pcm_buf, char* enc_buf, int samples, int channel);
int aacplus_enc_reinit(aacplus_enc *aacplus);
void aacplus_enc_close(aacplus_enc *aacplus);

#endif /* defined(__butt__aacplus_encode__) */
