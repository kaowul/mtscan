#ifndef MTSCAN_MODEL_H_
#define MTSCAN_MODEL_H_
#include <gtk/gtk.h>
#include "network.h"

#define MODEL_NO_SIGNAL G_MININT

#define MODEL_DEFAULT_ACTIVE_TIMEOUT 2
#define MODEL_DEFAULT_NEW_TIMEOUT    2

enum
{
    MODEL_STATE_INACTIVE,
    MODEL_STATE_ACTIVE,
    MODEL_STATE_NEW
};

enum
{
    MODEL_UPDATE_NONE,
    MODEL_UPDATE_NEW,
    MODEL_UPDATE,
    MODEL_UPDATE_ONLY_INACTIVE
};

typedef struct mtscan_model
{
    GtkListStore *store;
    GHashTable *map;
    GHashTable *active;
    gint active_timeout;
    gint new_timeout;
    gint disabled_sorting;
    gint last_sort_column;
    GtkSortType last_sort_order;
    GSList *buffer;
    gboolean clear_active_all;
    gboolean clear_active_changed;
} mtscan_model_t;

enum
{
    COL_STATE,
    COL_ICON,
    COL_BG,
    COL_ADDRESS,
    COL_FREQUENCY,
    COL_CHANNEL,
    COL_MODE,
    COL_SSID,
    COL_RADIONAME,
    COL_MAXRSSI,
    COL_RSSI,
    COL_NOISE,
    COL_PRIVACY,
    COL_ROUTEROS,
    COL_NSTREME,
    COL_TDMA,
    COL_WDS,
    COL_BRIDGE,
    COL_ROUTEROS_VER,
    COL_FIRSTSEEN,
    COL_LASTSEEN,
    COL_LATITUDE,
    COL_LONGITUDE,
    COL_SIGNALS,
    COL_COUNT
};

mtscan_model_t* mtscan_model_new();
void mtscan_model_free(mtscan_model_t*);
void mtscan_model_free(mtscan_model_t*);
void mtscan_model_clear(mtscan_model_t*);
void mtscan_model_clear_active(mtscan_model_t*);
void mtscan_model_remove(mtscan_model_t*, GtkTreeIter*);

void mtscan_model_buffer_add(mtscan_model_t*, network_t*);
void mtscan_model_buffer_clear(mtscan_model_t*);
gint mtscan_model_buffer_and_inactive_update(mtscan_model_t*);

void mtscan_model_add(mtscan_model_t*, network_t*, gboolean);

void mtscan_model_set_active_timeout(mtscan_model_t*, gint);
void mtscan_model_set_new_timeout(mtscan_model_t*, gint);

void mtscan_model_disable_sorting(mtscan_model_t*);
void mtscan_model_enable_sorting(mtscan_model_t*);

const gchar* model_format_address(gchar*);
const gchar* model_format_frequency(gint);
const gchar* model_format_date(gint64);
const gchar* model_format_gps(gdouble);

#endif