#include <gtk/gtk.h>
#include <string.h>
#include <inttypes.h>

#define MAC_ADDR_HEX_LEN 12

static gboolean create_liststore_from_tree_foreach(gpointer, gpointer, gpointer);
static gboolean fill_tree_from_liststore_foreach(GtkTreeModel*, GtkTreePath*, GtkTreeIter*, gpointer);

gint
gptrcmp(gconstpointer a,
        gconstpointer b)
{
    return GPOINTER_TO_INT(a) - GPOINTER_TO_INT(b);
}

gint
gint64cmp(const gint64 *a,
          const gint64 *b)
{
    if(*a < *b)
        return -1;
    if(*a > *b)
        return 1;
    return 0;
}

gint64*
gint64dup(const gint64* src)
{
    gint64 *dst;
    dst = g_malloc(sizeof(gint64));
    *dst = *src;
    return dst;
}

void
remove_char(gchar *str,
            gchar  c)
{
    gchar *pr = str;
    gchar *pw = str;
    while(*pr)
    {
        *pw = *pr++;
        pw += (*pw != c);
    }
    *pw = 0;
}

gchar*
str_scanlist_compress(const gchar *ptr)
{
    GString *output;
    gint prev, curr, last_written;

    if(!ptr)
        return NULL;

    output = g_string_new(NULL);

    prev = 0;
    last_written = 0;
    while(ptr)
    {
        curr = 0;
        sscanf(ptr, "%d", &curr);
        if(curr)
        {
            if(!prev)
            {
                g_string_append_printf(output, "%d", curr);
                prev = curr;
                last_written = curr;
            }
            else
            {
                if(prev != curr-5)
                {
                    if(last_written != prev)
                        g_string_append_printf(output, "-%d", prev);
                    g_string_append_printf(output, ",%d", curr);
                    last_written = curr;
                }
                prev = curr;
            }
        }

        ptr = strchr(ptr, ',');
        ptr = (ptr ? ptr + 1 : NULL);
    }

    if(last_written != prev)
        g_string_append_printf(output, "-%d", prev);

    return g_string_free(output, FALSE);
}

GtkListStore*
create_liststore_from_tree(GTree *tree)
{
    GtkListStore *model = gtk_list_store_new(1, G_TYPE_INT64);
    g_tree_foreach(tree, create_liststore_from_tree_foreach, model);
    return model;
}

static gboolean
create_liststore_from_tree_foreach(gpointer key,
                                   gpointer value,
                                   gpointer data)
{
    gint64 *v = (gint64*)key;
    GtkListStore *model = (GtkListStore*)data;
    gtk_list_store_insert_with_values(model, NULL, -1, 0, *v, -1);
    return FALSE;
}

void
fill_tree_from_liststore(GTree        *tree,
                         GtkListStore *model)
{
    g_tree_ref(tree);
    g_tree_destroy(tree);
    gtk_tree_model_foreach(GTK_TREE_MODEL(model), fill_tree_from_liststore_foreach, tree);
}

static gboolean
fill_tree_from_liststore_foreach(GtkTreeModel *model,
                                 GtkTreePath  *path,
                                 GtkTreeIter  *iter,
                                 gpointer      data)
{
    GTree *tree = (GTree*)data;
    gint64 value;

    gtk_tree_model_get(model, iter, 0, &value, -1);
    g_tree_insert(tree, gint64dup(&value), GINT_TO_POINTER(TRUE));
    return FALSE;
}

gint64
str_addr_to_gint64(const gchar* str,
                   gint         len)
{
    gchar buffer[MAC_ADDR_HEX_LEN+1];
    gchar *ptr;
    gint64 value;
    gint i;

    if(len == MAC_ADDR_HEX_LEN)
    {
        for(i=0; i<len; i++)
        {
            if(!((str[i] >= '0' && str[i] <= '9') ||
                (str[i] >= 'A' && str[i] <= 'F') ||
                (str[i] >= 'a' && str[i] <= 'f')))
                return -1;
        }

        memcpy(buffer, str, MAC_ADDR_HEX_LEN);
        buffer[MAC_ADDR_HEX_LEN] = '\0';
        value = g_ascii_strtoll(buffer, &ptr, 16);
        if(ptr != buffer)
            return value;
    }
    return -1;
}
