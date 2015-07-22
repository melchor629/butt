//
//  aac_encode.cpp
//  butt
//
//  Created by Melchor Garau Madrigal on 14/7/15.
//  Copyright (c) 2015 Daniel Nöthen. All rights reserved.
//

#include "aac_encode.h"
#include "fl_funcs.h"
#include "util.h"
#include <cstring>
#include <stdlib.h>
#include <stdio.h>

int fdk_select_profile(aac_enc *aacplus) {
    const int HE_AACv2 = 29, HE_AAC = 5, AAC_LC = 2;
    int ch = aacplus->channels;
    int br = aacplus->bitrate;
    int sr = aacplus->samplerate;
    int mode = AAC_LC;
    
    if(ch == 2) {
        switch(sr) {
            case 22050:
            case 24000:
                if(br >= 8000 && br <= 11999) mode = HE_AACv2;
                break;
                
            case 32000:
                if(br >= 12000 && br <= 17999) mode =  HE_AACv2;
            
            case 44100:
            case 48000:
                if(br >= 18000 && br <= 56000) mode =  HE_AACv2;
                if(br > 56000 && br <= 128000) mode =  HE_AAC;
                break;
        }
    } else if(ch == 1) {
        switch(sr) {
            case 22050:
            case 24000:
                if(br >= 8000 &&  br <= 11999) mode =  HE_AAC;
                break;
            
            case 32000:
                if(br >= 12000 && br <= 17999) mode =  HE_AAC;
            
            case 44100:
            case 48000:
                if(br >= 18000 && br <= 56000) mode =  HE_AAC;
        }
    }
    
    printf("AAC MODE: %s\n", mode == HE_AACv2 ? "HE-AACv2" : mode == HE_AAC ? "HE-AAC" : "AAC-LC");
    
    return mode;
}

int aac_enc_init(aac_enc *aacplus) {
#if HAVE_LIBAACPLUS
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
        ret = 1;
    }

    aacplus->buff = buffer(aacplus->input_samples); // Los samples en este codificador no son samples
    aacplus->state = AACPLUS_READY;
    return ret;
    
#elif HAVE_LIBFDK_AAC
    
    if(aacEncOpen(&aacplus->enc, 0, aacplus->channels) != AACENC_OK) return 1;
    if(aacEncoder_SetParam(aacplus->enc, AACENC_AOT, fdk_select_profile(aacplus)) != AACENC_OK) return 1;
    if(aacEncoder_SetParam(aacplus->enc, AACENC_SAMPLERATE, aacplus->samplerate) != AACENC_OK) return 1;
    if(aacEncoder_SetParam(aacplus->enc, AACENC_CHANNELMODE, aacplus->channels == 1 ? MODE_1 : MODE_2) != AACENC_OK) return 1;
    if(aacEncoder_SetParam(aacplus->enc, AACENC_CHANNELORDER, 1) != AACENC_OK) return 1;
    if(aacEncoder_SetParam(aacplus->enc, AACENC_BITRATE, aacplus->bitrate) != AACENC_OK) return 1;
    if(aacEncoder_SetParam(aacplus->enc, AACENC_TRANSMUX, 2) != AACENC_OK) return 1;
    if(aacEncoder_SetParam(aacplus->enc, AACENC_AFTERBURNER, 1) != AACENC_OK) return 1;
    if(aacEncEncode(aacplus->enc, NULL, NULL, NULL, NULL) != AACENC_OK) return 1;
    
    if(aacEncInfo(aacplus->enc, &aacplus->info) != AACENC_OK) return 1;
    
    aacplus->buff = buffer(aacplus->channels * aacplus->info.frameLength);
    aacplus->input_samples = aacplus->info.frameLength;
    printf("SAMPLES: %d\n", aacplus->info.frameLength);
    
    return 0;
    
#else
    
    char info_buf[100];
    snprintf(info_buf, sizeof(info_buf), "%s", _("No se ha compilado con libaacplus"));
    print_info(info_buf, 1);
    return 1;
#endif
}

int aac_enc_encode(aac_enc *aacplus, short *pcm_buf, char* enc_buf, int samples, int size) {
#if HAVE_LIBAACPLUS
    if(size == 0 || aacplus->encoder == NULL)
        return 0;
    int bytes = 0;
    size_t size_pcm_buf = samples * aacplus->channels * 2;
    size = aacplus->max_output_bytes;
    uint8_t* audio = (uint8_t*) pcm_buf;
    aacplus->state = AACPLUS_BUSY;
    
    if(samples * aacplus->channels < aacplus->input_samples) {
        size_t bytes_copied = aacplus->buff.append(pcm_buf, size_pcm_buf);
        if(bytes_copied < size_pcm_buf) {
            bytes = aacplusEncEncode(aacplus->encoder, aacplus->buff.get<int>(), aacplus->input_samples, (uint8_t*) enc_buf, size);
            enc_buf[1] |= 1 << 3;
            bytes_copied += aacplus->buff.insert(0, &audio[bytes_copied], size_pcm_buf - bytes_copied);
            if(size_pcm_buf != bytes_copied)
                printf("SOBRAN BYTES POR AQUI Y NO DEBERÍA %lu\n", size_pcm_buf - bytes_copied);
        }
    } else {
        size_t bytes_copied = 0;
        while(bytes_copied != size_pcm_buf) {
            bytes_copied += aacplus->buff.append(pcm_buf, size_pcm_buf);
            int old_bytes = bytes;
            bytes += aacplusEncEncode(aacplus->encoder, aacplus->buff.get<int>(), aacplus->input_samples, (uint8_t*) &enc_buf[bytes], size);
            enc_buf[old_bytes+1] |= 1 << 3;
            bytes_copied += aacplus->buff.insert(0, &audio[bytes_copied], size_pcm_buf - bytes_copied);
        }
    }
    
    aacplus->state = AACPLUS_READY;
    return bytes;
    
#elif HAVE_LIBFDK_AAC
    AACENC_BufDesc in_buf = {0}, out_buf = {0};
    AACENC_InArgs in_args = {0};
    AACENC_OutArgs out_args = {0};
    int in_identifier = IN_AUDIO_DATA, in_elem_size = 2, in_size = samples * sizeof(short) * aacplus->channels;
    int out_identifier = OUT_BITSTREAM_DATA, out_elem_size = 1, out_size = aacplus->info.maxOutBufBytes;
    const void* thaVoid = aacplus->buff.get<void>();
    aacplus->state = AACPLUS_BUSY;
    
    in_args.numInSamples = samples * aacplus->channels;
    in_buf.numBufs = 1;
    in_buf.bufs = (void**) &thaVoid;
    in_buf.bufferIdentifiers = &in_identifier;
    in_buf.bufSizes = &in_size;
    in_buf.bufElSizes = &in_elem_size;

    out_buf.numBufs = 1;
    out_buf.bufs = (void**) &enc_buf;
    out_buf.bufferIdentifiers = &out_identifier;
    out_buf.bufSizes = &out_size;
    out_buf.bufElSizes = &out_elem_size;

    int bytes = 0;
    uint8_t* audio = (uint8_t*) pcm_buf;
    
    if(samples * aacplus->channels < aacplus->input_samples) {
        size_t bytes_copied = aacplus->buff.append(pcm_buf, in_size);
        if(bytes_copied < in_size) {
            aacEncEncode(aacplus->enc, &in_buf, &out_buf, &in_args, &out_args);
            bytes += out_args.numOutBytes;
            bytes_copied += aacplus->buff.insert(0, &audio[bytes_copied], in_size - bytes_copied);
            if(in_size != bytes_copied)
                printf("SOBRAN BYTES POR AQUI Y NO DEBERÍA %lu\n", in_size - bytes_copied);
        }
    } else {
        size_t bytes_copied = 0;
        void* ptr;
        out_buf.bufs = &ptr;
        while(bytes_copied != in_size) {
            bytes_copied += aacplus->buff.append(pcm_buf, in_size);
            ptr = &enc_buf[bytes];
            aacEncEncode(aacplus->enc, &in_buf, &out_buf, &in_args, &out_args);
            bytes += out_args.numOutBytes;
            bytes_copied += aacplus->buff.insert(0, &audio[bytes_copied], in_size - bytes_copied);
        }
    }
    
    aacplus->state = AACPLUS_READY;
    return bytes;
    
#else
    
    return -1;
#endif
}

int aac_enc_reinit(aac_enc *aacplus) {
    if(aacplus != NULL) {
        aac_enc_close(aacplus);
        return aac_enc_init(aacplus);
    }
    return 0;
}

void aac_enc_close(aac_enc *aacplus) {
#if HAVE_LIBAACPLUS
    while(aacplus->state == AACPLUS_BUSY);
    
    if(aacplus->encoder != NULL)
        aacplusEncClose(aacplus->encoder);
    
    aacplus->buff.destroy();
    aacplus->conf = NULL;
    aacplus->encoder = NULL;
#elif HAVE_LIBFDK_AAC
    while(aacplus->state == AACPLUS_BUSY);
    
    if(aacplus->enc != NULL)
        aacEncClose(&aacplus->enc);
    
    memset(&aacplus->info, 0, sizeof(aacplus->info));
    aacplus->buff.destroy();
#endif
}
