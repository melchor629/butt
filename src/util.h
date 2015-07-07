// string manipulation functions for butt
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
//
#ifndef UTIL_H
#define UTIL_H

#include <libintl.h>
#include <FL/Fl_Widget.H>

#define _(a) static_cast<const char*>(gettext(a))

char *util_base64_enc(char *data);
char *util_get_file_extension(char *filename);
float util_factor_to_db(float factor);
float util_db_to_factor(float dB);
void util_set_visible(Fl_Widget* w);

#endif
