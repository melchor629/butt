#include "currentTrack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//https://snipt.net/raw/116eb6f92db87aa7909ff26da07e8430/?nice
const char* getCurrentTrackRhythmbox() {
    FILE* p = popen("rhythmbox-client --print-playing-format=\"%tt - %ta\"", "r");
    if(p != NULL) {
        char* buffStr = (char*) calloc(1, 512);
        char* bread = fgets(buffStr, 512, p);
        buffStr[strlen(buffStr)-1] = '\0';
        int ret = pclose(p);
        if(ret == 0 && bread != NULL) {
            return buffStr;
        } else {
            return NULL;
        }
    } else {
        return NULL;
    }
}

const char* getCurrentTrackBanshee() {
    FILE* p = popen("banshee --query-artist", "r");
    if(p != NULL) {
        char buff1[512];
        memset(buff1, 0, 512);
        char* bread = fgets(buff1, 512, p);
        buff1[strlen(buff1) - 1] = '\0';
        int ret = pclose(p);
        p = popen("banshee --query-title", "r");
        if(bread != NULL && ret == 0 && p != NULL) {
            char buff2[512];
            memset(buff2, 0, 512);
            bread = fgets(buff2, 512, p);
            buff2[strlen(buff2) - 1] = '\0';
            ret = pclose(p);

            if(ret == 0 && bread != NULL) {
                size_t size1 = strlen(buff1) - 8, size2 = strlen(buff2) - 7;
                char* str = (char*) calloc(1, size1 + size2 + 3 + 1);
                strcat(str, &buff1[8]);
                strcat(str, " - ");
                strcat(str, &buff2[7]);
                return str;
            } else {
                return NULL;
            }
        } else {
            return NULL;
        }
    } else {
        return NULL;
    }
}

currentTrackFunction getCurrentTrackFunctionFromId(int i) {
    switch(i) {
        case 0: return &getCurrentTrackRhythmbox;
        case 1: return &getCurrentTrackBanshee;
        default: return NULL;
    }
}
