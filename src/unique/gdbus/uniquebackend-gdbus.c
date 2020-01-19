/* Unique - Single Instance application library
 * uniquebackend-gdbus.c: GDBus implementation of UniqueBackend
 *
 * Copyright (C) 2007  Emmanuele Bassi  <ebassi@o-hand.com>
 * Copyright © 2010 Christian Persch
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

#include <gio/gio.h>
#include <gdk/gdk.h>

#include "../uniqueinternals.h"
#include "uniquebackend-gdbus.h"

struct _UniqueBackendGDBus
{
  UniqueBackend parent_instance;

  GDBusConnection *connection;
  guint registration_id;
  guint owner_id;
  gboolean owns_name;
  GMainLoop *loop;
};

struct _UniqueBackendGDBusClass
{
  UniqueBackendClass parent_class;
  GDBusNodeInfo *introspection_data;
};

G_DEFINE_TYPE (UniqueBackendGDBus, unique_backend_gdbus, UNIQUE_TYPE_BACKEND);

static const char introspection_xml[] =
  "<node name='/'>"
    "<interface name='org.gtk.UniqueApp'>"
      "<method name='SendMessage'>"
        "<arg name='command' type='s' direction='in'/>"
        "<arg name='message' type='(suuus)' direction='in'/>"
        "<arg name='time' type='u' direction='in'/>"
        "<arg name='response' type='s' direction='out'/>"
      "</method>"
    "</interface>"
  "</node>";

static void
method_call_cb (GDBusConnection       *connection,
                const gchar           *sender,
                const gchar           *object_path,
                const gchar           *interface_name,
                const gchar           *method_name,
                GVariant              *parameters,
                GDBusMethodInvocation *invocation,
                gpointer               user_data)
{
  if (g_strcmp0 (interface_name, "org.gtk.UniqueApp") != 0 ||
      g_strcmp0 (object_path, "/Factory") != 0)
    return;

  if (g_strcmp0 (method_name, "SendMessage") == 0)
    {
      UniqueBackend *backend = UNIQUE_BACKEND (user_data);
      const gchar *command_str, *data, *startup_id;
      guint len, screen_num, workspace, time_;
      UniqueMessageData message_data;
      gint command;
      UniqueResponse response;
      GdkDisplay *display;

      g_variant_get (parameters,
                     "(&s(&suuu&s)u)",
                     &command_str,
                     &data, &len, &screen_num, &workspace, &startup_id,
                     &time_);

      command = unique_command_from_string (backend->parent, command_str);
      if (command == 0)
        {
          g_dbus_method_invocation_return_error (invocation,
                                                 G_DBUS_ERROR,
                                                 G_DBUS_ERROR_INVALID_ARGS,
                                                 "Invalid command `%s' received",
                                                 command_str);
          return;
        }

      display = gdk_display_get_default ();

      message_data.data = len > 0 ? (guchar *) data : NULL;
      message_data.length = (gint) len;
      message_data.workspace = workspace;
      message_data.startup_id = (char *) startup_id;
      if (screen_num >= 0 && screen_num < gdk_display_get_n_screens (display))
        message_data.screen = gdk_display_get_screen (display, screen_num);
      else
        message_data.screen = gdk_screen_get_default ();

      response = unique_app_emit_message_received (backend->parent, command, &message_data, time_);

      g_dbus_method_invocation_return_value (invocation,
                                             g_variant_new ("(s)", unique_response_to_string (response)));
      return;
    }
}

static void
name_acquired_cb (GDBusConnection *connection,
                  const gchar     *name,
                  gpointer         user_data)
{
  UniqueBackendGDBus *backend_gdbus = UNIQUE_BACKEND_GDBUS (user_data);

  backend_gdbus->owns_name = TRUE;
  if (backend_gdbus->loop && g_main_loop_is_running (backend_gdbus->loop))
    g_main_loop_quit (backend_gdbus->loop);
}

static void
name_lost_cb (GDBusConnection *connection,
              const gchar     *name,
              gpointer         user_data)
{
  UniqueBackendGDBus *backend_gdbus = UNIQUE_BACKEND_GDBUS (user_data);

  backend_gdbus->owns_name = FALSE;
  if (backend_gdbus->loop && g_main_loop_is_running (backend_gdbus->loop))
    g_main_loop_quit (backend_gdbus->loop);
}

static const GDBusInterfaceVTable interface_vtable = {
  method_call_cb,
  NULL,
  NULL
};

static gboolean
unique_backend_gdbus_request_name (UniqueBackend *backend)
{
  UniqueBackendGDBus *backend_gdbus = UNIQUE_BACKEND_GDBUS (backend);
  UniqueBackendGDBusClass *klass = UNIQUE_BACKEND_GDBUS_GET_CLASS (backend);
  GError *error;

  error = NULL;
  backend_gdbus->connection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &error);
  if (!backend_gdbus->connection)
    {
      g_warning ("Unable to open a connection to the session bus: %s",
                 error->message);
      g_error_free (error);

      return FALSE;
    }

  backend_gdbus->registration_id =
      g_dbus_connection_register_object (backend_gdbus->connection,
                                         "/Factory",
                                         klass->introspection_data->interfaces[0],
                                         &interface_vtable,
                                         backend, NULL,
                                         &error);
  if (backend_gdbus->registration_id == 0)
    {
      g_warning ("Unable to register object with the session bus: %s",
                 error->message);
      g_error_free (error);

      return FALSE;
    }

  backend_gdbus->owns_name = FALSE;

  backend_gdbus->owner_id =
      g_bus_own_name_on_connection (backend_gdbus->connection,
                                    unique_backend_get_name (backend),
                                    G_BUS_NAME_OWNER_FLAGS_NONE,
                                    name_acquired_cb,
                                    name_lost_cb,
                                    backend, NULL);

  backend_gdbus->loop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run (backend_gdbus->loop);
  g_main_loop_unref (backend_gdbus->loop);
  backend_gdbus->loop = NULL;

  return backend_gdbus->owns_name;
}

static UniqueResponse
unique_backend_gdbus_send_message (UniqueBackend     *backend,
                                   gint               command,
                                   UniqueMessageData *message_data,
                                   guint              time_)
{
  UniqueBackendGDBus *backend_gdbus = UNIQUE_BACKEND_GDBUS (backend);
  GVariantBuilder builder;
  GVariant *result;
  const gchar *command_str, *resp;
  UniqueResponse response;
  GError *error;

  command_str = unique_command_to_string (backend->parent, command);

  g_variant_builder_init (&builder, G_VARIANT_TYPE ("(s(suuus)u)"));
  g_variant_builder_add (&builder, "s", command_str ? command_str : "");
  g_variant_builder_open (&builder, G_VARIANT_TYPE ("(suuus)"));
  g_variant_builder_add (&builder, "s", message_data->data ? (char *) message_data->data : "");
  g_variant_builder_add (&builder, "u", (guint) message_data->length);
  g_variant_builder_add (&builder, "u", (guint) gdk_screen_get_number (message_data->screen));
  g_variant_builder_add (&builder, "u", (guint) message_data->workspace);
  g_variant_builder_add (&builder, "s", message_data->startup_id ? message_data->startup_id : "");
  g_variant_builder_close (&builder);
  g_variant_builder_add (&builder, "u", time_);

  error = NULL;
  result = g_dbus_connection_call_sync (backend_gdbus->connection,
                                        unique_backend_get_name (backend),
                                        "/Factory",
                                        "org.gtk.UniqueApp",
                                        "SendMessage",
                                        g_variant_builder_end (&builder),
                                        G_VARIANT_TYPE ("(s)"),
                                        G_DBUS_CALL_FLAGS_NO_AUTO_START,
                                        -1,
                                        NULL,
                                        &error);
  if (error)
    {
      g_warning ("Error while sending message: %s", error->message);
      g_error_free (error);
      
      return UNIQUE_RESPONSE_INVALID;
    }

  g_variant_get (result, "(&s)", &resp);
  response = unique_response_from_string (resp);
  g_variant_unref (result);

  return response;
}

static void
unique_backend_gdbus_dispose (GObject *gobject)
{
  UniqueBackendGDBus *backend_gdbus = UNIQUE_BACKEND_GDBUS (gobject);

  if (backend_gdbus->owner_id != 0)
    {
      g_bus_unown_name (backend_gdbus->owner_id);
      backend_gdbus->owner_id = 0;
    }
  if (backend_gdbus->registration_id != 0)
    {
      g_assert (backend_gdbus->connection != NULL);
      g_dbus_connection_unregister_object (backend_gdbus->connection,
                                           backend_gdbus->registration_id);
      backend_gdbus->registration_id = 0;
    }
  if (backend_gdbus->connection)
    {
      g_object_unref (backend_gdbus->connection);
      backend_gdbus->connection = NULL;
    }

  G_OBJECT_CLASS (unique_backend_gdbus_parent_class)->dispose (gobject);
}

static void
unique_backend_gdbus_class_init (UniqueBackendGDBusClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  UniqueBackendClass *backend_class = UNIQUE_BACKEND_CLASS (klass);

  gobject_class->dispose = unique_backend_gdbus_dispose;

  backend_class->request_name = unique_backend_gdbus_request_name;
  backend_class->send_message = unique_backend_gdbus_send_message;

  klass->introspection_data = g_dbus_node_info_new_for_xml (introspection_xml, NULL);
  g_assert (klass->introspection_data != NULL);
}

static void
unique_backend_gdbus_init (UniqueBackendGDBus *backend)
{
}
