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

#include <gdk/gdkkeysyms.h>
#include <string.h>
#include <math.h>
#include "conf.h"
#include "ui.h"
#include "ui-view.h"
#include "ui-view-menu.h"
#include "ui-icons.h"
#include "ui-dialogs.h"
#include "signals.h"

static gboolean ui_view_popup(GtkWidget*, gpointer);
static gboolean ui_view_clicked(GtkWidget*, GdkEventButton*, gpointer);
static gboolean ui_view_key_press(GtkWidget*, GdkEventKey*, gpointer);
static void ui_view_size_alloc(GtkWidget *widget, GtkAllocation *allocation, gpointer);
static gboolean ui_view_disable_motion_redraw(GtkWidget*, GdkEvent*, gpointer);
static void ui_view_format_background(GtkTreeViewColumn*, GtkCellRenderer*, GtkTreeModel*, GtkTreeIter* , gpointer);
static void ui_view_format_icon(GtkTreeViewColumn*, GtkCellRenderer*, GtkTreeModel*, GtkTreeIter*, gpointer);
static void ui_view_format_address(GtkTreeViewColumn*, GtkCellRenderer*, GtkTreeModel*, GtkTreeIter*, gpointer);
static void ui_view_format_freq(GtkTreeViewColumn*, GtkCellRenderer*, GtkTreeModel*, GtkTreeIter*, gpointer);
static void ui_view_format_date(GtkTreeViewColumn*, GtkCellRenderer*, GtkTreeModel*, GtkTreeIter*, gpointer);
static void ui_view_format_level(GtkTreeViewColumn*, GtkCellRenderer*, GtkTreeModel*, GtkTreeIter*, gpointer);
static void ui_view_format_gps(GtkTreeViewColumn*, GtkCellRenderer*, GtkTreeModel*, GtkTreeIter*, gpointer);
static void ui_view_format_azimuth(GtkTreeViewColumn*, GtkCellRenderer*, GtkTreeModel*, GtkTreeIter*, gpointer);
static gboolean ui_view_compare_string(GtkTreeModel*, gint, const gchar*, GtkTreeIter*, gpointer);
static void ui_view_column_clicked(GtkTreeViewColumn*, gpointer);

GtkWidget*
ui_view_new(mtscan_model_t *model,
            gint            icon_size)
{
    GtkWidget *treeview;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model->store));
    gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)), GTK_SELECTION_MULTIPLE);
    g_object_set_data(G_OBJECT(treeview), "mtscan-model", model);
    g_object_set_data(G_OBJECT(treeview), "mtscan-sort", GINT_TO_POINTER(-1));

    /* Activity column */
    renderer = gtk_cell_renderer_pixbuf_new();
    gtk_cell_renderer_set_padding(renderer, 0, 0);
    gtk_cell_renderer_set_fixed_size(renderer, icon_size, icon_size);
    g_object_set_data(G_OBJECT(treeview), "mtscan-icon-renderer", renderer);
    column = gtk_tree_view_column_new_with_attributes(MODEL_TEXT_ICON, renderer, NULL);
    gtk_tree_view_column_set_clickable(column, TRUE);
    g_signal_connect(column, "clicked", (GCallback)ui_view_column_clicked, GINT_TO_POINTER(COL_PRIVACY));
    gtk_tree_view_column_set_cell_data_func(column, renderer, ui_view_format_icon, NULL, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    /* Address column */
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_renderer_set_padding(renderer, 0, 0);
    g_object_set(renderer, "font", "Dejavu Sans Mono", NULL);
    gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer), 1);
    column = gtk_tree_view_column_new_with_attributes(MODEL_TEXT_ADDRESS, renderer, NULL);
    gtk_tree_view_column_set_clickable(column, TRUE);
    g_signal_connect(column, "clicked", (GCallback)ui_view_column_clicked, GINT_TO_POINTER(COL_ADDRESS));
    gtk_tree_view_column_set_cell_data_func(column, renderer, ui_view_format_address, GINT_TO_POINTER(COL_ADDRESS), NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    /* Frequency column */
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_renderer_set_padding(renderer, 1, 0);
    gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer), 1);
    column = gtk_tree_view_column_new_with_attributes(MODEL_TEXT_FREQUENCY, renderer, NULL);
    gtk_tree_view_column_set_clickable(column, TRUE);
    g_signal_connect(column, "clicked", (GCallback)ui_view_column_clicked, GINT_TO_POINTER(COL_FREQUENCY));
    gtk_tree_view_column_set_cell_data_func(column, renderer, ui_view_format_freq, GINT_TO_POINTER(COL_FREQUENCY), NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    /* Mode column */
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_renderer_set_padding(renderer, 1, 0);
    gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer), 1);
    column = gtk_tree_view_column_new_with_attributes(MODEL_TEXT_MODE, renderer, "text", COL_MODE, NULL);
    gtk_tree_view_column_set_clickable(column, TRUE);
    g_signal_connect(column, "clicked", (GCallback)ui_view_column_clicked, GINT_TO_POINTER(COL_MODE));
    gtk_tree_view_column_set_cell_data_func(column, renderer, ui_view_format_background, GINT_TO_POINTER(COL_MODE), NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    /* Channel width column */
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_renderer_set_padding(renderer, 1, 0);
    gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer), 1);
    column = gtk_tree_view_column_new_with_attributes(MODEL_TEXT_CHANNEL, renderer, "text", COL_CHANNEL, NULL);
    gtk_tree_view_column_set_clickable(column, TRUE);
    g_signal_connect(column, "clicked", (GCallback)ui_view_column_clicked, GINT_TO_POINTER(COL_CHANNEL));
    gtk_tree_view_column_set_cell_data_func(column, renderer, ui_view_format_background, GINT_TO_POINTER(COL_CHANNEL), NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    /* SSID column */
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_renderer_set_padding(renderer, 2, 0);
    gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer), 1);
    column = gtk_tree_view_column_new_with_attributes(MODEL_TEXT_SSID, renderer, "text", COL_SSID, NULL);
    gtk_tree_view_column_set_clickable(column, TRUE);
    gtk_tree_view_column_set_expand(column, TRUE);
    g_signal_connect(column, "clicked", (GCallback)ui_view_column_clicked, GINT_TO_POINTER(COL_SSID));
    gtk_tree_view_column_set_cell_data_func(column, renderer, ui_view_format_background, GINT_TO_POINTER(COL_SSID), NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    /* Radio name column */
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_renderer_set_padding(renderer, 2, 0);
    gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer), 1);
    column = gtk_tree_view_column_new_with_attributes(MODEL_TEXT_RADIONAME, renderer, "text", COL_RADIONAME, NULL);
    gtk_tree_view_column_set_clickable(column, TRUE);
    g_signal_connect(column, "clicked", (GCallback)ui_view_column_clicked, GINT_TO_POINTER(COL_RADIONAME));
    gtk_tree_view_column_set_cell_data_func(column, renderer, ui_view_format_background, GINT_TO_POINTER(COL_RADIONAME), NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    /* Max signal column */
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_renderer_set_padding(renderer, 3, 0);
    gtk_cell_renderer_set_alignment(renderer, 1.0, 0.5);
    gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer), 1);
    column = gtk_tree_view_column_new_with_attributes(MODEL_TEXT_MAXRSSI, renderer, NULL);
    gtk_tree_view_column_set_clickable(column, TRUE);
    g_signal_connect(column, "clicked", (GCallback)ui_view_column_clicked, GINT_TO_POINTER(COL_MAXRSSI));
    gtk_tree_view_column_set_cell_data_func(column, renderer, ui_view_format_level, GINT_TO_POINTER(COL_MAXRSSI), NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    /* Signal column */
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_renderer_set_padding(renderer, 3, 0);
    gtk_cell_renderer_set_alignment(renderer, 1.0, 0.5);
    gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer), 1);
    column = gtk_tree_view_column_new_with_attributes(MODEL_TEXT_RSSI, renderer, NULL);
    gtk_tree_view_column_set_clickable(column, TRUE);
    g_signal_connect(column, "clicked", (GCallback)ui_view_column_clicked, GINT_TO_POINTER(COL_RSSI));
    gtk_tree_view_column_set_cell_data_func(column, renderer, ui_view_format_level, GINT_TO_POINTER(COL_RSSI), NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    /* Noise floor column */
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_renderer_set_padding(renderer, 3, 0);
    gtk_cell_renderer_set_alignment(renderer, 1.0, 0.5);
    gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer), 1);
    column = gtk_tree_view_column_new_with_attributes(MODEL_TEXT_NOISE, renderer, NULL);
    gtk_tree_view_column_set_clickable(column, TRUE);
    g_signal_connect(column, "clicked", (GCallback)ui_view_column_clicked, GINT_TO_POINTER(COL_NOISE));
    gtk_tree_view_column_set_cell_data_func(column, renderer, ui_view_format_level, GINT_TO_POINTER(COL_NOISE), NULL);
    g_object_set_data(G_OBJECT(treeview), "mtscan-column-noise", column);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    /* Privacy column */
    renderer = gtk_cell_renderer_toggle_new();
    gtk_cell_renderer_set_padding(renderer, 0, 0);
    column = gtk_tree_view_column_new_with_attributes(MODEL_TEXT_PRIVACY, renderer, "active", COL_PRIVACY, NULL);
    gtk_tree_view_column_set_clickable(column, TRUE);
    g_signal_connect(column, "clicked", (GCallback)ui_view_column_clicked, GINT_TO_POINTER(COL_PRIVACY));
    gtk_tree_view_column_set_visible(column, FALSE);
    gtk_tree_view_column_set_cell_data_func(column, renderer, ui_view_format_background, GINT_TO_POINTER(COL_PRIVACY), NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    /* Mikrotik RouterOS column */
    renderer = gtk_cell_renderer_toggle_new();
    gtk_cell_renderer_set_padding(renderer, 1, 0);
    column = gtk_tree_view_column_new_with_attributes(MODEL_TEXT_ROUTEROS, renderer, "active", COL_ROUTEROS, NULL);
    gtk_tree_view_column_set_clickable(column, TRUE);
    g_signal_connect(column, "clicked", (GCallback)ui_view_column_clicked, GINT_TO_POINTER(COL_ROUTEROS));
    gtk_tree_view_column_set_cell_data_func(column, renderer, ui_view_format_background, GINT_TO_POINTER(COL_ROUTEROS), NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    /* Nstreme column */
    renderer = gtk_cell_renderer_toggle_new();
    gtk_cell_renderer_set_padding(renderer, 0, 0);
    column = gtk_tree_view_column_new_with_attributes(MODEL_TEXT_NSTREME, renderer, "active", COL_NSTREME, NULL);
    gtk_tree_view_column_set_clickable(column, TRUE);
    g_signal_connect(column, "clicked", (GCallback)ui_view_column_clicked, GINT_TO_POINTER(COL_NSTREME));
    gtk_tree_view_column_set_cell_data_func(column, renderer, ui_view_format_background, GINT_TO_POINTER(COL_NSTREME), NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    /* TDMA column */
    renderer = gtk_cell_renderer_toggle_new();
    gtk_cell_renderer_set_padding(renderer, 0, 0);
    column = gtk_tree_view_column_new_with_attributes(MODEL_TEXT_TDMA, renderer, "active", COL_TDMA, NULL);
    gtk_tree_view_column_set_clickable(column, TRUE);
    g_signal_connect(column, "clicked", (GCallback)ui_view_column_clicked, GINT_TO_POINTER(COL_TDMA));
    gtk_tree_view_column_set_cell_data_func(column, renderer, ui_view_format_background, GINT_TO_POINTER(COL_TDMA), NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    /* WDS column */
    renderer = gtk_cell_renderer_toggle_new();
    gtk_cell_renderer_set_padding(renderer, 0, 0);
    column = gtk_tree_view_column_new_with_attributes(MODEL_TEXT_WDS, renderer, "active", COL_WDS, NULL);
    gtk_tree_view_column_set_clickable(column, TRUE);
    g_signal_connect(column, "clicked", (GCallback)ui_view_column_clicked, GINT_TO_POINTER(COL_WDS));
    gtk_tree_view_column_set_cell_data_func(column, renderer, ui_view_format_background, GINT_TO_POINTER(COL_WDS), NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    /* Bridge column */
    renderer = gtk_cell_renderer_toggle_new();
    gtk_cell_renderer_set_padding(renderer, 0, 0);
    column = gtk_tree_view_column_new_with_attributes(MODEL_TEXT_BRIDGE, renderer, "active", COL_BRIDGE, NULL);
    gtk_tree_view_column_set_clickable(column, TRUE);
    g_signal_connect(column, "clicked", (GCallback)ui_view_column_clicked, GINT_TO_POINTER(COL_BRIDGE));
    gtk_tree_view_column_set_cell_data_func(column, renderer, ui_view_format_background, GINT_TO_POINTER(COL_BRIDGE), NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    /* Router OS version column */
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_renderer_set_padding(renderer, 1, 0);
    gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer), 1);
    column = gtk_tree_view_column_new_with_attributes(MODEL_TEXT_ROUTEROS_VER, renderer, "text", COL_ROUTEROS_VER, NULL);
    gtk_tree_view_column_set_clickable(column, TRUE);
    g_signal_connect(column, "clicked", (GCallback)ui_view_column_clicked, GINT_TO_POINTER(COL_ROUTEROS_VER));
    gtk_tree_view_column_set_cell_data_func(column, renderer, ui_view_format_background, GINT_TO_POINTER(COL_ROUTEROS_VER), NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    /* First seen column */
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_renderer_set_padding(renderer, 2, 0);
    gtk_cell_renderer_set_alignment(renderer, 1.0, 0.5);
    gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer), 1);
    column = gtk_tree_view_column_new_with_attributes(MODEL_TEXT_FIRSTSEEN, renderer, NULL);
    gtk_tree_view_column_set_clickable(column, TRUE);
    g_signal_connect(column, "clicked", (GCallback)ui_view_column_clicked, GINT_TO_POINTER(COL_FIRSTSEEN));
    gtk_tree_view_column_set_cell_data_func(column, renderer, ui_view_format_date, GINT_TO_POINTER(COL_FIRSTSEEN), NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    /* Last seen column */
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_renderer_set_padding(renderer, 2, 0);
    gtk_cell_renderer_set_alignment(renderer, 1.0, 0.5);
    gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer), 1);
    column = gtk_tree_view_column_new_with_attributes(MODEL_TEXT_LASTSEEN, renderer, NULL);
    gtk_tree_view_column_set_clickable(column, TRUE);
    g_signal_connect(column, "clicked", (GCallback)ui_view_column_clicked, GINT_TO_POINTER(COL_LASTSEEN));
    gtk_tree_view_column_set_cell_data_func(column, renderer, ui_view_format_date, GINT_TO_POINTER(COL_LASTSEEN), NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    /* Latitude column */
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_renderer_set_padding(renderer, 2, 0);
    gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer), 1);
    column = gtk_tree_view_column_new_with_attributes(MODEL_TEXT_LATITUDE, renderer, NULL);
    gtk_tree_view_column_set_clickable(column, TRUE);
    g_signal_connect(column, "clicked", (GCallback)ui_view_column_clicked, GINT_TO_POINTER(COL_LATITUDE));
    gtk_tree_view_column_set_cell_data_func(column, renderer, ui_view_format_gps, GINT_TO_POINTER(COL_LATITUDE), NULL);
    g_object_set_data(G_OBJECT(treeview), "mtscan-column-latitude", column);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    /* Longitude column */
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_renderer_set_padding(renderer, 2, 0);
    gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer), 1);
    column = gtk_tree_view_column_new_with_attributes(MODEL_TEXT_LONGITUDE, renderer, NULL);
    gtk_tree_view_column_set_clickable(column, TRUE);
    g_signal_connect(column, "clicked", (GCallback)ui_view_column_clicked, GINT_TO_POINTER(COL_LONGITUDE));
    gtk_tree_view_column_set_cell_data_func(column, renderer, ui_view_format_gps, GINT_TO_POINTER(COL_LONGITUDE), NULL);
    g_object_set_data(G_OBJECT(treeview), "mtscan-column-longitude", column);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    /* Azimuth column */
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_renderer_set_padding(renderer, 2, 0);
    gtk_cell_renderer_set_alignment(renderer, 1.0, 0.5);
    gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer), 1);
    column = gtk_tree_view_column_new_with_attributes(MODEL_TEXT_AZIMUTH, renderer, NULL);
    gtk_tree_view_column_set_clickable(column, TRUE);
    g_signal_connect(column, "clicked", (GCallback)ui_view_column_clicked, GINT_TO_POINTER(COL_AZIMUTH));
    gtk_tree_view_column_set_cell_data_func(column, renderer, ui_view_format_azimuth, GINT_TO_POINTER(COL_AZIMUTH), NULL);
    g_object_set_data(G_OBJECT(treeview), "mtscan-column-azimuth", column);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    g_signal_connect(treeview, "popup-menu", G_CALLBACK(ui_view_popup), NULL);
    g_signal_connect(treeview, "button-press-event", G_CALLBACK(ui_view_clicked), NULL);
    g_signal_connect(treeview, "key-press-event", G_CALLBACK(ui_view_key_press), NULL);
    g_signal_connect(treeview, "size-allocate", G_CALLBACK(ui_view_size_alloc), NULL);
    g_signal_connect(treeview, "enter-notify-event", G_CALLBACK(ui_view_disable_motion_redraw), NULL);
    g_signal_connect(treeview, "motion-notify-event", G_CALLBACK(ui_view_disable_motion_redraw), NULL);
    g_signal_connect(treeview, "leave-notify-event", G_CALLBACK(ui_view_disable_motion_redraw), NULL);

    ui_view_configure(treeview);
    return treeview;
}

void
ui_view_configure(GtkWidget *view)
{
    gtk_tree_view_set_enable_search(GTK_TREE_VIEW(view), TRUE);

    switch(conf_get_preferences_search_column())
    {
    case 0:
        gtk_tree_view_set_search_column(GTK_TREE_VIEW(view), COL_ADDRESS);
        gtk_tree_view_set_search_equal_func(GTK_TREE_VIEW(view), ui_view_compare_address, NULL, NULL);
        break;
    case 1:
        gtk_tree_view_set_search_column(GTK_TREE_VIEW(view), COL_SSID);
        gtk_tree_view_set_search_equal_func(GTK_TREE_VIEW(view), ui_view_compare_string, NULL, NULL);
        break;
    case 2:
        gtk_tree_view_set_search_column(GTK_TREE_VIEW(view), COL_RADIONAME);
        gtk_tree_view_set_search_equal_func(GTK_TREE_VIEW(view), ui_view_compare_string, NULL, NULL);
        break;
    }

}

void
ui_view_set_icon_size(GtkWidget *view,
                      gint       size)
{
    GtkCellRenderer *renderer = GTK_CELL_RENDERER(g_object_get_data(G_OBJECT(view), "mtscan-icon-renderer"));
    if(renderer)
        gtk_cell_renderer_set_fixed_size(renderer, size, size);

    ui_icon_size(size);
    g_signal_emit_by_name(view, "style-set", NULL, NULL);
}

static gboolean
ui_view_popup(GtkWidget *treeview,
              gpointer   data)
{
    ui_view_menu(treeview, NULL, data);
    return TRUE;
}

static gboolean
ui_view_clicked(GtkWidget      *treeview,
                GdkEventButton *event,
                gpointer        data)
{
    GtkTreeSelection *selection;
    GtkTreePath *path;
    if(event->type == GDK_BUTTON_PRESS && event->button == 3)
    {
        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
        if(gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview),
                                         (gint)event->x,
                                         (gint)event->y,
                                         &path, NULL, NULL, NULL))
        {
            if(!gtk_tree_selection_path_is_selected(selection, path))
            {
                gtk_tree_selection_unselect_all(selection);
                gtk_tree_selection_select_path(selection, path);
            }
            gtk_tree_path_free(path);
            ui_view_menu(treeview, event, data);
            return TRUE;
        }
    }
    return FALSE;
}

static gboolean
ui_view_key_press(GtkWidget   *widget,
                  GdkEventKey *event,
                  gpointer     data)
{
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    GList *list;
    GList *i;
    GtkClipboard *clipboard;
    GString *str;
    gchar *string;

    if(event->keyval == GDK_KEY_Delete)
    {
        ui_view_remove_selection(widget);
    }
    else if(event->keyval == GDK_KEY_c && event->state & GDK_CONTROL_MASK)
    {
        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
        list = gtk_tree_selection_get_selected_rows(selection, &model);

        if(!list)
            return FALSE;

        str = g_string_new("");
        for(i=list; i; i=i->next)
        {
            gtk_tree_model_get_iter(model, &iter, (GtkTreePath*)i->data);
            gtk_tree_model_get(model, &iter, COL_SSID, &string, -1);
            if(strlen(string))
                g_string_append_printf(str, "%s, ", string);
            g_free(string);
        }

        string = g_string_free(str, FALSE);
        /* Remove ending comma */
        string[strlen(string)-2] = '\0';

        clipboard = gtk_widget_get_clipboard(widget, GDK_SELECTION_CLIPBOARD);
        gtk_clipboard_set_text(clipboard, string, -1);
        g_free(string);
        g_list_foreach(list, (GFunc)gtk_tree_path_free, NULL);
        g_list_free(list);
    }

    return FALSE;
}

static void
ui_view_size_alloc(GtkWidget     *treeview,
                   GtkAllocation *allocation,
                   gpointer       data)
{
    GtkWidget *scroll = gtk_widget_get_parent(treeview);
    GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scroll));
    gboolean auto_scroll = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(treeview), "auto-scroll"));
    if(auto_scroll)
    {
        gtk_adjustment_set_value(adj, gtk_adjustment_get_lower(adj));
        g_object_set_data(G_OBJECT(treeview), "auto-scroll", GINT_TO_POINTER(FALSE));
    }
}

static gboolean
ui_view_disable_motion_redraw(GtkWidget *widget,
                              GdkEvent  *event,
                              gpointer   data)
{
    /* Return TRUE and don't process other signal handlers */
    return TRUE;
}

static void
ui_view_format_background(GtkTreeViewColumn *col,
                          GtkCellRenderer   *renderer,
                          GtkTreeModel      *store,
                          GtkTreeIter       *iter,
                          gpointer           data)
{
    static const GdkColor c_sort[2]      = {{ 0, 0xf300, 0xf300, 0xf300 }, { 0, 0x3000, 0x3000, 0x3000 }};
    static const GdkColor c_new[4]       = {{ 0, 0x9700, 0xfa00, 0x9700 }, { 0, 0x2200, 0x4d00, 0x2200 },
                                            { 0, 0x8800, 0xeb00, 0x8800 }, { 0, 0x3100, 0x5e00, 0x3100 }};
    static const GdkColor c_highlight[4] = {{ 0, 0xff00, 0xff00, 0xd400 }, { 0, 0x3300, 0x3300, 0x1900 },
                                            { 0, 0xf000, 0xf000, 0xc500 }, { 0, 0x4700, 0x4700, 0x2b00 }};
    gint col_id, col_sorted, id;
    const GdkColor *ptr = NULL;
    guint8 state;
    gint64 address;

    col_id = GPOINTER_TO_INT(data);
    col_sorted = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(gtk_tree_view_column_get_tree_view(col)), "mtscan-sort"));
    id = conf_get_interface_dark_mode();
    gtk_tree_model_get(store, iter, COL_ADDRESS, &address, COL_STATE, &state, -1);

    if(state == MODEL_STATE_NEW)
    {
        ptr = (col_id == col_sorted ? &c_new[id+2] : &c_new[id]);
    }
    else if(conf_get_preferences_highlightlist_enabled() &&
            conf_get_preferences_highlightlist(address))
    {
        ptr = (col_id == col_sorted ? &c_highlight[id+2] : &c_highlight[id]);
    }
    else if(col_id == col_sorted)
    {
        ptr = &c_sort[id];
    }

    g_object_set(renderer, "cell-background-gdk", ptr, NULL);
}

static void
ui_view_format_icon(GtkTreeViewColumn *col,
                    GtkCellRenderer   *renderer,
                    GtkTreeModel      *store,
                    GtkTreeIter       *iter,
                    gpointer           data)
{
    guint8 state;
    gint8 rssi;
    gboolean privacy;

    gtk_tree_model_get(store, iter,
                       COL_STATE, &state,
                       COL_RSSI, &rssi,
                       COL_PRIVACY, &privacy, -1);

    if(state == MODEL_STATE_INACTIVE)
        rssi = MODEL_NO_SIGNAL;

    g_object_set(renderer, "pixbuf", ui_icon(rssi, privacy), NULL);
    ui_view_format_background(col, renderer, store, iter, data);
}

static void
ui_view_format_address(GtkTreeViewColumn *col,
                       GtkCellRenderer   *renderer,
                       GtkTreeModel      *store,
                       GtkTreeIter       *iter,
                       gpointer           data)
{
    gint col_id = GPOINTER_TO_INT(data);
    gint64 address;
    gtk_tree_model_get(store, iter, col_id, &address, -1);
    g_object_set(renderer, "text", model_format_address(address, FALSE), NULL);
    ui_view_format_background(col, renderer, store, iter, data);
}

static void
ui_view_format_freq(GtkTreeViewColumn *col,
                    GtkCellRenderer   *renderer,
                    GtkTreeModel      *store,
                    GtkTreeIter       *iter,
                    gpointer           data)
{
    gint col_id = GPOINTER_TO_INT(data);
    gint value;
    gtk_tree_model_get(store, iter, col_id, &value, -1);
    g_object_set(renderer, "text", model_format_frequency(value), NULL);
    ui_view_format_background(col, renderer, store, iter, data);
}

static void
ui_view_format_date(GtkTreeViewColumn *col,
                    GtkCellRenderer   *renderer,
                    GtkTreeModel      *store,
                    GtkTreeIter       *iter,
                    gpointer           data)
{
    gint col_id = GPOINTER_TO_INT(data);
    gint64 seen;

    gtk_tree_model_get(store, iter, col_id, &seen, -1);
    g_object_set(renderer, "text", model_format_date(seen), NULL);
    ui_view_format_background(col, renderer, store, iter, data);
}

static void
ui_view_format_level(GtkTreeViewColumn *col,
                     GtkCellRenderer   *renderer,
                     GtkTreeModel      *store,
                     GtkTreeIter       *iter,
                     gpointer           data)
{
    gint col_id = GPOINTER_TO_INT(data);
    guint8 state;
    gint8 value;
    gchar text[10];

    gtk_tree_model_get(store, iter,
                       COL_STATE, &state,
                       col_id, &value,
                       -1);
    if(value == MODEL_NO_SIGNAL ||
       (state == MODEL_STATE_INACTIVE && col_id != COL_MAXRSSI) ||
       (col_id == COL_NOISE && value == 0))
    {
        g_object_set(renderer, "text", "", NULL);
    }
    else
    {
        snprintf(text, sizeof(text), "%d", value);
        g_object_set(renderer, "text", text, NULL);
    }
    ui_view_format_background(col, renderer, store, iter, data);
}

static void
ui_view_format_gps(GtkTreeViewColumn *col,
                   GtkCellRenderer   *renderer,
                   GtkTreeModel      *store,
                   GtkTreeIter       *iter,
                   gpointer           data)
{
    gint col_id = GPOINTER_TO_INT(data);
    gchar text[16];
    gdouble value;
    gtk_tree_model_get(store, iter, col_id, &value, -1);
    if(isnan(value))
    {
        g_object_set(renderer, "text", "", NULL);
    }
    else
    {
        snprintf(text, sizeof(text), "%.5f", value);
        g_object_set(renderer, "text", text, NULL);
    }
    ui_view_format_background(col, renderer, store, iter, data);
}

static void
ui_view_format_azimuth(GtkTreeViewColumn *col,
                       GtkCellRenderer   *renderer,
                       GtkTreeModel      *store,
                       GtkTreeIter       *iter,
                       gpointer           data)
{
    gint col_id = GPOINTER_TO_INT(data);
    gchar text[16];
    gfloat value;
    gtk_tree_model_get(store, iter, col_id, &value, -1);

    if(isnan(value))
    {
        g_object_set(renderer, "text", "", NULL);
    }
    else
    {
        snprintf(text, sizeof(text), "%.1f", value);
        g_object_set(renderer, "text", text, NULL);
    }

    ui_view_format_background(col, renderer, store, iter, data);
}

gboolean
ui_view_compare_address(GtkTreeModel *model,
                        gint          column,
                        const gchar  *string,
                        GtkTreeIter  *iter,
                        gpointer      user_data)
{
    gint64 addr;
    guint i, offset;
    size_t len;
    gchar partial[13];
    const gchar *hex;

    gtk_tree_model_get(model, iter, column, &addr, -1);
    hex = model_format_address(addr, FALSE);
    len = strlen(string);

    for(i=0, offset=0; i<len && offset<12; i++)
    {
        if(string[i] == ':')
            continue;

        if((string[i] >= '0' && string[i] <= '9') ||
           (string[i] >= 'A' && string[i] <= 'F'))
        {
            partial[offset++] = string[i];
        }
        else if(string[i] >= 'a' && string[i] <= 'f')
        {
            partial[offset++] = (gchar)(string[i] - ('a' - 'A'));
        }
    }
    partial[offset] = '\0';
    return strncmp(hex, partial, offset);
}

static gboolean
ui_view_compare_string(GtkTreeModel *model,
                       gint          column,
                       const gchar  *string,
                       GtkTreeIter  *iter,
                       gpointer      user_data)
{

    gchar *value;
    gboolean ret;

    gtk_tree_model_get(model, iter, column, &value, -1);
    ret = g_ascii_strncasecmp(value, string, strlen(string));
    return ret;
}

static void
ui_view_column_clicked(GtkTreeViewColumn *column,
                       gpointer           data)
{
    GtkTreeView *treeview = GTK_TREE_VIEW(gtk_tree_view_column_get_tree_view(column));
    GtkTreeModel *store = gtk_tree_view_get_model(treeview);
    gint to_sort = GPOINTER_TO_INT(data);
    gint current_sort;
    GtkSortType order;
    gboolean sorted, swap_direction;

    sorted = gtk_tree_sortable_get_sort_column_id(GTK_TREE_SORTABLE(store), &current_sort, &order);

    switch(to_sort)
    {
        case COL_MAXRSSI:
        case COL_RSSI:
        case COL_PRIVACY:
        case COL_ROUTEROS:
        case COL_NSTREME:
        case COL_TDMA:
        case COL_WDS:
        case COL_BRIDGE:
            swap_direction = TRUE;
        break;

        default:
            swap_direction = FALSE;
        break;
    }

    if(sorted && current_sort == to_sort)
    {
        if(order == GTK_SORT_ASCENDING)
            order = GTK_SORT_DESCENDING;
        else
            order = GTK_SORT_ASCENDING;
    }
    else
    {
        if(swap_direction)
            order = GTK_SORT_DESCENDING;
        else
            order = GTK_SORT_ASCENDING;
    }

    g_object_set_data(G_OBJECT(treeview), "mtscan-sort", GINT_TO_POINTER(to_sort));
    gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), to_sort, order);
}

void
ui_view_lock(GtkWidget *treeview)
{
    mtscan_model_t *model = g_object_get_data(G_OBJECT(treeview), "mtscan-model");
    gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), NULL);

    /* Disable sorting to speed up things */
    mtscan_model_disable_sorting(model);
}

void
ui_view_unlock(GtkWidget *treeview)
{
    mtscan_model_t *model = g_object_get_data(G_OBJECT(treeview), "mtscan-model");
    mtscan_model_enable_sorting(model);
    gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(model->store));
    ui_view_configure(treeview);
}

void
ui_view_remove_iter(GtkWidget    *treeview,
                    GtkTreeIter  *iter,
                    gboolean      reselect)
{
    GtkTreeModel *store = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
    GtkTreeSelection *selection;
    GtkTreeIter iter_selection = *iter;
    GtkTreeIter iter_tmp;
    gboolean move_selection = (reselect && gtk_tree_model_iter_next(store, &iter_selection));
    mtscan_model_t *model = g_object_get_data(G_OBJECT(treeview), "mtscan-model");

    mtscan_model_remove(model, iter);
    ui_changed();
    ui_status_update_networks();

    if(reselect)
    {
        /* Did we remove last item in the list? */
        if(!move_selection)
        {
            move_selection = gtk_tree_model_get_iter_first(store, &iter_selection);
            if(move_selection)
            {
                iter_tmp = iter_selection;
                while(gtk_tree_model_iter_next(store, &iter_tmp))
                    iter_selection = iter_tmp;
            }
        }

        /* Select next item */
        if(move_selection)
        {
            selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
            gtk_tree_selection_unselect_all(selection);
            gtk_tree_selection_select_iter(selection, &iter_selection);
        }
    }
}

void
ui_view_remove_selection(GtkWidget *treeview)
{
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
    GtkTreeModel *model;
    GtkTreeIter iter;
    GList *list = gtk_tree_selection_get_selected_rows(selection, &model);
    GList *i, *iterlist = NULL;
    gint count = g_list_length(list);
    gchar *string;

    if(!list)
        return;

    for(i=list; i; i=i->next)
    {
        gtk_tree_model_get_iter(model, &iter, (GtkTreePath*)i->data);
        iterlist = g_list_append(iterlist, (gpointer)gtk_tree_iter_copy(&iter));
    }

    if(count == 1)
        string = g_strdup_printf("Are you sure you want to remove the selected network?");
    else
        string = g_strdup_printf("Are you sure you want to remove %d selected networks?", count);

    if(ui_dialog_yesno(GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(treeview))), string) == UI_DIALOG_YES)
    {
        for(i=iterlist; i; i=i->next)
            ui_view_remove_iter(treeview, (GtkTreeIter*)(i->data), FALSE);
    }

    g_list_foreach(iterlist, (GFunc)gtk_tree_iter_free, NULL);
    g_list_free(iterlist);
    g_free(string);
    g_list_foreach(list, (GFunc)gtk_tree_path_free, NULL);
    g_list_free(list);
}

void
ui_view_check_position(GtkWidget *treeview)
{
    GtkWidget *scroll = gtk_widget_get_parent(treeview);
    GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scroll));
    gboolean auto_scroll = gtk_adjustment_get_value(adj) == gtk_adjustment_get_lower(adj);
    g_object_set_data(G_OBJECT(treeview), "auto-scroll", GINT_TO_POINTER(auto_scroll));
}

void
ui_view_dark_mode(GtkWidget *treeview,
                  gboolean   dark_mode)
{
    static const GdkColor bg       = { 0, 0x1500, 0x1500, 0x1500 };
    static const GdkColor text     = { 0, 0xdc00, 0xea00, 0xf200 };
    static const GdkColor selected = { 0, 0x4a00, 0x4a00, 0x4a00 };
    static const GdkColor active   = { 0, 0x3000, 0x3000, 0x3000 };

    if(dark_mode)
    {
        gtk_widget_modify_base(treeview, GTK_STATE_NORMAL, &bg);
        gtk_widget_modify_base(treeview, GTK_STATE_SELECTED, &selected);
        gtk_widget_modify_base(treeview, GTK_STATE_ACTIVE, &active);
        gtk_widget_modify_text(treeview, GTK_STATE_NORMAL, &text);
        gtk_widget_modify_text(treeview, GTK_STATE_SELECTED, &text);
        gtk_widget_modify_text(treeview, GTK_STATE_ACTIVE, &text);
    }
    else
    {
        gtk_widget_set_name(treeview, NULL);

        gtk_widget_modify_base(treeview, GTK_STATE_NORMAL, NULL);
        gtk_widget_modify_base(treeview, GTK_STATE_SELECTED, NULL);
        gtk_widget_modify_base(treeview, GTK_STATE_ACTIVE, NULL);
        gtk_widget_modify_text(treeview, GTK_STATE_NORMAL, NULL);
        gtk_widget_modify_text(treeview, GTK_STATE_SELECTED, NULL);
        gtk_widget_modify_text(treeview, GTK_STATE_ACTIVE, NULL);
    }
}

void
ui_view_noise_column(GtkWidget *treeview,
                      gboolean   visible)
{
    GtkTreeViewColumn *column;
    column = GTK_TREE_VIEW_COLUMN(g_object_get_data(G_OBJECT(treeview), "mtscan-column-noise"));
    gtk_tree_view_column_set_visible(column, visible);
}

void
ui_view_latlon_column(GtkWidget *treeview,
                      gboolean   visible)
{
    GtkTreeViewColumn *column;
    column = GTK_TREE_VIEW_COLUMN(g_object_get_data(G_OBJECT(treeview), "mtscan-column-latitude"));
    gtk_tree_view_column_set_visible(column, visible);
    column = GTK_TREE_VIEW_COLUMN(g_object_get_data(G_OBJECT(treeview), "mtscan-column-longitude"));
    gtk_tree_view_column_set_visible(column, visible);
}

void
ui_view_azimuth_column(GtkWidget *treeview,
                       gboolean   visible)
{
    GtkTreeViewColumn *column;
    column = GTK_TREE_VIEW_COLUMN(g_object_get_data(G_OBJECT(treeview), "mtscan-column-azimuth"));
    gtk_tree_view_column_set_visible(column, visible);
}
