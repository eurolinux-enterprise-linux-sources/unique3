/* Unique - Single Instance Backendlication library
 * uniquefactory-bacon.c: Unix domain socket implementation of UniqueBackend
 *
 * Copyright (C) 2007  Emmanuele Bassi  <ebassi@gnome.com>
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
 *
 * Based on libbacon implementation in Totem
 *      Copyright (C) 2003  Bastien Nocera
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
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <glib/gstdio.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <errno.h>

#include "../uniqueinternals.h"
#include "uniquefactory-bacon.h"
#include "uniquemessage-bacon.h"

G_DEFINE_TYPE (UniqueFactoryBacon, unique_factory_bacon, G_TYPE_OBJECT);

static void
unique_factory_bacon_dispose (GObject *gobject)
{
  UniqueFactoryBacon *factory = UNIQUE_FACTORY_BACON (gobject);

  if (factory->source_id)
    {
      g_source_remove (factory->source_id);
      factory->source_id = 0;
    }

  if (factory->channel)
    {
      g_io_channel_shutdown (factory->channel, TRUE, NULL);
      g_io_channel_unref (factory->channel);
      factory->channel = NULL;
    }

  if (factory->socket_fd != -1)
    {
      close (factory->socket_fd);
      factory->socket_fd = -1;
    }

  G_OBJECT_CLASS (unique_factory_bacon_parent_class)->dispose (gobject);
}

static void
unique_factory_bacon_class_init (UniqueFactoryBaconClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->dispose = unique_factory_bacon_dispose;
}

static void
unique_factory_bacon_init (UniqueFactoryBacon *factory)
{
  factory->parent = NULL;

  factory->socket_fd = -1;
  
  factory->channel = NULL;
  factory->source_id = 0;
}

static gboolean
connection_cb (GIOChannel   *channel,
               GIOCondition  condition,
               gpointer      data)
{
  UniqueFactoryBacon *factory = data;
  GError *read_error, *write_error;
  GIOStatus res;
  gint command_id;
  guint time_;
  UniqueMessageData *message_data;
  UniqueResponse response_id;
  gchar *response;
  gchar *message;
  gsize len, term;

  if (!factory->channel)
    {
      g_warning ("No channel available");
      return FALSE;
    }

  if (condition & G_IO_ERR)
    {
      g_warning ("Connection to the sender failed");
      goto finished;
    }

  read_error = NULL;
  res = g_io_channel_read_line (factory->channel,
                                &message, &len,
                                &term,
                                &read_error);
  if (res == G_IO_STATUS_ERROR)
    {
      g_warning ("Unable to receive the command: %s", read_error->message);
      g_error_free (read_error);
      goto finished;
    }

  if (len == 0)
    goto finished;
  
  /* truncate the message at the line terminator */
  message[term] = '\0';
  message_data = unique_message_data_unpack (factory->parent,
                                             message,
                                             &command_id,
                                             &time_);
  if (!message_data)
    {
      g_warning ("Unable to unpack the message");
      g_free (message);
      goto finished;
    }

  response_id = unique_app_emit_message_received (factory->parent,
                                                  command_id, message_data,
                                                  time_);
  
  response = g_strconcat (unique_response_to_string (response_id), "\r\n", NULL);
  
  write_error = NULL;
  res = g_io_channel_write_chars (factory->channel,
                                  response, -1,
                                  NULL,
                                  &write_error);
  if (res == G_IO_STATUS_ERROR)
    {
      g_warning ("Unable to send response: %s", write_error->message);
      g_error_free (write_error);
    }
  else
    g_io_channel_flush (factory->channel, NULL);

  g_free (response);
  g_free (message);

finished:
  factory->source_id = 0;

  return FALSE;
}

static void
cleanup_connection (gpointer data)
{

}

gboolean
unique_factory_bacon_accept (UniqueFactoryBacon *factory,
                             gint                socket_fd)
{
  unsigned int len;

  g_return_val_if_fail (UNIQUE_IS_FACTORY_BACON (factory), FALSE);
  g_return_val_if_fail (socket_fd != -1, FALSE);

  factory->socket_fd = accept (socket_fd, NULL, &len);

  factory->channel = g_io_channel_unix_new (factory->socket_fd);
  g_io_channel_set_line_term (factory->channel, "\r\n", 2);

  factory->source_id = g_io_add_watch_full (factory->channel,
                                            G_PRIORITY_DEFAULT,
                                            G_IO_IN | G_IO_ERR,
                                            connection_cb,
                                            factory,
                                            cleanup_connection);
  
  return TRUE;
}

gint
unique_factory_bacon_get_fd (UniqueFactoryBacon *factory)
{
  g_return_val_if_fail (UNIQUE_IS_FACTORY_BACON (factory), -1);

  return factory->socket_fd;
}
