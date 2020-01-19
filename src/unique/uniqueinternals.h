#ifndef __UNIQUE_INTERNALS_H__
#define __UNIQUE_INTERNALS_H__

#include <time.h>

#include "uniqueapp.h"
#include "uniquebackend.h"
#include "uniquemessage.h"
#include "uniqueversion.h"

G_BEGIN_DECLS

struct _UniqueMessageData
{
  guchar *data;
  gint length;  /* length of data not including NUL terminator */

  GdkScreen *screen;
  gchar *startup_id;
  guint workspace;
};

/* GObject ought to export symbols like these */
#define UNIQUE_PARAM_READABLE   (G_PARAM_READABLE | \
                                 G_PARAM_STATIC_NAME | \
                                 G_PARAM_STATIC_NICK | \
                                 G_PARAM_STATIC_BLURB)
#define UNIQUE_PARAM_READWRITE  (G_PARAM_READABLE | G_PARAM_WRITABLE | \
                                 G_PARAM_STATIC_NAME | \
                                 G_PARAM_STATIC_NICK | \
                                 G_PARAM_STATIC_BLURB)

#define I_(str) (g_intern_static_string ((str)))

/* this method emits the UniqueApp::message-received signal on app; it
 * should be called by the backend on its parent UniqueApp instance.
 */
UniqueResponse unique_app_emit_message_received (UniqueApp         *app,
                                                 gint               command_id,
                                                 UniqueMessageData *message,
                                                 guint              time_);

/* transform a command or a response id to something more readable,
 * and then back into an id
 */
UniqueResponse        unique_response_from_string  (const gchar    *response);
const gchar *         unique_response_to_string    (UniqueResponse  response);

gint                  unique_command_from_string   (UniqueApp      *app,
                                                    const gchar    *command);
const gchar *         unique_command_to_string     (UniqueApp      *app,
                                                    gint            command);

G_END_DECLS

#endif /* __UNIQUE_INTERNALS_H__ */
