// shoutcast functions for butt
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
#include <string.h>

#ifdef _WIN32
 #define usleep(us) Sleep(us/1000)
#else
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <unistd.h>
 #include <netinet/in.h> //defines IPPROTO_TCP on BSD
 #include <netdb.h>
#endif

#include <errno.h>

#include "cfg.h"
#include "butt.h"
#include "timer.h"
#include "strfuncs.h"
#include "shoutcast.h"
#include "parseconfig.h"
#include "sockfuncs.h"
#include "flgui.h"
#include "fl_funcs.h"

void send_icy_header(char *key, char *val)
{
    char *icy_line;
    int len;

    if((val == NULL) || (strlen(val) == 0))
        return;

    len = strlen(key) + strlen(val) + strlen(":\r\n") + 1;
    icy_line = (char*)malloc(len * sizeof(char)+1);
    snprintf(icy_line, len, "%s:%s\r\n", key, val);
    
    sock_send(&stream_socket, icy_line, strlen(icy_line), SEND_TIMEOUT);

    free(icy_line);
}

int sc_connect(void)
{
    int ret;
    char recv_buf[100];
    char send_buf[100];
    static bool error_printed = 0;

    stream_socket = sock_connect(cfg.srv[cfg.selected_srv]->addr,
            cfg.srv[cfg.selected_srv]->port+1, CONN_TIMEOUT);

    if(stream_socket < 0)
    {
        switch(stream_socket)
        {
            case SOCK_ERR_CREATE:
                if(!error_printed)
                {
                    print_info("\nConnect: Could not create network socket", 1);
                    error_printed = 1;
                }
                ret = 2;
                break;
            case SOCK_ERR_RESOLVE:
                if(!error_printed)
                {
                    print_info("\nConnect: Error resolving server address", 1);
                    error_printed = 1;
                }
                ret = 1;
                break;
            case SOCK_TIMEOUT:
            case SOCK_INVALID:
                ret = 1;
                break;
            default:
                ret = 2;
        }

        sc_disconnect();
        return ret;
    }

    error_printed = 0;

    sock_send(&stream_socket, cfg.srv[cfg.selected_srv]->pwd,
            strlen(cfg.srv[cfg.selected_srv]->pwd), SEND_TIMEOUT);
    sock_send(&stream_socket, "\n", 1, SEND_TIMEOUT);


   if(cfg.main.num_of_icy > 0)
    {
        send_icy_header((char*)"icy-name", cfg.icy[cfg.selected_icy]->name);
        send_icy_header((char*)"icy-genre", cfg.icy[cfg.selected_icy]->genre);
        send_icy_header((char*)"icy-url", cfg.icy[cfg.selected_icy]->url);
        send_icy_header((char*)"icy-irc", cfg.icy[cfg.selected_icy]->irc);
        send_icy_header((char*)"icy-icq", cfg.icy[cfg.selected_icy]->icq);
        send_icy_header((char*)"icy-aim", cfg.icy[cfg.selected_icy]->aim);
        send_icy_header((char*)"icy-pub", cfg.icy[cfg.selected_icy]->pub);
    }
    else
    {
        send_icy_header((char*)"icy-name", (char*)"No Name");
        send_icy_header((char*)"icy-pub", (char*)"0");
    }


    snprintf(send_buf, sizeof(send_buf), "%u", cfg.audio.bitrate);
    send_icy_header((char*)"icy-br", send_buf);

    sock_send(&stream_socket, "\n\n", 2, SEND_TIMEOUT);

    /*
    sock_send(&stream_socket, "content-type:", 13, SEND_TIMEOUT);

    if(!strcmp(cfg.audio.codec, "mp3"))
    {
        strcpy(send_buf, "audio/mpeg");
        sock_send(&stream_socket, send_buf, strlen(send_buf), SEND_TIMEOUT);
    }*/


    if((ret = sock_recv(&stream_socket, recv_buf, sizeof(recv_buf)-1, RECV_TIMEOUT)) == 0)
    {
        usleep(100*1000);
        sc_disconnect();
        return 1;
    }

    recv_buf[ret] = '\0';
    

    if( (recv_buf[0] != 'O') || (recv_buf[1] != 'K') || (ret <= 2) )
    {
        if(strstr(recv_buf, "invalid password") != NULL)
        {
            print_info("\nConnect: Invalid password!\n", 1);
            sc_disconnect();
            return 2;
        }
        return 1;
    }



 

    connected = 1;

    timer_init(&stream_timer, 1);       //starts the "online" timer

    return 0;
}

int sc_send(char *buf, int buf_len)
{
    int ret;
    ret = sock_send(&stream_socket, buf, buf_len, SEND_TIMEOUT);

    if(ret == SOCK_TIMEOUT)
        ret = -1;

    return ret;
}

int sc_update_song(void)
{
    int ret;
    int web_socket;
    char send_buf[1024];
    char *song_buf;

    web_socket = sock_connect(cfg.srv[cfg.selected_srv]->addr,
            cfg.srv[cfg.selected_srv]->port, CONN_TIMEOUT);

    if(web_socket < 0)
    {
        switch(web_socket)
        {
            case SOCK_ERR_CREATE:
                print_info("\nUpdate song: Could not create network socket", 1);
                ret = 1;
                break;
            case SOCK_ERR_RESOLVE:
                print_info("\nUpdate song: Error resolving server address", 1);
                ret = 1;
                break;
            case SOCK_TIMEOUT:
            case SOCK_INVALID:
                ret = 1;
                break;
            default:
                ret = 1;
        }

        return ret;
    }

    song_buf = strdup(cfg.main.song);

    strrpl(&song_buf, (char*)" ", (char*)"%20", MODE_ALL);
    strrpl(&song_buf, (char*)"&", (char*)"%26", MODE_ALL);

    snprintf(send_buf, 500, "GET /admin.cgi?pass=%s&mode=updinfo&song=%s&url= HTTP/1.0\n"
                      "User-Agent: ShoutcastDSP (Mozilla Compatible)\n\n",
                      cfg.srv[cfg.selected_srv]->pwd,
                      song_buf
                     );

    sock_send(&web_socket, send_buf, strlen(send_buf), SEND_TIMEOUT);
    sock_close(&web_socket);
    free(song_buf);

    return 0;
}

void sc_disconnect(void)
{
    sock_close(&stream_socket);
}

