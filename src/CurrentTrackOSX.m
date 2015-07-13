//
//  CurrentTrackOSX.m
//  butt
//
//  Created by Melchor Garau Madrigal on 8/7/15.
//  Copyright (c) 2015 Daniel Nöthen. All rights reserved.
//

#import "CurrentTrackOSX.h"
#import <iTunes.h>
#import <Spotify.h>
#import <VOX.h>

const char* getCurrentTrackFromiTunes() {
    char* ret = NULL;
    iTunesApplication *iTunes = [SBApplication applicationWithBundleIdentifier:@"com.apple.itunes"];
    if([iTunes isRunning]) {
        if([iTunes playerState] != iTunesEPlSStopped) {
            NSString* track = [NSString stringWithFormat:@"%@ - %@", [[iTunes currentTrack] name], [[iTunes currentTrack] artist]];
            ret = (char*) malloc([track length] + 1);
            [track getCString:ret maxLength:([track length] + 1) encoding:NSUTF8StringEncoding];
        }
    }
    
    return ret;
}

const char* getCurrentTrackFromSpotify() {
    char* ret = NULL;
    SpotifyApplication *spotify = [SBApplication applicationWithBundleIdentifier:@"com.spotify.client"];
    if([spotify isRunning]) {
        if([spotify playerState] != SpotifyEPlSStopped) {
            NSString* track = [NSString stringWithFormat:@"%@ - %@", [[spotify currentTrack] name], [[spotify currentTrack] artist]];
            ret = (char*) malloc([track length] + 1);
            [track getCString:ret maxLength:([track length] + 1) encoding:NSUTF8StringEncoding];
        }
    }
    
    return ret;
}

const char* getCurrentTrackFromVOX() {
    char* ret = NULL;
    VOXApplication *vox = [SBApplication applicationWithBundleIdentifier:@"com.coppertino.Vox"];
    if([vox isRunning]) {
        if([vox playerState]) {
            NSString* track = [NSString stringWithFormat:@"%@ - %@", [vox track], [vox artist]];
            ret = (char*) malloc([track length] + 1);
            [track getCString:ret maxLength:([track length] + 1) encoding:NSUTF8StringEncoding];
        }
    }
    
    return ret;
}

typedef const char* (*currentTrackFunction)(void);
currentTrackFunction getCurrentTrackFunctionFromId(int i) {
    switch(i) {
        case 0: return &getCurrentTrackFromiTunes;
        case 1: return &getCurrentTrackFromSpotify;
        case 2: return getCurrentTrackFromVOX;
        default: return NULL;
    }
}