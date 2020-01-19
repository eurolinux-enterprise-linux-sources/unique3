/* Unique - Single Instance Application library
 * uniquefactory-dbus.h: D-Bus factory
 *
 * Copyright (C) 2007  Emmanuele Bassi  <ebassi@o-hand.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <errno.h>

#include <dbus/dbus-protocol.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include "../uniqueinternals.h"
#include "uniquefactory-dbus.h"

G_DEFINE_TYPE (UniqueFactoryDBus, unique_factory_dbus, G_TYPE_OBJECT);

#define uniquebackend_send_message       unique_factory_dbus_send_message

static gboolean
unique_factory_dbus_send_message (UniqueFactoryDBus  *factory,
                                  const gchar        *command_IN,
                                  GValueArray        *message_IN,
                                  guint               time_IN,
                                  gchar             **response_OUT,
                                  GError            **error)
{
  UniqueMessageData *data;
  gint command;
  UniqueResponse response;
  GdkDisplay *display;
  guint screen_n;

  command = unique_command_from_string (factory->parent, command_IN);
  if (command == 0)
    {
      g_warning ("Invalid command `%s' received", command_IN);
      return TRUE;
    }

  display = gdk_display_get_default ();

  data = g_slice_new (UniqueMessageData);
  data->data = (guchar *) g_value_dup_string (g_value_array_get_nth (message_IN, 0));
  data->length = g_value_get_uint (g_value_array_get_nth (message_IN, 1));
  screen_n = g_value_get_uint (g_value_array_get_nth (message_IN, 2));
  data->screen = gdk_display_get_screen (display, screen_n);
  data->workspace = g_value_get_uint (g_value_array_get_nth (message_IN, 3));
  data->startup_id = g_value_dup_string (g_value_array_get_nth (message_IN, 4));

  response = unique_app_emit_message_received (factory->parent, command, data, time_IN);
  unique_message_data_free (data);

  *response_OUT = g_strdup (unique_response_to_string (response));

  return TRUE;
}

#include "uniquebackend-glue.h"
#include "uniquebackend-bindings.h"

static void
unique_factory_dbus_class_init (UniqueFactoryDBusClass *klass)
{
  dbus_g_object_type_install_info (G_TYPE_FROM_CLASS (klass),
                                   &dbus_glib_uniquebackend_object_info);
}

static void
unique_factory_dbus_init (UniqueFactoryDBus *factory)
{

}
