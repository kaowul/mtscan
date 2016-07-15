#ifndef MTSCAN_SIGNALS_H_
#define MTSCAN_SIGNALS_H_
#include <glib.h>

typedef struct signals_node
{
    struct signals_node *next;
    gint64 timestamp;
    gdouble latitude;
    gdouble longitude;
    gint8 rssi;
} signals_node_t;

typedef struct signals
{
    signals_node_t *head;
    signals_node_t *tail;
} signals_t;

signals_t* signals_new();
signals_node_t* signals_node_new0();
signals_node_t* signals_node_new(gint64, gint8, gdouble, gdouble);
void signals_append(signals_t*, signals_node_t*);
gboolean signals_merge_and_free(signals_t**, signals_t*);
void signals_free(signals_t*);

#endif