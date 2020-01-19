/* Unique - Single Instance Backendlication library
 * uniquemessage.c: Wrapper around messages
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

#include <config.h>

#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include "uniquemessage.h"
#include "uniqueinternals.h"

/**
 * SECTION:unique-message
 * @short_description: Message container for UniqueApp
 *
 * #UniqueMessageData contains the data sent from a #UniqueApp to a
 * running instance of the same application. It can contain arbitrary
 * binary data, and provides convenience functions to set plain text
 * or URI list.
 *
 * You should create a #UniqueMessageData structure using
 * unique_message_data_new(), you can copy it using the
 * unique_message_data_copy() and you should free it using
 * unique_message_data_free().
 *
 * You can set data using unique_message_data_set(),
 * unique_message_data_set_text(), unique_message_data_set_filename()
 * or unique_message_data_set_uris().
 *
 * You can retrieve the data set using unique_message_data_get(),
 * unique_message_data_get_text(), unique_message_data_get_filename()
 * or unique_message_data_get_uris().
 */

GType
unique_message_data_get_type (void)
{
  static GType data_type = 0;

  if (G_UNLIKELY (data_type == 0))
    {
      data_type =
        g_boxed_type_register_static (I_("UniqueMessageData"),
                                      (GBoxedCopyFunc) unique_message_data_copy,
                                      (GBoxedFreeFunc) unique_message_data_free);
    }

  return data_type;
}

/**
 * unique_message_data_new:
 *
 * Creates a new #UniqueMessageData structure. This structure holds the
 * message data passed between running instances with
 * unique_app_send_message().
 *
 * Return value: the newly created #UniqueMessageData
 */
UniqueMessageData *
unique_message_data_new (void)
{
  return g_slice_new0 (UniqueMessageData);
}

/**
 * unique_message_data_copy:
 * @message_data: a #UniqueMessageData
 *
 * Copies @message_data.
 *
 * Return value: (transfer full): a copy of the passed #UniqueMessageData
 */
UniqueMessageData *
unique_message_data_copy (UniqueMessageData *message_data)
{
  UniqueMessageData *retval;

  retval = g_slice_new (UniqueMessageData);
  *retval = *message_data;

  if (message_data->data)
    {
      retval->data = g_malloc (message_data->length + 1);
      memcpy (retval->data, message_data->data, message_data->length + 1);
    }

  retval->screen = message_data->screen;
  retval->startup_id = g_strdup (message_data->startup_id);

  return retval;
}

/**
 * unique_message_data_free:
 * @message_data: a #UniqueMessageData
 *
 * Frees all the resources allocated by @message_data.
 */
void
unique_message_data_free (UniqueMessageData *message_data)
{
  if (G_LIKELY (message_data))
    {
      g_free (message_data->startup_id);
      g_free (message_data->data);

      g_slice_free (UniqueMessageData, message_data);
    }
}

/**
 * unique_message_data_set:
 * @message_data: a #UniqueMessageData
 * @data: (allow-none): binary blob to set, or %NULL
 * @length: length of @data
 *
 * Sets @data as the payload of @message_data. Any other data is removed
 * from the message data. If @data is %NULL, a @length of -1 will unset
 * the payload, while a @length of 0 will set the payload to an empty
 * string.
 *
 * You can use unique_message_data_get() to retrieve the data.
 */
void
unique_message_data_set (UniqueMessageData *message_data,
                         const guchar      *data,
                         gssize             length)
{
  g_return_if_fail (message_data != NULL);

  g_free (message_data->data);

  if (data)
    {
      message_data->data = g_new (guchar, length + 1);
      memcpy (message_data->data, data, length);
      message_data->data[length] = 0;
    }
  else
    {
      g_return_if_fail (length <= 0);

      if (length < 0)
        message_data->data = NULL;
      else
        message_data->data = (guchar *) g_strdup ("");
    }

  message_data->length = length >= 0 ? length : 0;
}

/**
 * unique_message_data_get:
 * @message_data: a #UniqueMessageData
 * @length: (out): return location for the length of the contents
 *
 * Retrieves the raw contents of @message_data set using
 * unique_messaget_data_set().
 *
 * Return value: the contents of the message data or %NULL. The
 *   returned string is owned by the #UniqueMessageData and should
 *   never be modified or freed
 *
 * Since: 1.0.2
 */
const guchar *
unique_message_data_get (UniqueMessageData *message_data,
                         gsize             *length)
{
  g_return_val_if_fail (message_data != NULL, NULL);

  if (length)
    *length = message_data->length;

  return message_data->data;
}

/* taken from gtkselection.c */
/* normalize \r and \n into \r\n */
static gchar *
normalize_to_crlf (const gchar *str, 
                   gint         len)
{
  GString *result = g_string_sized_new (len);
  const gchar *p = str;

  while (1)
    {
      if (*p == '\n')
        g_string_append_c (result, '\r');

      if (*p == '\r')
        {
          g_string_append_c (result, *p);
          p++;
          if (*p != '\n')
            g_string_append_c (result, '\n');
        }

      if (*p == '\0')
        break;

      g_string_append_c (result, *p);
      p++;
    }

  return g_string_free (result, FALSE);  
}

/* taken from gtkselection.c */
/* normalize \r and \r\n into \n */
static gchar *
normalize_to_lf (gchar *str, 
                 gint   len)
{
  GString *result = g_string_sized_new (len);
  const gchar *p = str;

  while (1)
    {
      if (*p == '\r')
        {
          p++;
          if (*p != '\n')
            g_string_append_c (result, '\n');
        }

      if (*p == '\0')
        break;

      g_string_append_c (result, *p);
      p++;
    }

  return g_string_free (result, FALSE);  
}

static gboolean
message_data_set_text_plain (UniqueMessageData *message_data,
                             const gchar       *str,
                             gsize              length)
{
  const gchar *charset = NULL;
  gchar *result, *tmp;
  GError *error = NULL;

  result = normalize_to_crlf (str, length);
  
  g_get_charset (&charset);
  if (!charset)
    charset = "ASCII";

  tmp = result;
  result = g_convert_with_fallback (tmp, -1, charset, "UTF-8",
                                    NULL, NULL, NULL,
                                    &error);
  g_free (tmp);

  if (!result)
    {
      g_warning ("Error converting from %s to %s: %s",
                 "UTF-8", charset, error->message);
      g_error_free (error);

      return FALSE;
    }

  unique_message_data_set (message_data, (guchar *) result, strlen (result));
  
  return TRUE;
}

static gchar *
message_data_get_text_plain (UniqueMessageData *message_data)
{
  const gchar *charset = NULL;
  gchar *str, *tmp, *result;
  gsize len;
  GError *error = NULL;

  if (message_data->length == 0)
    return NULL;

  str = g_strdup ((gchar *) message_data->data);
  len = message_data->length;

  if (!g_utf8_validate (str, -1, NULL))
    {
      g_get_charset (&charset);
      if (!charset)
        charset = "ISO-8859-1";

      tmp = str;
      str = g_convert_with_fallback (tmp, len,
                                     charset, "UTF-8",
                                     NULL, NULL, &len,
                                     &error);
      g_free (tmp);
    }

  if (!str)
    {
      g_warning ("Error converting from %s to %s: %s",
                 charset, "UTF-8", error->message);
      g_error_free (error);

      return NULL;
    }
  else if (!g_utf8_validate (str, -1, NULL))
    {
      g_warning ("Error converting from %s to %s: %s",
                 "text/plain;charset=utf-8", "UTF-8", "invalid UTF-8");
      g_free (str);

      return NULL;
    }

  result = normalize_to_lf (str, len);
  g_free (str);

  return result;
}

/**
 * unique_message_data_set_text:
 * @message_data: a #UniqueMessageData
 * @str: plain text to be set as payload
 * @length: length of the text, or -1
 *
 * Sets @str as the plain text payload of @message_data, converting it
 * to UTF-8 if needed. If @length is -1, the length of the string will
 * be used. Use unique_message_data_get_text() to retrieve the text.
 *
 * Return value: %TRUE if the text was successfully converted to UTF-8
 */
gboolean
unique_message_data_set_text (UniqueMessageData *message_data,
                              const gchar       *str,
                              gssize             length)
{
  if (length < 0)
    length = strlen (str);

  if (g_utf8_validate (str, length, NULL))
    {
      unique_message_data_set (message_data, (guchar *) str, length);
      return TRUE;
    }

  return message_data_set_text_plain (message_data, str, length);
}

/**
 * unique_message_data_get_text:
 * @message_data: a #UniqueMessageData
 *
 * Retrieves the text set using unique_message_data_set_text().
 *
 * Return value: (transfer full): a newly-allocated string.
 */
gchar *
unique_message_data_get_text (UniqueMessageData *message_data)
{
  return message_data_get_text_plain (message_data);
}

/**
 * unique_message_data_set_uris:
 * @message_data: a #UniqueMessageData
 * @uris: (array zero-terminated=1) (element-type utf8): a list of URIs
 *   in a %NULL-terminated string vector
 *
 * Converts @uris to a valid URI list and sets it as payload of
 * @message_data. You can use unique_message_data_get_uris() to
 * retrieve the list from a #UniqueMessageData.
 *
 * Return value: %TRUE if the URIs were successfully converted
 */
gboolean
unique_message_data_set_uris (UniqueMessageData  *message_data,
                              gchar             **uris)
{
  GString *list;
  gint i;
  gchar *result;
  gsize length;

  list = g_string_new (NULL);
  for (i = 0; uris[i]; i++)
    {
      g_string_append (list, uris[i]);
      g_string_append (list, "\r\n");
    }

  result = g_convert (list->str, list->len,
                      "ASCII", "UTF-8",
                      NULL, &length, NULL);
  g_string_free (list, TRUE);

  if (result)
    {
      unique_message_data_set (message_data, (guchar *) result, length);
      g_free (result);

      return TRUE;
    }

  return FALSE;
}

/**
 * unique_message_data_get_uris:
 * @message_data: a #UniqueMessageData
 *
 * Retrieves a %NULL-terminated string vector containing the URIs set with
 * unique_message_data_set_uris().
 *
 * Return value: (array zero-terminated=1) (element-type utf8) (transfer full): a newly-allocated,
 *   %NULL-terminated list of URIs. Use g_strfreev() to free it.
 */
gchar **
unique_message_data_get_uris (UniqueMessageData *message_data)
{
  gchar **result = NULL;

  if (message_data->length >= 0)
    {
      gchar *text;

      text = message_data_get_text_plain (message_data);
      if (text)
        {
          result = g_uri_list_extract_uris (text);
          g_free (text);
        }
    }

  return result;
}

/**
 * unique_message_data_set_filename:
 * @message_data: a #UniqueMessageData
 * @filename: a filename
 *
 * Sets @filename as the contents of @message_data.
 *
 * Since: 1.0.2
 */
void
unique_message_data_set_filename (UniqueMessageData *message_data,
                                  const gchar       *filename)
{
  g_return_if_fail (message_data != NULL);
  g_return_if_fail (filename != NULL);

  unique_message_data_set (message_data,
                           (const guchar *) filename,
                           strlen (filename));
}

/**
 * unique_message_data_get_filename:
 * @message_data: a #UniqueMessageData
 *
 * Retrieves the filename set with unique_message_data_set_filename().
 *
 * Return value: (transfer full): a newly allocated string containing the
 *   filename. Use g_free() to free the resources used by the returned
 *   value.
 *
 * Since: 1.0.2
 */
gchar *
unique_message_data_get_filename (UniqueMessageData *message_data)
{
  g_return_val_if_fail (message_data != NULL, NULL);

  return g_memdup (message_data->data, message_data->length + 1);
}

/**
 * unique_message_data_get_screen:
 * @message_data: a #UniqueMessageData
 *
 * Returns a pointer to the screen from where the message came. You
 * can use gtk_window_set_screen() to move windows or dialogs to the
 * right screen. This field is always set by the Unique library.
 *
 * Return value: (transfer none): a #GdkScreen
 */
GdkScreen *
unique_message_data_get_screen (UniqueMessageData *message_data)
{
  g_return_val_if_fail (message_data != NULL, NULL);

  return message_data->screen;
}

/**
 * unique_message_data_get_startup_id:
 * @message_data: a #UniqueMessageData
 *
 * Retrieves the startup notification id set inside @message_data. This
 * field is always set by the Unique library.
 *
 * Return value: the startup notification id. The returned string is
 *   owned by the #UniqueMessageData structure and should not be
 *   modified or freed
 */
const gchar *
unique_message_data_get_startup_id (UniqueMessageData *message_data)
{
  g_return_val_if_fail (message_data != NULL, NULL);

  return message_data->startup_id;
}

/**
 * unique_message_data_get_workspace:
 * @message_data: a #UniqueMessageData
 *
 * Retrieves the workspace number from where the message came. This
 * field is always set by the Unique library.
 *
 * Return value: the workspace number
 */
guint
unique_message_data_get_workspace (UniqueMessageData *message_data)
{
  g_return_val_if_fail (message_data != NULL, 0);

  return message_data->workspace;
}
