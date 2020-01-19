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

#ifndef __UNIQUE_FACTORY_DBUS_H__
#define __UNIQUE_FACTORY_DBUS_H__

#include <glib-object.h>
#include <unique/uniqueapp.h>

G_BEGIN_DECLS

#define UNIQUE_TYPE_FACTORY_DBUS            (unique_factory_dbus_get_type ())
#define UNIQUE_FACTORY_DBUS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), UNIQUE_TYPE_FACTORY_DBUS, UniqueFactoryDBus))
#define UNIQUE_IS_FACTORY_DBUS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), UNIQUE_TYPE_FACTORY_DBUS))
#define UNIQUE_FACTORY_DBUS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), UNIQUE_TYPE_FACTORY_DBUS, UniqueFactoryDBusClass))
#define UNIQUE_IS_FACTORY_DBUS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), UNIQUE_TYPE_FACTORY_DBUS))
#define UNIQUE_FACTORY_DBUS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), UNIQUE_TYPE_FACTORY_DBUS, UniqueFactoryDBusClass))

typedef struct _UniqueFactoryDBus            UniqueFactoryDBus;
typedef struct _UniqueFactoryDBusClass       UniqueFactoryDBusClass;

struct _UniqueFactoryDBus
{
  GObject parent_class;

  UniqueApp *parent;
  UniqueBackend *backend;
};

struct _UniqueFactoryDBusClass
{
  GObjectClass parent_class;
};

GType unique_factory_dbus_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __UNIQUE_FACTORY_DBUS_H__ */
