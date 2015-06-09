// FLTK callback functions for butt
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

#ifndef FL_CALLBACKS_H
#define FL_CALLBACKS_H

enum { STREAM_TIME = 0, REC_TIME, SENT_DATA, REC_DATA };
enum { STREAM = 0, RECORD };

class flgui;

extern int display_info;
extern flgui *fl_g;

void button_cfg_cb(void);
void button_info_cb(void);
void button_record_cb(void);
void button_connect_cb(void);
void choice_cfg_dev_cb(void);
void button_disconnect_cb(void);
void button_add_icy_add_cb(void);
void button_cfg_del_srv_cb(void);
void button_cfg_del_icy_cb(void);
void choice_cfg_act_srv_cb(void);
void choice_cfg_act_icy_cb(void);
void button_cfg_add_srv_cb(void);
void button_cfg_add_icy_cb(void);
void choice_cfg_bitrate_cb(void);
void choice_cfg_samplerate_cb(void);
void button_cfg_song_go_cb(void);
void choice_cfg_codec_mp3_cb(void);
void choice_cfg_codec_ogg_cb(void);
void choice_cfg_codec_opus_cb(void);
void button_cfg_export_cb(void);
void button_cfg_import_cb(void);
void button_add_icy_save_cb(void);
void button_add_srv_cancel_cb(void);
void button_add_icy_cancel_cb(void);
void choice_cfg_channel_stereo_cb(void);
void choice_cfg_channel_mono_cb(void);
void button_cfg_browse_songfile_cb(void);
void input_cfg_song_file_cb(void);
void input_cfg_song_cb(void);
void input_cfg_buffer_cb(bool print_message);
void choice_cfg_resample_mode_cb(void);

void button_add_srv_add_cb(void);
void button_add_srv_save_cb(void);
void button_add_srv_show_pwd_cb(void);
void radio_add_srv_shoutcast_cb(void);
void radio_add_srv_icecast_cb(void);

void button_rec_browse_cb(void);
void choice_rec_bitrate_cb(void);
void choice_rec_samplerate_cb(void);
void choice_rec_channel_stereo_cb(void);
void choice_rec_channel_mono_cb(void);
void choice_rec_codec_mp3_cb(void);
void choice_rec_codec_ogg_cb(void);
void choice_rec_codec_wav_cb(void);
void choice_rec_codec_opus_cb(void);
void choice_rec_codec_flac_cb(void);
void button_cfg_edit_srv_cb(void);
void button_cfg_edit_icy_cb(void);
void check_song_update_active_cb(void);
void check_sync_to_full_hour_cb(void);

void input_rec_filename_cb(void);
void input_rec_folder_cb(void);
void input_rec_split_time_cb(void);
void input_log_filename_cb(void);
void button_cfg_log_browse_cb(void);

void check_gui_attach_cb(void);
void check_gui_ontop_cb(void);
void check_gui_lcd_auto_cb(void);
void button_gui_bg_color_cb(void);
void button_gui_text_color_cb(void);

void slider_gain_cb(void);

void check_cfg_rec_cb(void);
void check_cfg_rec_hourly_cb(void);
void check_cfg_connect_cb(void);

void lcd_rotate(void*);
void ILM216_cb(void);
void window_main_close_cb(void);

#endif

