/* Unique - Single Instance Application library
 * uniquemessage-bacon.h: Packing functions for UniqueMessageData
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

#ifndef __UNIQUE_MESSAGE_BACON_H__
#define __UNIQUE_MESSAGE_BACON_H__

#include <glib.h>
#include <unique/uniquemessage.h>

G_BEGIN_DECLS

gchar *            unique_message_data_pack   (UniqueApp         *app,
                                               gint               command_id,
                                               UniqueMessageData *message_data,
                                               guint              time_,
                                               gsize             *length);
UniqueMessageData *unique_message_data_unpack (UniqueApp         *app,
                                               const gchar       *data,
                                               gint              *command_id,
                                               guint             *time_);

G_END_DECLS

#endif /* __UNIQUE_MESSAGE_BACON_H__ */
