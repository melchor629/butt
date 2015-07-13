// FLTK GUI related functions
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

#ifndef FL_FUNCS_H
#define FL_FUNCS_H

#include <FL/fl_ask.H>

#define PRINT_LCD(msg, home, clear) print_lcd(msg, home, clear)      //prints text to the LCD
#define GUI_LOOP() Fl::run();
#define CHECK_EVENTS() Fl::check()
#define ALERT(msg) fl_alert("%s", msg)

void fill_cfg_widgets(void);
void update_samplerates(void);
void print_info(const char* info, int info_type);

/**
 * Print a string to the LCD widget.
 * @param text String to print
 * @param home Set cursor position to the start
 * @param clear Clear the before content
 */
void print_lcd(const char *text, int home, int clear);

void test_file_extension(void);
void expand_string(char **str);
void init_main_gui_and_audio(void);

void init_choice_app(void*);
typedef const char* (*currentTrackFunction)(void);
extern currentTrackFunction getCurrentTrackFunctionFromId(int i);

#endif

