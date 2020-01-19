/* Unique - Single Instance Backendlication library
 * uniquebackend.c: Base class for IPC transports
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

/**
 * SECTION:unique-backend
 * @short_description: Backend abstraction
 *
 * #UniqueBackend is the base, abstract class implemented by the different
 * IPC mechanisms used by Unique. Each #UniqueApp instance creates a
 * #UniqueBackend to request the name or to send messages.
 */

#include <config.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <gdk/gdk.h>
#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#include <X11/Xatom.h>
#endif

#include "uniquebackend.h"
#include "uniqueinternals.h"

G_DEFINE_ABSTRACT_TYPE (UniqueBackend, unique_backend, G_TYPE_OBJECT);

static void
unique_backend_finalize (GObject *gobject)
{
  UniqueBackend *backend = UNIQUE_BACKEND (gobject);

  g_free (backend->name);
  g_free (backend->startup_id);

  G_OBJECT_CLASS (unique_backend_parent_class)->finalize (gobject);
}

static void
unique_backend_class_init (UniqueBackendClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = unique_backend_finalize;
}

static void
unique_backend_init (UniqueBackend *backend)
{
  backend->name = NULL;
  backend->startup_id = NULL;
  backend->screen = gdk_screen_get_default ();
  backend->workspace = -1;
}

/**
 * unique_backend_set_name:
 * @backend: FIXME
 * @name: FIXME
 *
 * FIXME
 */
void
unique_backend_set_name (UniqueBackend *backend,
                         const gchar   *name)
{
  g_return_if_fail (UNIQUE_IS_BACKEND (backend));
  g_return_if_fail (name != NULL);

  if (!backend->name)
    {
      backend->name = g_strdup (name);
      return;
    }

  if (strcmp (backend->name, name) != 0)
    {
      g_free (backend->name);
      backend->name = g_strdup (name);
    }
}

/**
 * unique_backend_get_name:
 * @backend: FIXME
 *
 * FIXME
 *
 * Return value: FIXME
 */
const gchar *
unique_backend_get_name (UniqueBackend *backend)
{
  g_return_val_if_fail (UNIQUE_IS_BACKEND (backend), NULL);

  return backend->name;
}

/**
 * unique_backend_set_startup_id:
 * @backend: FIXME
 * @startup_id: FIXME
 *
 * FIXME
 */
void
unique_backend_set_startup_id (UniqueBackend *backend,
                               const gchar   *startup_id)
{
  g_return_if_fail (UNIQUE_IS_BACKEND (backend));
  g_return_if_fail (startup_id != NULL);

  if (!backend->startup_id)
    {
      backend->startup_id = g_strdup (startup_id);
      return;
    }

  if (strcmp (backend->startup_id, startup_id) != 0)
    {
      g_free (backend->startup_id);
      backend->startup_id = g_strdup (startup_id);
    }
}

/**
 * unique_backend_get_startup_id:
 * @backend: FIXME
 *
 * FIXME
 *
 * Return value: FIXME
 */
const gchar *
unique_backend_get_startup_id (UniqueBackend *backend)
{
  g_return_val_if_fail (UNIQUE_IS_BACKEND (backend), NULL);

  return backend->startup_id;
}

/**
 * unique_backend_set_screen:
 * @backend: FIXME
 * @screen: FIXME
 *
 * FIXME
 */
void
unique_backend_set_screen (UniqueBackend *backend,
                           GdkScreen     *screen)
{
  g_return_if_fail (UNIQUE_IS_BACKEND (backend));
  g_return_if_fail (screen == NULL || GDK_IS_SCREEN (screen));

  if (!screen)
    screen = gdk_screen_get_default ();

  backend->screen = screen;
}

/**
 * unique_backend_get_screen:
 * @backend: FIXME
 *
 * FIXME
 *
 * Return value: (transfer none): FIXME
 */
GdkScreen *
unique_backend_get_screen (UniqueBackend *backend)
{
  g_return_val_if_fail (UNIQUE_IS_BACKEND (backend), NULL);

  return backend->screen;
}

/**
 * unique_backend_get_workspace:
 * @backend: a #UniqueBackend
 *
 * Retrieves the current workspace.
 *
 * Return value: a workspace number
 */
guint
unique_backend_get_workspace (UniqueBackend *backend)
{
  GdkDisplay *display;
  GdkWindow *root_win;
#ifdef GDK_WINDOWING_X11
  Atom _net_current_desktop, type;
  int format;
  unsigned long n_items, bytes_after;
  unsigned char *data_return = 0;
#endif

  g_return_val_if_fail (UNIQUE_IS_BACKEND (backend), 0);

  if (backend->workspace != -1)
    return backend->workspace;

  display = gdk_screen_get_display (backend->screen);
  root_win = gdk_screen_get_root_window (backend->screen);

#ifdef GDK_WINDOWING_X11
  _net_current_desktop =
    gdk_x11_get_xatom_by_name_for_display (display, "_NET_CURRENT_DESKTOP");

  XGetWindowProperty (GDK_DISPLAY_XDISPLAY (display),
                      GDK_WINDOW_XID (root_win),
                      _net_current_desktop,
                      0, G_MAXLONG,
                      False, XA_CARDINAL,
                      &type, &format, &n_items, &bytes_after,
                      &data_return);

  if (type == XA_CARDINAL && format == 32 && n_items > 0)
    {
      backend->workspace = (guint) data_return[0];
      XFree (data_return);
    }
#endif

  return backend->workspace;
}

/**
 * unique_backend_request_name:
 * @backend: a #UniqueBackend
 *
 * Requests the name set using unique_backend_set_name() using @backend.
 *
 * Return value: %TRUE if the name was assigned to us, %FALSE if there
 *   already is a registered name
 */
gboolean
unique_backend_request_name (UniqueBackend *backend)
{
  g_return_val_if_fail (UNIQUE_IS_BACKEND (backend), FALSE);

  return UNIQUE_BACKEND_GET_CLASS (backend)->request_name (backend);
}

/**
 * unique_backend_send_message:
 * @backend: a #UniqueBackend
 * @command_id: command to send
 * @message_data: message to send, or %NULL
 * @time_: time of the command emission, or 0 for the current time
 *
 * Sends @command_id, and optionally @message_data, to a running instance
 * using @backend.
 *
 * Return value: a #UniqueResponse value sent by the running instance
 */
UniqueResponse
unique_backend_send_message (UniqueBackend     *backend,
                             gint               command_id,
                             UniqueMessageData *message_data,
                             guint              time_)
{
  g_return_val_if_fail (UNIQUE_IS_BACKEND (backend), UNIQUE_RESPONSE_INVALID);
  g_return_val_if_fail (command_id != 0, UNIQUE_RESPONSE_INVALID);
  
  if (time_ == 0)
    time_ = (guint) time (NULL);

  return UNIQUE_BACKEND_GET_CLASS (backend)->send_message (backend, command_id,
                                                           message_data,
                                                           time_);
}

#include "bacon/uniquebackend-bacon.h"
#ifdef HAVE_DBUS
#include "dbus/uniquebackend-dbus.h"
#endif
#ifdef HAVE_GDBUS
#include "gdbus/uniquebackend-gdbus.h"
#endif

/**
 * unique_backend_create:
 * 
 * Creates a #UniqueBackend using the default backend defined at
 * compile time. You can override the default backend by setting the
 * <literal>UNIQUE_BACKEND</literal> environment variable with the
 * name of the desired backend.
 *
 * Return value: (transfer full): the newly created #UniqueBackend instance
 */
UniqueBackend *
unique_backend_create (void)
{
  const gchar *backend_name;
  GType backend_gtype = G_TYPE_INVALID;

  backend_name = g_getenv ("UNIQUE_BACKEND");
  if (!backend_name)
    backend_name = UNIQUE_DEFAULT_BACKEND_S;

  if (backend_name && backend_name[0] != '\0')
    {
#ifdef HAVE_BACON
      if (strcmp (backend_name, "bacon") == 0)
        backend_gtype = unique_backend_bacon_get_type ();
#endif
#ifdef HAVE_DBUS
      if (strcmp (backend_name, "dbus") == 0)
        backend_gtype = unique_backend_dbus_get_type ();
#endif /* HAVE_DBUS */
#ifdef HAVE_GDBUS
      if (strcmp (backend_name, "gdbus") == 0)
        backend_gtype = unique_backend_gdbus_get_type ();
#endif /* HAVE_GDBUS */
#if !defined(HAVE_BACON) && !defined(HAVE_DBUS)
#error Need either bacon or dbus
#endif 
    }

  return g_object_new (backend_gtype, NULL);
}
