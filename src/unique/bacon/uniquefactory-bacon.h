/* Unique - Single Instance Backendlication library
 * uniquefactory-bacon.h: Unix domain socket implementation of UniqueBackend
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

#ifndef __UNIQUE_FACTORY_BACON_H__
#define __UNIQUE_FACTORY_BACON_H__

#include <glib-object.h>
#include <unique/uniqueapp.h>

G_BEGIN_DECLS

#define UNIQUE_TYPE_FACTORY_BACON            (unique_factory_bacon_get_type ())
#define UNIQUE_FACTORY_BACON(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), UNIQUE_TYPE_FACTORY_BACON, UniqueFactoryBacon))
#define UNIQUE_IS_FACTORY_BACON(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), UNIQUE_TYPE_FACTORY_BACON))
#define UNIQUE_FACTORY_BACON_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), UNIQUE_TYPE_FACTORY_BACON, UniqueFactoryBaconClass))
#define UNIQUE_IS_FACTORY_BACON_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), UNIQUE_TYPE_FACTORY_BACON))
#define UNIQUE_FACTORY_BACON_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), UNIQUE_TYPE_FACTORY_BACON, UniqueFactoryBaconClass))

typedef struct _UniqueFactoryBacon            UniqueFactoryBacon;
typedef struct _UniqueFactoryBaconClass       UniqueFactoryBaconClass;

struct _UniqueFactoryBacon
{
  GObject parent_class;

  UniqueApp *parent;
  
  gint socket_fd;

  GIOChannel *channel;
  guint source_id;
};

struct _UniqueFactoryBaconClass
{
  GObjectClass parent_class;
};

GType    unique_factory_bacon_get_type (void) G_GNUC_CONST;
gboolean unique_factory_bacon_accept   (UniqueFactoryBacon *factory,
                                        gint                socket_fd);
gint     unique_factory_bacon_get_fd   (UniqueFactoryBacon *factory);

G_END_DECLS

#endif /* __UNIQUE_FACTORY_BACON_H__ */
