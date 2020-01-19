/* Unique - Single Instance application library
 * uniquebackend-dbus.c: D-Bus implementation of UniqueBackend
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
#include <errno.h>

#include <dbus/dbus-protocol.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include "../uniqueinternals.h"
#include "uniquefactory-dbus.h"
#include "uniquebackend-dbus.h"
#include "uniquebackend-glue.h"

struct _UniqueBackendDBus
{
  UniqueBackend parent_instance;

  DBusGProxy *proxy;
};

struct _UniqueBackendDBusClass
{
  UniqueBackendClass parent_class;
};

G_DEFINE_TYPE (UniqueBackendDBus, unique_backend_dbus, UNIQUE_TYPE_BACKEND);

static gboolean
unique_backend_dbus_register_proxy (UniqueBackendDBus *backend_dbus)
{
  DBusGConnection *connection;
  GError *error;
  const gchar *name;

  error = NULL;
  connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  if (!connection)
    {
      g_warning ("Unable to open a connection to the session bus: %s",
                 error->message);
      g_error_free (error);

      return FALSE;
    }

  name = unique_backend_get_name (UNIQUE_BACKEND (backend_dbus));
  backend_dbus->proxy = dbus_g_proxy_new_for_name (connection, name,
                                                   "/Factory",
                                                   "org.gtk.UniqueApp");

  /* do not wait for the default 30 seconds timeout */
  dbus_g_proxy_set_default_timeout (backend_dbus->proxy, 3000);

  return (backend_dbus->proxy != NULL);
}

static gboolean
unique_backend_dbus_request_name (UniqueBackend *backend)
{
  const gchar *name;
  DBusGConnection *connection;
  DBusGProxy *proxy;
  GError *error;
  guint32 request_name;
  gboolean res, retval;

  error = NULL;
  connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  if (!connection)
    return FALSE;

  retval = TRUE;
  name = unique_backend_get_name (backend);
  g_assert (name != NULL);

  proxy = dbus_g_proxy_new_for_name (connection,
                                     DBUS_SERVICE_DBUS,
                                     DBUS_PATH_DBUS,
                                     DBUS_INTERFACE_DBUS);

  res = org_freedesktop_DBus_request_name (proxy, name,
                                           0,
                                           &request_name,
                                           &error);
  if (!res)
    retval = FALSE;

  if (request_name != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
    retval = FALSE;

  if (retval)
    {
      UniqueFactoryDBus *factory;

      factory = g_object_new (UNIQUE_TYPE_FACTORY_DBUS, NULL);
      dbus_g_connection_register_g_object (connection, "/Factory",
                                           G_OBJECT (factory));

      factory->backend = backend;
      factory->parent = backend->parent;
    }

  g_object_unref (proxy);

  return retval;
}

static GValueArray *
create_value_array (UniqueMessageData *message_data)
{
  GValueArray *retval;
  GValue item = { 0 };

  retval = g_value_array_new (4);

  /* data */
  g_value_init (&item, G_TYPE_STRING);
  g_value_set_string (&item, (const gchar *) message_data->data);
  g_value_array_append (retval, &item);
  g_value_unset (&item);

  /* length */
  g_value_init (&item, G_TYPE_UINT);
  g_value_set_uint (&item, message_data->length);
  g_value_array_append (retval, &item);
  g_value_unset (&item);

  /* screen */
  g_value_init (&item, G_TYPE_UINT);
  g_value_set_uint (&item, gdk_screen_get_number (message_data->screen));
  g_value_array_append (retval, &item);
  g_value_unset (&item);

  /* workspace */
  g_value_init (&item, G_TYPE_UINT);
  g_value_set_uint (&item, message_data->workspace);
  g_value_array_append (retval, &item);
  g_value_unset (&item);

  /* startup-id */
  g_value_init (&item, G_TYPE_STRING);
  g_value_set_string (&item, message_data->startup_id);
  g_value_array_append (retval, &item);
  g_value_unset (&item);

  return retval;
}

static UniqueResponse
unique_backend_dbus_send_message (UniqueBackend     *backend,
                                  gint               command,
                                  UniqueMessageData *message_data,
                                  guint              time_)
{
  UniqueBackendDBus *backend_dbus;
  GValueArray *data;
  gchar *cmd;
  gchar *resp;
  gboolean res;
  GError *error;
  UniqueResponse response;

  backend_dbus = UNIQUE_BACKEND_DBUS (backend);

  if (!unique_backend_dbus_register_proxy (backend_dbus))
    {
      g_warning ("Unable to connect to the running instance, aborting.");
      return UNIQUE_RESPONSE_INVALID;
    }

  cmd = g_strdup (unique_command_to_string (backend->parent, command));
  data = create_value_array (message_data);
  resp = NULL;
 
  error = NULL;
  res = org_gtk_UniqueApp_send_message (backend_dbus->proxy,
                                        cmd, data, time_,
                                        &resp,
                                        &error);
  if (!res)
    {
      if (error)
        {
          g_warning ("Error while sending message: %s", error->message);
          g_error_free (error);
        }

      g_free (cmd);
      
      return UNIQUE_RESPONSE_INVALID;
    }

  response = unique_response_from_string (resp);

  g_free (cmd);
  g_free (resp);

  return response;
}

static void
unique_backend_dbus_dispose (GObject *gobject)
{
  UniqueBackendDBus *backend_dbus = UNIQUE_BACKEND_DBUS (gobject);

  if (backend_dbus->proxy)
    {
      g_object_unref (backend_dbus->proxy);
      backend_dbus->proxy = NULL;
    }

  G_OBJECT_CLASS (unique_backend_dbus_parent_class)->dispose (gobject);
}

static void
unique_backend_dbus_class_init (UniqueBackendDBusClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  UniqueBackendClass *backend_class = UNIQUE_BACKEND_CLASS (klass);

  gobject_class->dispose = unique_backend_dbus_dispose;

  backend_class->request_name = unique_backend_dbus_request_name;
  backend_class->send_message = unique_backend_dbus_send_message;
}

static void
unique_backend_dbus_init (UniqueBackendDBus *backend)
{

}
