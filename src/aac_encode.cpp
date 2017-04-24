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
#include "DynamicLibraryLoad.hpp"

#if !defined(HAVE_LIBFDK_AAC)
static DynamicLibrary* fdk_lib = NULL;

typedef AACENC_ERROR (*Open)(
                        HANDLE_AACENCODER        *phAacEncoder,
                        const UINT                encModules,
                        const UINT                maxChannels
                        );

typedef AACENC_ERROR (*Close)(
                         HANDLE_AACENCODER        *phAacEncoder
                         );

typedef AACENC_ERROR (*Encode)(
                          const HANDLE_AACENCODER   hAacEncoder,
                          const AACENC_BufDesc     *inBufDesc,
                          const AACENC_BufDesc     *outBufDesc,
                          const AACENC_InArgs      *inargs,
                          AACENC_OutArgs           *outargs
                          );

typedef AACENC_ERROR (*SetParam)(
                                 const HANDLE_AACENCODER   hAacEncoder,
                                 const AACENC_PARAM        param,
                                 const UINT                value
                                 );

typedef AACENC_ERROR (*Info)(
                                const HANDLE_AACENCODER   hAacEncoder,
                                AACENC_InfoStruct        *pInfo
                                );

#define aacEncOpen _aacEncOpen
#define aacEncClose _aacEncClose
#define aacEncEncode _aacEncEncode
#define aacEncoder_SetParam _aacEncoder_SetParam
#define aacEncInfo _aacEncInfo
static Open aacEncOpen;
static Close aacEncClose;
static Encode aacEncEncode;
static SetParam aacEncoder_SetParam;
static Info aacEncInfo;
#endif

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
    
    return mode;
}

int aac_enc_init(aac_enc *aacplus) {
#ifndef HAVE_LIBFDK_AAC
    if(fdk_lib == NULL) {
        try {
            fdk_lib = new DynamicLibrary("libfdk-aac");
            aacEncOpen = fdk_lib->getFunctionPointer<Open>("aacEncOpen");
            aacEncClose = fdk_lib->getFunctionPointer<Close>("aacEncClose");
            aacEncEncode = fdk_lib->getFunctionPointer<Encode>("aacEncEncode");
            aacEncoder_SetParam = fdk_lib->getFunctionPointer<SetParam>("aacEncoder_SetParam");
            aacEncInfo = fdk_lib->getFunctionPointer<Info>("aacEncInfo");
        } catch (DynamicLibraryException &e) {
            char info_buf[100];
            snprintf(info_buf, sizeof(info_buf), "%s", _("Could not find libfdk installed. AAC support disabled"));
            print_info(info_buf, 1);
            fprintf(stderr, "%s\n", e.what());
            fdk_lib = NULL;
            return 1;
        }
    }
#endif
    
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
    
    return 0;
}

int aac_enc_encode(aac_enc *aacplus, short *pcm_buf, char* enc_buf, int samples, int size) {
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
}

int aac_enc_reinit(aac_enc *aacplus) {
    if(aacplus != NULL) {
        aac_enc_close(aacplus);
        return aac_enc_init(aacplus);
    }
    return 0;
}

void aac_enc_close(aac_enc *aacplus) {
    while(aacplus->state == AACPLUS_BUSY);
    
    if(aacplus->enc != NULL)
        aacEncClose(&aacplus->enc);
    
    memset(&aacplus->info, 0, sizeof(aacplus->info));
    aacplus->buff.destroy();
}

#if !defined(HAVE_LIBFDK_AAC)
void __attribute__((destructor)) aac_library_dealloc() {
    if(fdk_lib != NULL)
        delete fdk_lib;
}
#endif
