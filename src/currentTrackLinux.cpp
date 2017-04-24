#include "currentTrack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <dbus/dbus.h>

static DBusError err;
static DBusConnection* conn = NULL;

static bool dbus_init(void) {
    if(conn == NULL) {
        int ret;
        dbus_error_init(&err);
        conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
        if(dbus_error_is_set(&err)) {
            printf("Connection Error: %s\n", err.message);
            dbus_error_free(&err);
            return false;
        }
        
        ret = dbus_bus_request_name(conn, "me.melchor9000.butt",
            DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
        if(dbus_error_is_set(&err)) {
            printf("Name error: %s\n", err.message);
            dbus_error_free(&err);
        }
        if(ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
            return false;
        }
    }
}

struct DBusMethodCall {
    const char* target;
    const char* object;
    const char* interface;
    const char* method;
    void (*arg_iter)(void* data, DBusMessageIter* args);
    void* data;
};

static bool dbus_call_method(const struct DBusMethodCall* opt, unsigned n, ...) {
    DBusMessage* msg;
    DBusMessageIter args;
    DBusPendingCall* pending;

    va_list va;
    va_start(va, n);
    
    msg = dbus_message_new_method_call(opt->target, opt->object, opt->interface, opt->method);
    if(msg == NULL) {
        printf("Message Null\n");
        for(unsigned i = 0; i < n; i++) va_arg(va, void*);
        va_end(va);
        return false;
    }
    
    dbus_message_iter_init_append(msg, &args);
    
    for(unsigned i = 0; i < n; i++) {
        const char* arg = va_arg(va, const char*);
        dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &arg);
    }
    
    va_end(va);
    dbus_connection_send_with_reply(conn, msg, &pending, -1);
    if(pending == NULL) {
        printf("Pending call null\n");
        return false;
    }
    
    dbus_connection_flush(conn);
    dbus_message_unref(msg);
    
    dbus_pending_call_block(pending);
    msg = dbus_pending_call_steal_reply(pending);
    if(msg == NULL) {
        printf("Reply NULL\n");
        return false;
    }
    
    dbus_pending_call_unref(pending);

    if(dbus_message_get_type(msg) != DBUS_MESSAGE_TYPE_ERROR) {
        if(dbus_message_iter_init(msg, &args)) {
            do {
                if(opt->arg_iter) opt->arg_iter(opt->data, &args);
            } while(dbus_message_iter_next(&args));
        }

        dbus_message_unref(msg);
        return true;
    } else {
        dbus_message_unref(msg);
        return false;
    }
}

static void playbackstatus(void* data, DBusMessageIter* args) {
    DBusMessageIter var;
    char* str;
    dbus_message_iter_recurse(args, &var);
    dbus_message_iter_get_basic(&var, &str);
    *((char**) data) = strdup(str);
}

static void metadata(void* data, DBusMessageIter* args) {
    DBusMessageIter var, arr, var2, var3, arr2;
    char* str = NULL;
    char** values = (char**) data;
    dbus_message_iter_recurse(args, &var);

    dbus_message_iter_recurse(&var, &arr);
    do {
        dbus_message_iter_recurse(&arr, &var2);
        dbus_message_iter_get_basic(&var2, &str);
        dbus_message_iter_next(&var2);
        if(values[1] == NULL && !strcmp("xesam:artist", str)) {
            dbus_message_iter_recurse(&var2, &var3);
            dbus_message_iter_recurse(&var3, &arr2);
            values[1] = (char*) calloc(1, 1);
            int s = 1, didFirst = 0;
            do {
                dbus_message_iter_get_basic(&arr2, &str);
                if(strlen(str)) {
                    s += strlen(str) + (didFirst ? 2 : 0);
                    values[1] = (char*) realloc(values[1], s);
                    if(didFirst) strcat(values[1], ", ");
                    strcat(values[1], str);
                    if(!didFirst) didFirst = 1;
                }
            } while(dbus_message_iter_next(&arr2));
        } else if(values[0] == NULL && !strcmp("xesam:title", str)) {
            dbus_message_iter_recurse(&var2, &var3);
            dbus_message_iter_get_basic(&var3, &str);
            values[0] = strdup(str);
        }
    } while(dbus_message_iter_next(&arr));
}

static char* get_mpris(const char* target) {
    char* playbackStatusValue = NULL;
    char* returnString = NULL;
    struct DBusMethodCall opt = {
        target,
        "/org/mpris/MediaPlayer2",
        "org.freedesktop.DBus.Properties",
        "Get",
        playbackstatus,
        &playbackStatusValue
    };

    dbus_init();
    dbus_call_method(&opt, 2, "org.mpris.MediaPlayer2.Player", "PlaybackStatus");
    if(playbackStatusValue != NULL && strcmp(playbackStatusValue, "Stopped")) {
        char* values[2] = { NULL, NULL };
        opt.arg_iter = metadata;
        opt.data = values;
        dbus_call_method(&opt, 2, "org.mpris.MediaPlayer2.Player", "Metadata");
        returnString = (char*) malloc(strlen(values[1]) + 3 + strlen(values[0]) + 1);
        strcat(returnString, values[1]);
        strcat(returnString, " - ");
        strcat(returnString, values[0]);
    }

    free(playbackStatusValue);
    return returnString;
}

const char* getCurrentTrackRhythmbox() {
    return get_mpris("org.gnome.Rhythmbox3");
}

const char* getCurrentTrackClementine() {
    return get_mpris("org.mpris.MediaPlayer2.clementine");
}

const char* getCurrentTrackSpotify() {
    return get_mpris("org.mpris.MediaPlayer2.spotify");
}

const char* getCurrentTrackCantata() {
    return get_mpris("org.mpris.MediaPlayer2.cantata");
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
        case 2: return &getCurrentTrackClementine;
        case 3: return &getCurrentTrackSpotify;
        case 4: return &getCurrentTrackCantata;
        default: return NULL;
    }
}
