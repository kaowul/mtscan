/*
 *  MTscan - MikroTik RouterOS wireless scanner
 *  Copyright (c) 2015-2018  Konrad Kosmatka
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
#include "conf.h"
#include "ui.h"
#include "ui-view.h"
#include "gps.h"
#include "ui-dialogs.h"
#include "misc.h"

typedef struct ui_preferences_list
{
    GtkWidget *page;
    GtkWidget *box;
    GtkWidget *box_buttons;
    GtkWidget *x_enabled;
    GtkWidget *x_inverted;
    GtkWidget *view;
    GtkWidget *scroll;
    GtkWidget *b_add;
    GtkWidget *b_remove;
    GtkWidget *b_clear;
} ui_preferences_list_t;

typedef struct ui_preferences
{
    GtkWidget *window;
    GtkWidget *content;
    GtkWidget *notebook;

    GtkWidget *page_general;
    GtkWidget *table_general;
    GtkWidget *l_general_icon_size;
    GtkWidget *s_general_icon_size;
    GtkWidget *l_general_icon_size_unit;
    GtkWidget *l_general_autosave_interval;
    GtkWidget *s_general_autosave_interval;
    GtkWidget *l_general_autosave_interval_unit;
    GtkWidget *l_general_autosave_directory;
    GtkWidget *c_general_autosave_directory;
    GtkWidget *l_general_screenshot_directory;
    GtkWidget *c_general_screenshot_directory;
    GtkWidget *l_general_search_column;
    GtkWidget *c_general_search_column;
    GtkWidget *x_general_noise_column;
    GtkWidget *x_general_latlon_column;
    GtkWidget *x_general_azimuth_column;
    GtkWidget *x_general_signals;

    GtkWidget *page_gps;
    GtkWidget *table_gps;
    GtkWidget *l_gps_hostname;
    GtkWidget *e_gps_hostname;
    GtkWidget *l_gps_tcp_port;
    GtkWidget *s_gps_tcp_port;
    GtkWidget *x_gps_show_altitude;
    GtkWidget *x_gps_show_errors;

    ui_preferences_list_t blacklist;
    ui_preferences_list_t highlightlist;

    GtkWidget *box_button;
    GtkWidget *b_apply;
    GtkWidget *b_cancel;
} ui_preferences_t;


static void ui_preferences_list_create(ui_preferences_list_t*, GtkWidget*, const gchar*, const gchar*, const gchar*);
static void ui_preferences_list_format(GtkTreeViewColumn*, GtkCellRenderer*, GtkTreeModel*, GtkTreeIter*, gpointer);

static gboolean ui_preferences_key(GtkWidget*, GdkEventKey*, gpointer);
static gboolean ui_preferences_key_list(GtkWidget*, GdkEventKey*, gpointer);
static gboolean ui_preferences_key_list_add(GtkWidget*, GdkEventKey*, gpointer);
static void ui_preferences_list_add(GtkWidget*, gpointer);
static void ui_preferences_list_remove(GtkWidget*, gpointer);
static void ui_preferences_list_clear(GtkWidget*, gpointer);
static void ui_preferences_load(ui_preferences_t*);
static void ui_preferences_apply(GtkWidget*, gpointer);

void
ui_preferences_dialog(void)
{
    static ui_preferences_t p;
    guint row;

    p.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_modal(GTK_WINDOW(p.window), TRUE);
    gtk_window_set_resizable(GTK_WINDOW(p.window), FALSE);
    gtk_window_set_title(GTK_WINDOW(p.window), "Preferences");
    gtk_window_set_destroy_with_parent(GTK_WINDOW(p.window), TRUE);
    gtk_container_set_border_width(GTK_CONTAINER(p.window), 2);
    gtk_window_set_transient_for(GTK_WINDOW(p.window), GTK_WINDOW(ui.window));
    gtk_window_set_position(GTK_WINDOW(p.window), GTK_WIN_POS_CENTER_ON_PARENT);

    p.content = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(p.window), p.content);

    p.notebook = gtk_notebook_new();
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(p.notebook), GTK_POS_LEFT);
    gtk_notebook_popup_enable(GTK_NOTEBOOK(p.notebook));
    gtk_container_add(GTK_CONTAINER(p.content), p.notebook);

    /* General */
    p.page_general = gtk_vbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(p.page_general), 4);
    gtk_notebook_append_page(GTK_NOTEBOOK(p.notebook), p.page_general, gtk_label_new("General"));
    gtk_container_child_set(GTK_CONTAINER(p.notebook), p.page_general, "tab-expand", FALSE, "tab-fill", FALSE, NULL);

    p.table_general = gtk_table_new(10, 3, TRUE);
    gtk_table_set_homogeneous(GTK_TABLE(p.table_general), FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(p.table_general), 4);
    gtk_table_set_col_spacings(GTK_TABLE(p.table_general), 4);
    gtk_container_add(GTK_CONTAINER(p.page_general), p.table_general);

    row = 0;
    p.l_general_icon_size = gtk_label_new("Icon size:");
    gtk_misc_set_alignment(GTK_MISC(p.l_general_icon_size), 0.0, 0.5);
    gtk_table_attach(GTK_TABLE(p.table_general), p.l_general_icon_size, 0, 1, row, row+1, GTK_EXPAND|GTK_FILL, 0, 0, 0);
    p.s_general_icon_size = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new(0.0, 16.0, 64.0, 1.0, 10.0, 0.0)), 0, 0);
    gtk_table_attach(GTK_TABLE(p.table_general), p.s_general_icon_size, 1, 2, row, row+1, GTK_EXPAND|GTK_FILL, 0, 0, 0);
    p.l_general_icon_size_unit = gtk_label_new("px");
    gtk_table_attach(GTK_TABLE(p.table_general), p.l_general_icon_size_unit, 2, 3, row, row+1, GTK_EXPAND|GTK_FILL, 0, 0, 0);

    row++;
    p.l_general_autosave_interval = gtk_label_new("Autosave interval:");
    gtk_misc_set_alignment(GTK_MISC(p.l_general_autosave_interval), 0.0, 0.5);
    gtk_table_attach(GTK_TABLE(p.table_general), p.l_general_autosave_interval, 0, 1, row, row+1, GTK_EXPAND|GTK_FILL, 0, 0, 0);
    p.s_general_autosave_interval = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new(0.0, 1.0, 60.0, 1.0, 5.0, 0.0)), 0, 0);
    gtk_table_attach(GTK_TABLE(p.table_general), p.s_general_autosave_interval, 1, 2, row, row+1, GTK_EXPAND|GTK_FILL, 0, 0, 0);
    p.l_general_autosave_interval_unit = gtk_label_new("min");
    gtk_table_attach(GTK_TABLE(p.table_general), p.l_general_autosave_interval_unit, 2, 3, row, row+1, GTK_EXPAND|GTK_FILL, 0, 0, 0);

    row++;
    p.l_general_autosave_directory = gtk_label_new("Autosave path:");
    gtk_misc_set_alignment(GTK_MISC(p.l_general_autosave_directory), 0.0, 0.5);
    gtk_table_attach(GTK_TABLE(p.table_general), p.l_general_autosave_directory, 0, 1, row, row+1, GTK_EXPAND|GTK_FILL, 0, 0, 0);
    p.c_general_autosave_directory = gtk_file_chooser_button_new("Autosave", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
    gtk_table_attach(GTK_TABLE(p.table_general), p.c_general_autosave_directory, 1, 3, row, row+1, GTK_EXPAND|GTK_FILL, 0, 0, 0);

    row++;
    p.l_general_screenshot_directory = gtk_label_new("Screenshot path:");
    gtk_misc_set_alignment(GTK_MISC(p.l_general_screenshot_directory), 0.0, 0.5);
    gtk_table_attach(GTK_TABLE(p.table_general), p.l_general_screenshot_directory, 0, 1, row, row+1, GTK_EXPAND|GTK_FILL, 0, 0, 0);
    p.c_general_screenshot_directory = gtk_file_chooser_button_new("Screenshots", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
    gtk_table_attach(GTK_TABLE(p.table_general), p.c_general_screenshot_directory, 1, 3, row, row+1, GTK_EXPAND|GTK_FILL, 0, 0, 0);

    row++;
    p.l_general_search_column = gtk_label_new("Search column:");
    gtk_misc_set_alignment(GTK_MISC(p.l_general_search_column), 0.0, 0.5);
    gtk_table_attach(GTK_TABLE(p.table_general), p.l_general_search_column, 0, 1, row, row+1, GTK_EXPAND|GTK_FILL, 0, 0, 0);
    p.c_general_search_column = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(p.c_general_search_column), "Address");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(p.c_general_search_column), "SSID");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(p.c_general_search_column), "Radio name");
    gtk_table_attach(GTK_TABLE(p.table_general), p.c_general_search_column, 1, 3, row, row+1, GTK_EXPAND|GTK_FILL, 0, 0, 0);

    row++;
    p.x_general_noise_column = gtk_check_button_new_with_label("Show noise floor column");
    gtk_table_attach(GTK_TABLE(p.table_general), p.x_general_noise_column, 0, 3, row, row+1, GTK_EXPAND|GTK_FILL, 0, 0, 0);

    row++;
    p.x_general_latlon_column = gtk_check_button_new_with_label("Show latitude & longitude column");
    gtk_table_attach(GTK_TABLE(p.table_general), p.x_general_latlon_column, 0, 3, row, row+1, GTK_EXPAND|GTK_FILL, 0, 0, 0);

    row++;
    p.x_general_azimuth_column = gtk_check_button_new_with_label("Show azimuth column");
    gtk_table_attach(GTK_TABLE(p.table_general), p.x_general_azimuth_column, 0, 3, row, row+1, GTK_EXPAND|GTK_FILL, 0, 0, 0);

    row++;
    gtk_table_attach(GTK_TABLE(p.table_general), gtk_hseparator_new(), 0, 3, row, row+1, GTK_EXPAND|GTK_FILL, 0, 0, 0);

    row++;
    p.x_general_signals = gtk_check_button_new_with_label("Record all signal samples");
    gtk_table_attach(GTK_TABLE(p.table_general), p.x_general_signals, 0, 3, row, row+1, GTK_EXPAND|GTK_FILL, 0, 0, 0);

    /* GPS */
    p.page_gps = gtk_vbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(p.page_gps), 4);
    gtk_notebook_append_page(GTK_NOTEBOOK(p.notebook), p.page_gps, gtk_label_new("GPSd"));
    gtk_container_child_set(GTK_CONTAINER(p.notebook), p.page_gps, "tab-expand", FALSE, "tab-fill", FALSE, NULL);

    p.table_gps = gtk_table_new(4, 2, TRUE);
    gtk_table_set_homogeneous(GTK_TABLE(p.table_gps), FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(p.table_gps), 4);
    gtk_table_set_col_spacings(GTK_TABLE(p.table_gps), 4);
    gtk_container_add(GTK_CONTAINER(p.page_gps), p.table_gps);

    row = 0;
    p.l_gps_hostname = gtk_label_new("Hostname:");
    gtk_misc_set_alignment(GTK_MISC(p.l_gps_hostname), 0.0, 0.5);
    gtk_table_attach(GTK_TABLE(p.table_gps), p.l_gps_hostname, 0, 1, row, row+1, GTK_EXPAND|GTK_FILL, 0, 0, 0);
    p.e_gps_hostname = gtk_entry_new();
    gtk_table_attach(GTK_TABLE(p.table_gps), p.e_gps_hostname, 1, 2, row, row+1, GTK_EXPAND|GTK_FILL, 0, 0, 0);

    row++;
    p.l_gps_tcp_port = gtk_label_new("TCP Port:");
    gtk_misc_set_alignment(GTK_MISC(p.l_gps_tcp_port), 0.0, 0.5);
    gtk_table_attach(GTK_TABLE(p.table_gps), p.l_gps_tcp_port, 0, 1, row, row+1, GTK_EXPAND|GTK_FILL, 0, 0, 0);
    p.s_gps_tcp_port = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new(2947.0, 1024.0, 65535.0, 1.0, 10.0, 0.0)), 0, 0);
    gtk_table_attach(GTK_TABLE(p.table_gps), p.s_gps_tcp_port, 1, 2, row, row+1, GTK_EXPAND|GTK_FILL, 0, 0, 0);

    row++;
    p.x_gps_show_altitude = gtk_check_button_new_with_label("Show altitude");
    gtk_table_attach(GTK_TABLE(p.table_gps), p.x_gps_show_altitude, 0, 2, row, row+1, GTK_EXPAND|GTK_FILL, 0, 0, 0);

    row++;
    p.x_gps_show_errors = gtk_check_button_new_with_label("Show error estimates");
    gtk_table_attach(GTK_TABLE(p.table_gps), p.x_gps_show_errors, 0, 2, row, row+1, GTK_EXPAND|GTK_FILL, 0, 0, 0);

    ui_preferences_list_create(&p.blacklist, p.notebook, "Blacklist", "Enable blacklist", "Invert to whitelist");
    ui_preferences_list_create(&p.highlightlist, p.notebook, "Highlight", "Enable highlight list", "Invert highlight list");

    /* --- */
    p.box_button = gtk_hbutton_box_new();
    gtk_button_box_set_layout(GTK_BUTTON_BOX(p.box_button), GTK_BUTTONBOX_END);
    gtk_box_set_spacing(GTK_BOX(p.box_button), 5);
    gtk_box_pack_start(GTK_BOX(p.content), p.box_button, FALSE, FALSE, 5);

    p.b_apply = gtk_button_new_from_stock(GTK_STOCK_APPLY);
    g_signal_connect(p.b_apply, "clicked", G_CALLBACK(ui_preferences_apply), &p);
    gtk_container_add(GTK_CONTAINER(p.box_button), p.b_apply);

    p.b_cancel = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
    g_signal_connect_swapped(p.b_cancel, "clicked", G_CALLBACK(gtk_widget_destroy), p.window);
    gtk_container_add(GTK_CONTAINER(p.box_button), p.b_cancel);

    g_signal_connect(p.window, "key-press-event", G_CALLBACK(ui_preferences_key), NULL);
    ui_preferences_load(&p);
    gtk_widget_show_all(p.window);
}

static void
ui_preferences_list_create(ui_preferences_list_t *l,
                           GtkWidget             *notebook,
                           const gchar           *title,
                           const gchar           *enable_title,
                           const gchar           *invert_title)
{
    GtkTreeViewColumn *column;
    GtkCellRenderer *renderer;

    l->page = gtk_vbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(l->page), 4);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), l->page, gtk_label_new(title));
    gtk_container_child_set(GTK_CONTAINER(notebook), l->page, "tab-expand", FALSE, "tab-fill", FALSE, NULL);

    l->box = gtk_vbox_new(FALSE, 4);
    gtk_container_add(GTK_CONTAINER(l->page), l->box);

    l->x_enabled = gtk_check_button_new_with_label(enable_title);
    gtk_box_pack_start(GTK_BOX(l->box), l->x_enabled, FALSE, FALSE, 0);

    l->x_inverted = gtk_check_button_new_with_label(invert_title);
    gtk_box_pack_start(GTK_BOX(l->box), l->x_inverted, FALSE, FALSE, 0);

    l->view = gtk_tree_view_new();
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(renderer), 1);
    column = gtk_tree_view_column_new_with_attributes("Address", renderer, NULL);
    gtk_tree_view_column_set_cell_data_func(column, renderer, ui_preferences_list_format, NULL, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(l->view), column);
    gtk_tree_view_set_enable_search(GTK_TREE_VIEW(l->view), TRUE);
    gtk_tree_view_set_search_column(GTK_TREE_VIEW(l->view), 0);
    gtk_tree_view_set_search_equal_func(GTK_TREE_VIEW(l->view), ui_view_compare_address, NULL, NULL);

    l->scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(l->scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(l->scroll), l->view);
    gtk_widget_set_size_request(l->scroll, -1, 150);
    gtk_box_pack_start(GTK_BOX(l->box), l->scroll, TRUE, TRUE, 0);

    l->box_buttons = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(l->box), l->box_buttons, FALSE, FALSE, 0);

    l->b_add = gtk_button_new_from_stock(GTK_STOCK_ADD);
    g_signal_connect(l->b_add, "clicked", G_CALLBACK(ui_preferences_list_add), l->view);
    gtk_box_pack_start(GTK_BOX(l->box_buttons), l->b_add, TRUE, TRUE, 0);

    l->b_remove = gtk_button_new_from_stock(GTK_STOCK_REMOVE);
    g_signal_connect(l->b_remove, "clicked", G_CALLBACK(ui_preferences_list_remove), l->view);
    gtk_box_pack_start(GTK_BOX(l->box_buttons), l->b_remove, TRUE, TRUE, 0);
    g_signal_connect(l->view, "key-press-event", G_CALLBACK(ui_preferences_key_list), l->b_remove);

    l->b_clear = gtk_button_new_from_stock(GTK_STOCK_CLEAR);
    g_signal_connect(l->b_clear, "clicked", G_CALLBACK(ui_preferences_list_clear), l->view);
    gtk_box_pack_start(GTK_BOX(l->box_buttons), l->b_clear, TRUE, TRUE, 0);
}

static void
ui_preferences_list_format(GtkTreeViewColumn *col,
                           GtkCellRenderer   *renderer,
                           GtkTreeModel      *store,
                           GtkTreeIter       *iter,
                           gpointer           data)
{
    gint64 address;
    gtk_tree_model_get(store, iter, 0, &address, -1);
    g_object_set(renderer, "text", model_format_address(address, FALSE), NULL);
}

static gboolean
ui_preferences_key(GtkWidget   *widget,
                   GdkEventKey *event,
                   gpointer     data)
{
    guint current = gdk_keyval_to_upper(event->keyval);
    if(current == GDK_KEY_Escape)
    {
        gtk_widget_destroy(widget);
        return TRUE;
    }
    return FALSE;
}

static gboolean
ui_preferences_key_list(GtkWidget   *widget,
                        GdkEventKey *event,
                        gpointer     data)
{
    GtkButton *b_remove = GTK_BUTTON(data);
    if(event->keyval == GDK_KEY_Delete)
    {
        gtk_button_clicked(b_remove);
        return TRUE;
    }
    return FALSE;
}

static gboolean
ui_preferences_key_list_add(GtkWidget   *widget,
                            GdkEventKey *event,
                            gpointer     data)
{
    GtkDialog *dialog = GTK_DIALOG(data);

    if(event->keyval == GDK_KEY_Return)
    {
        gtk_dialog_response(dialog, GTK_RESPONSE_OK);
        return TRUE;
    }
    return FALSE;
}

static void
ui_preferences_list_add(GtkWidget *widget,
                        gpointer   data)
{
    GtkTreeView *treeview = GTK_TREE_VIEW(data);
    GtkWidget *toplevel = gtk_widget_get_toplevel(widget);
    GtkWidget *dialog;
    GtkWidget *entry;
    const gchar *value;
    GError *err = NULL;
    GMatchInfo *matchInfo;
    GRegex *regex;
    gchar *match = NULL;
    gint64 addr = -1;

    dialog = gtk_message_dialog_new_with_markup(GTK_WINDOW(toplevel),
                                                GTK_DIALOG_MODAL,
                                                GTK_MESSAGE_QUESTION,
                                                GTK_BUTTONS_OK_CANCEL,
                                                "<big>Enter network address</big>\n"
                                                "eg. 01:23:45:67:89:AB\n"
                                                "or 0123456789AB");

    gtk_window_set_title(GTK_WINDOW(dialog), "New network entry");
    entry = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry), 17);
    gtk_container_add(GTK_CONTAINER(gtk_message_dialog_get_message_area(GTK_MESSAGE_DIALOG(dialog))), entry);
    g_signal_connect(entry, "key-press-event", G_CALLBACK(ui_preferences_key_list_add), dialog);
    gtk_widget_show_all(dialog);

    if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK)
    {
        value = gtk_entry_get_text(GTK_ENTRY(entry));
        regex = g_regex_new("^([0-9A-Fa-f]{2}:){5}([0-9A-Fa-f]{2})", 0, 0, &err);
        if(regex)
        {
            g_regex_match(regex, value, 0, &matchInfo);
            if(g_match_info_matches(matchInfo))
            {
                match = g_match_info_fetch(matchInfo, 0);
                if(match)
                    remove_char(match, ':');
            }
            g_match_info_free(matchInfo);
            g_regex_unref(regex);
        }

        if(!match)
        {
            regex = g_regex_new("^([0-9A-Fa-f]{12})", 0, 0, &err);
            if(regex)
            {
                g_regex_match(regex, value, 0, &matchInfo);
                if(g_match_info_matches(matchInfo))
                    match = g_match_info_fetch(matchInfo, 0);
                g_match_info_free(matchInfo);
                g_regex_unref(regex);
            }
        }

        if(!match ||
           ((addr = str_addr_to_gint64(match, strlen(match))) < 0))
        {
            ui_dialog(GTK_WINDOW(toplevel),
                      GTK_MESSAGE_ERROR,
                      "New network entry",
                      "<big>Invalid address format:</big>\n%s", value);
        }
        g_free(match);
    }

    gtk_widget_destroy(dialog);

    if(addr >= 0)
        gtk_list_store_insert_with_values(GTK_LIST_STORE(gtk_tree_view_get_model(treeview)), NULL, -1, 0, addr, -1);
}

static void
ui_preferences_list_remove(GtkWidget *widget,
                           gpointer   data)
{
    GtkTreeView *treeview = GTK_TREE_VIEW(data);
    GtkTreeIter iter;
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    selection = gtk_tree_view_get_selection(treeview);
    if(gtk_tree_selection_get_selected(selection, &model, &iter))
        gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
}

static void
ui_preferences_list_clear(GtkWidget *widget,
                          gpointer   data)
{
    GtkTreeView *treeview = GTK_TREE_VIEW(data);
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    gtk_list_store_clear(GTK_LIST_STORE(model));
}

static void
ui_preferences_load(ui_preferences_t *p)
{
    GtkListStore *model;

    /* General */
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(p->s_general_icon_size), conf_get_preferences_icon_size());
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(p->s_general_autosave_interval), conf_get_preferences_autosave_interval());
    gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(p->c_general_autosave_directory), conf_get_path_autosave());
    gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(p->c_general_screenshot_directory), conf_get_path_screenshot());
    gtk_combo_box_set_active(GTK_COMBO_BOX(p->c_general_search_column), conf_get_preferences_search_column());
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(p->x_general_noise_column), conf_get_preferences_noise_column());
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(p->x_general_latlon_column), conf_get_preferences_latlon_column());
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(p->x_general_azimuth_column), conf_get_preferences_azimuth_column());
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(p->x_general_signals), conf_get_preferences_signals());

    /* GPS */
    gtk_entry_set_text(GTK_ENTRY(p->e_gps_hostname), conf_get_preferences_gps_hostname());
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(p->s_gps_tcp_port), conf_get_preferences_gps_tcp_port());
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(p->x_gps_show_altitude), conf_get_preferences_gps_show_altitude());
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(p->x_gps_show_errors), conf_get_preferences_gps_show_errors());

    /* Blacklist */
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(p->blacklist.x_enabled), conf_get_preferences_blacklist_enabled());
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(p->blacklist.x_inverted), conf_get_preferences_blacklist_inverted());
    model = conf_get_preferences_blacklist_as_liststore();
    gtk_tree_view_set_model(GTK_TREE_VIEW(p->blacklist.view), GTK_TREE_MODEL(model));
    g_object_unref(model);

    /* Highlight list */
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(p->highlightlist.x_enabled), conf_get_preferences_highlightlist_enabled());
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(p->highlightlist.x_inverted), conf_get_preferences_highlightlist_inverted());
    model = conf_get_preferences_highlightlist_as_liststore();
    gtk_tree_view_set_model(GTK_TREE_VIEW(p->highlightlist.view), GTK_TREE_MODEL(model));
    g_object_unref(model);
}

static void
ui_preferences_apply(GtkWidget *widget,
                     gpointer   data)
{
    ui_preferences_t *p = (ui_preferences_t*)data;
    gint new_icon_size;
    gchar *new_autosave_directory;
    gchar *new_screenshot_directory;
    gint new_search_column;
    gboolean new_noise_column;
    gboolean new_latlon_column;
    gboolean new_azimuth_column;
    const gchar *new_gps_hostname;
    gint new_gps_tcp_port;

    /* General */
    new_icon_size = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(p->s_general_icon_size));
    if(new_icon_size != conf_get_preferences_icon_size())
    {
        conf_set_preferences_icon_size(new_icon_size);
        ui_view_set_icon_size(ui.treeview, new_icon_size);
    }

    conf_set_preferences_autosave_interval(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(p->s_general_autosave_interval)));

    new_autosave_directory = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(p->c_general_autosave_directory));
    conf_set_path_autosave(new_autosave_directory ? new_autosave_directory : "");
    g_free(new_autosave_directory);

    new_screenshot_directory = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(p->c_general_screenshot_directory));
    conf_set_path_screenshot(new_screenshot_directory ? new_screenshot_directory : "");
    g_free(new_screenshot_directory);

    new_search_column = gtk_combo_box_get_active(GTK_COMBO_BOX(p->c_general_search_column));
    if(new_search_column != conf_get_preferences_search_column())
    {
        conf_set_preferences_search_column(new_search_column);
        ui_view_configure(ui.treeview);
    }

    new_noise_column = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p->x_general_noise_column));
    if(new_noise_column != conf_get_preferences_noise_column())
    {
        conf_set_preferences_noise_column(new_noise_column);
        ui_view_noise_column(ui.treeview, new_noise_column);
    }

    new_latlon_column = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p->x_general_latlon_column));
    if(new_latlon_column != conf_get_preferences_latlon_column())
    {
        conf_set_preferences_latlon_column(new_latlon_column);
        ui_view_latlon_column(ui.treeview, new_latlon_column);
    }

    new_azimuth_column = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p->x_general_azimuth_column));
    if(new_azimuth_column != conf_get_preferences_azimuth_column())
    {
        conf_set_preferences_azimuth_column(new_azimuth_column);
        ui_view_azimuth_column(ui.treeview, new_azimuth_column);
    }

    conf_set_preferences_signals(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p->x_general_signals)));

    /* GPS */
    new_gps_hostname = gtk_entry_get_text(GTK_ENTRY(p->e_gps_hostname));
    new_gps_tcp_port = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(p->s_gps_tcp_port));
    if(new_gps_tcp_port !=  conf_get_preferences_gps_tcp_port() ||
       strcmp(new_gps_hostname, conf_get_preferences_gps_hostname()))
    {
        conf_set_preferences_gps_hostname(new_gps_hostname);
        conf_set_preferences_gps_tcp_port(new_gps_tcp_port);
        if(conf_get_interface_gps())
        {
            gps_stop();
            gps_start(new_gps_hostname, new_gps_tcp_port);
        }
    }
    conf_set_preferences_gps_show_altitude(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p->x_gps_show_altitude)));
    conf_set_preferences_gps_show_errors(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p->x_gps_show_errors)));

    /* Blacklist */
    conf_set_preferences_blacklist_enabled(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p->blacklist.x_enabled)));
    conf_set_preferences_blacklist_inverted(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p->blacklist.x_inverted)));
    conf_set_preferences_blacklist_from_liststore(GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(p->blacklist.view))));

    /* Highlight list */
    conf_set_preferences_highlightlist_enabled(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p->highlightlist.x_enabled)));
    conf_set_preferences_highlightlist_inverted(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p->highlightlist.x_inverted)));
    conf_set_preferences_highlightlist_from_liststore(GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(p->highlightlist.view))));

    /* --- */
    gtk_widget_destroy(p->window);
}
