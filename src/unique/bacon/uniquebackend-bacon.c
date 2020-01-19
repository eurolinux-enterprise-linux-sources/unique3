/* Unique - Single Instance Backendlication library
 * uniquebackend-bacon.c: Unix domain socket implementation of UniqueBackend
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <glib/gstdio.h>

#include "../uniqueinternals.h"
#include "uniquefactory-bacon.h"
#include "uniquemessage-bacon.h"
#include "uniquebackend-bacon.h"

#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX 108
#endif

struct _UniqueBackendBacon
{
  UniqueBackend parent_instance;

  gchar *socket_path;
  gint socket_fd;
  
  GIOChannel *channel;
  guint source_id;

  GSList *connections;

  guint is_server : 1;
};

struct _UniqueBackendBaconClass
{
  UniqueBackendClass parent_class;
};

G_DEFINE_TYPE (UniqueBackendBacon, unique_backend_bacon, UNIQUE_TYPE_BACKEND);


static gboolean
is_socket (const gchar *path)
{
  struct stat stat_buf;

  if (g_stat (path, &stat_buf) == -1)
    return FALSE;

  return (S_ISSOCK (stat_buf.st_mode));
}

static gboolean
is_socket_owned_by_user (const gchar *path)
{
  struct stat stat_buf;

  if (g_stat (path, &stat_buf) == -1)
    return FALSE;

  return (S_ISSOCK (stat_buf.st_mode) && stat_buf.st_uid == geteuid ());
}

/* free the return value */
static gchar *
find_file_with_pattern (const gchar *path,
                        const gchar *pattern)
{
  GDir *dir;
  gchar *retval;
  const gchar *file;
  GPatternSpec *pat;

  dir = g_dir_open (path, 0, NULL);
  if (!dir)
    return NULL;

  pat = g_pattern_spec_new (pattern);
  if (!pat)
    {
      g_dir_close (dir);
      return NULL;
    }

  retval = NULL;
  while ((file = g_dir_read_name (dir)) != NULL)
    {
      if (g_pattern_match_string (pat, file))
        {
          gchar *temp = g_build_filename (path, file, NULL);

          if (is_socket_owned_by_user (temp))
            {
              retval = temp;
              break;
            }
          
          g_free (temp);
        }
    }

  g_pattern_spec_free (pat);
  g_dir_close (dir);

  return retval;
}

/* free the returned value */
static gchar *
find_socket_file (const gchar *name)
{
  const gchar *token;
  gchar *basename, *path;
  gchar *tmpdir;
  
  /* socket file name template:
   *   /tmp/unique/org.gnome.YourApplication.token.process-id
   */
  token = g_getenv ("DISPLAY");
  if (!token || *token == '\0')
    {
      g_warning ("The $DISPLAY environment variable is not set. You must "
                 "set it in order for the application '%s' to run correctly.",
                 g_get_prgname ());
      return NULL;
    }

  basename = g_strconcat (name, ".", token, ".*", NULL);
  tmpdir = g_build_path (G_DIR_SEPARATOR_S,
                         g_get_tmp_dir (),
                         "unique",
                         NULL);

  if (g_mkdir_with_parents (tmpdir, 0777) == -1)
    {
      if (errno != EEXIST)
        {
          g_warning ("Unable to create socket path `%s': %s",
                     tmpdir,
                     g_strerror (errno));
          return NULL;
        }
    }

  path = find_file_with_pattern (tmpdir, basename);
  if (path)
    {
      g_free (tmpdir);
      g_free (basename);

      return path;
    }

  g_free (basename);

  basename = g_strdup_printf ("%s.%s.%d", name, token, getpid ());
  
  path = g_build_filename (tmpdir, basename, NULL);

  g_free (tmpdir);
  g_free (basename);

  return path;
}

static gboolean
server_socket_cb (GIOChannel   *source,
                  GIOCondition  condition,
                  gpointer      data)
{
  UniqueBackendBacon *backend_bacon = data;

  if (!backend_bacon)
    return FALSE;

  if (!backend_bacon->channel)
    return FALSE;

  if (condition & G_IO_IN)
    {
      UniqueFactoryBacon *factory;

      factory = g_object_new (UNIQUE_TYPE_FACTORY_BACON, NULL);
      factory->parent = UNIQUE_BACKEND (backend_bacon)->parent;

      if (unique_factory_bacon_accept (factory, backend_bacon->socket_fd))
        backend_bacon->connections = g_slist_prepend (backend_bacon->connections, factory);
      else
        {
          g_warning ("Could not accept the connection");
          g_object_unref (factory);
        }
    }
  
  if (condition & G_IO_HUP)
    g_debug (G_STRLOC ": factory hung up");
  else if (condition & G_IO_ERR)
    g_warning ("Server error");

  return TRUE;
}

static gboolean
setup_connection (UniqueBackendBacon *backend_bacon)
{
  g_assert (backend_bacon->socket_fd != -1);
  g_assert (backend_bacon->channel == NULL);

  backend_bacon->channel = g_io_channel_unix_new (backend_bacon->socket_fd);
  g_io_channel_set_line_term (backend_bacon->channel, "\r\n", 2);
      
  backend_bacon->source_id = g_io_add_watch (backend_bacon->channel,
                                             G_IO_IN | G_IO_ERR | G_IO_HUP,
                                             server_socket_cb,
                                             backend_bacon);
  return (backend_bacon->source_id > 0);
}

static gboolean
try_client (UniqueBackendBacon *backend)
{
  struct sockaddr_un uaddr;
  size_t path_len;
  int res;

  g_assert (backend->socket_path != NULL);

  path_len = MIN (strlen (backend->socket_path) + 1, UNIX_PATH_MAX);

  uaddr.sun_family = AF_UNIX;
  strncpy (uaddr.sun_path, backend->socket_path, path_len);
  
  backend->socket_fd = socket (PF_UNIX, SOCK_STREAM, 0);
  
  res = connect (backend->socket_fd,
                 (struct sockaddr *) &uaddr,
                 sizeof (uaddr));

  if (res == -1)
    {
      backend->socket_fd = -1;
      return FALSE;
    }

  return TRUE;
}

#define MAX_CONNECTIONS 5

static void
create_server (UniqueBackendBacon *backend)
{
  struct sockaddr_un uaddr;
  size_t path_len;
  int res;

  g_assert (backend->socket_path != NULL);

  path_len = MIN (strlen (backend->socket_path) + 1, UNIX_PATH_MAX);

  uaddr.sun_family = AF_UNIX;
  strncpy (uaddr.sun_path, backend->socket_path, path_len);

  backend->socket_fd = socket (PF_UNIX, SOCK_STREAM, 0);
  res = bind (backend->socket_fd, (struct sockaddr *) &uaddr, sizeof (uaddr));
  if (res == -1)
    {
      backend->socket_fd = -1;
      return;
    }
  
  /* For security concern, the socket files should be set 700 */
  g_chmod (backend->socket_path, 0700);

  listen (backend->socket_fd, MAX_CONNECTIONS);
  setup_connection (backend);
}

static void
unique_backend_bacon_close_connection (UniqueBackendBacon *backend)
{
  if (backend->source_id)
    {
      g_source_remove (backend->source_id);
      backend->source_id = 0;
    }

  if (backend->channel)
    {
      g_io_channel_shutdown (backend->channel, FALSE, NULL);
      g_io_channel_unref (backend->channel);
      backend->channel = NULL;
    }

  if (backend->socket_fd != -1)
    close (backend->socket_fd);

  if (g_unlink (backend->socket_path) == -1)
    {
      if (errno != ENOENT)
        g_warning ("Unable to remove old socket file: %s",
                   g_strerror (errno));
    }

  g_slist_foreach (backend->connections, (GFunc) g_object_unref, NULL);
  g_slist_free (backend->connections);
}

static void
unique_backend_bacon_finalize (GObject *gobject)
{
  UniqueBackendBacon *backend_bacon;
  
  backend_bacon = UNIQUE_BACKEND_BACON (gobject);

  if (backend_bacon->is_server || backend_bacon->connections)
    unique_backend_bacon_close_connection (backend_bacon);

  g_free (backend_bacon->socket_path);

  G_OBJECT_CLASS (unique_backend_bacon_parent_class)->finalize (gobject);
}

static UniqueResponse
unique_backend_bacon_send_message (UniqueBackend     *backend,
                                   gint               command_id,
                                   UniqueMessageData *message,
                                   guint              time_)
{
  UniqueBackendBacon *backend_bacon;
  UniqueResponse response_id;
  gchar *packed, *response;
  gsize packed_len;
  GString *resp_buffer;
  gsize res, term;
  gchar buf;

  backend_bacon = UNIQUE_BACKEND_BACON (backend);

  if (!try_client (backend_bacon))
    {
      g_warning ("Unable to send message: no connection to the "
                 "running instance found (stale named pipe)");
      
      /* force removal of the named pipe */
      if (g_unlink (backend_bacon->socket_path) == -1)
        {
          if (errno != ENOENT)
            {
              g_warning ("Unable to remove stale named pipe: %s",
                         g_strerror (errno));
            }
        }

      if (!try_client (backend_bacon))
        return UNIQUE_RESPONSE_FAIL;
    }

  packed = unique_message_data_pack (backend->parent,
                                     command_id, message, time_,
                                     &packed_len);
  if (write (backend_bacon->socket_fd, packed, packed_len) == -1)
    {
      g_warning ("Unable to send message: %s", g_strerror (errno));
      g_free (packed);

      return UNIQUE_RESPONSE_FAIL;
    }
  else
    fsync (backend_bacon->socket_fd);

  g_free (packed);

  resp_buffer = g_string_new (NULL);
  buf = term = res = 0;

  res = read (backend_bacon->socket_fd, &buf, 1);
  while (res > 0 && buf != '\n')
    {
      resp_buffer = g_string_append_c (resp_buffer, buf);
      term += res;

      res = read (backend_bacon->socket_fd, &buf, 1);
      if (res < 0)
        {
          g_warning ("Unable to receive the response: %s", g_strerror (errno));

          close (backend_bacon->socket_fd);
          backend_bacon->socket_fd = -1;

          g_string_free (resp_buffer, TRUE);

          return UNIQUE_RESPONSE_FAIL;
        }
    }

  response = g_string_free (resp_buffer, FALSE);
  response[term - 1] = '\0';

  response_id = unique_response_from_string (response);

  g_free (response);

  return response_id;
}

static gboolean
unique_backend_bacon_request_name (UniqueBackend *backend)
{
  UniqueBackendBacon *backend_bacon;
  const gchar *name;

  g_return_val_if_fail (UNIQUE_IS_BACKEND (backend), FALSE);

  name = unique_backend_get_name (backend);
  g_assert (name != NULL);

  backend_bacon = UNIQUE_BACKEND_BACON (backend);

  g_assert (backend_bacon->socket_path == NULL);
  backend_bacon->socket_path = find_socket_file (name);
  if (!is_socket (backend_bacon->socket_path))
    {
      create_server (backend_bacon);
      backend_bacon->is_server = TRUE;
    }
  else
    {
      if (!try_client (backend_bacon))
        {
          if (g_unlink (backend_bacon->socket_path) == -1)
            {
              if (errno != ENOENT)
                g_warning ("Unable to remove stale pipe: %s",
                           g_strerror (errno));
            }

          create_server (backend_bacon);
          backend_bacon->is_server = TRUE;
        }
      else
        backend_bacon->is_server = FALSE;
    }

  return backend_bacon->is_server;
}

static void
unique_backend_bacon_class_init (UniqueBackendBaconClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  UniqueBackendClass *backend_class = UNIQUE_BACKEND_CLASS (klass);

  gobject_class->finalize = unique_backend_bacon_finalize;

  backend_class->request_name = unique_backend_bacon_request_name;
  backend_class->send_message = unique_backend_bacon_send_message;
}

static void
unique_backend_bacon_init (UniqueBackendBacon *backend_bacon)
{
  backend_bacon->is_server = FALSE;

  backend_bacon->socket_fd = -1;
  backend_bacon->socket_path = NULL;
}
