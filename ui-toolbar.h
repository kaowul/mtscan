/*
 *  MTscan - MikroTik RouterOS wireless scanner
 *  Copyright (c) 2015-2017  Konrad Kosmatka
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#ifndef MTSCAN_UI_BUTTONS_H_
#define MTSCAN_UI_BUTTONS_H_
#include <gtk/gtk.h>

GtkWidget* ui_toolbar_create(void);

void ui_toolbar_connect_set_state(gboolean);
void ui_toolbar_mode_set_state(gint);

#endif
