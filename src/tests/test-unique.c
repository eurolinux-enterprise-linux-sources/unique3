/* Unique - Single Instance Application library
 *
 * Copyright (C) 2007  Emmanuele Bassi  <ebassi@o-hand.com>
 */

#include <stdlib.h>
#include <gtk/gtk.h>
#include <unique/unique.h>

enum {
  COMMAND_0,

  COMMAND_FOO,
  COMMAND_BAR
};

static GtkWidget *main_window = NULL;

static UniqueResponse
app_message_cb (UniqueApp         *app,
                gint               command,
                UniqueMessageData *message_data,
                guint              time_,
                gpointer           user_data)
{
  GtkWidget *dialog;
  gchar *message = NULL;
  gchar *title = NULL;
  GdkScreen *screen;
  const gchar *startup_id;

  screen = unique_message_data_get_screen (message_data);
  startup_id = unique_message_data_get_startup_id (message_data);

  g_print ("Message received from screen: %d, startup-id: %s, workspace: %d\n",
           gdk_screen_get_number (screen),
           startup_id,
           unique_message_data_get_workspace (message_data));

  /* raise the window */
  gtk_window_present_with_time (GTK_WINDOW (main_window), time_);

  switch (command)
    {
    case UNIQUE_NEW:
      title = g_strdup ("Received the NEW command");
      break;
    case UNIQUE_OPEN:
      {
        gchar **uris;
        gint n_uris, i;
        GString *buf;

        title = g_strdup ("Received the OPEN command");
        
        uris = unique_message_data_get_uris (message_data);
        n_uris = g_strv_length (uris);
        buf = g_string_new (NULL);
        for (i = 0; i < n_uris; i++)
          g_string_append_printf (buf, "uri: %s\n", uris[i]);

        message = g_string_free (buf, FALSE);
        g_strfreev (uris);
      }
      break;
    case UNIQUE_ACTIVATE:
      title = g_strdup ("Received the ACTIVATE command");
      break;
    case COMMAND_FOO:
      title = g_strdup ("Received the FOO command");
      message = unique_message_data_get_text (message_data);
      break;
    case COMMAND_BAR:
      title = g_strdup ("Received the BAR command");
      message = g_strdup ("Thid command doesn't do anything special");
      break;
    default:
      break;
    }

  dialog = gtk_message_dialog_new (GTK_WINDOW (main_window),
                                   GTK_DIALOG_DESTROY_WITH_PARENT,
                                   GTK_MESSAGE_INFO,
                                   GTK_BUTTONS_CLOSE,
                                   "%s",
                                   title);
  if (message)
    gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
                                              "%s",
                                              message);

  gtk_window_set_urgency_hint (GTK_WINDOW (dialog), TRUE);

  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);

  g_free (message);
  g_free (title);

  return UNIQUE_RESPONSE_OK;
}

int
main (int argc, char *argv[])
{
  UniqueApp *app;
  gboolean new = FALSE;
  gboolean activate = FALSE;
  gboolean foo = FALSE;
  gchar **uris = NULL;
  GError *init_error = NULL;
  GOptionEntry entries[] = {
    { "new", 'n',
      0,
      G_OPTION_ARG_NONE, &new,
      "Send 'new' command", NULL,
    },
    { "open-uri", 'o',
      0,
      G_OPTION_ARG_STRING_ARRAY, &uris,
      "Send 'open' command", "URI",
    },
    {
      "activate", 'a',
      0,
      G_OPTION_ARG_NONE, &activate,
      "Send 'activate' command", NULL,
    },
    {
      "foo", 'f',
      0,
      G_OPTION_ARG_NONE, &foo,
      "Send 'foo' command", NULL,
    },
    { NULL },
  };

  gtk_init_with_args (&argc, &argv,
                      "Test GtkUnique",
                      entries,
                      NULL,
                      &init_error);
  if (init_error)
    {
      g_print ("*** Error: %s\n"
               "Usage: test-unique [COMMAND]\n",
               init_error->message);
      g_error_free (init_error);

      exit (1);
    }

  app = unique_app_new_with_commands ("org.gnome.TestUnique", NULL,
                                      "foo", COMMAND_FOO,
                                      "bar", COMMAND_BAR,
                                      NULL);
  if (unique_app_is_running (app))
    {
      UniqueMessageData *message = NULL;
      UniqueResponse response;
      gint command;


      if (new)
        command = UNIQUE_NEW;
      else if (uris && uris[0])
        {
          command = UNIQUE_OPEN;
      
          message = unique_message_data_new ();
          unique_message_data_set_uris (message, uris); 
        }
      else if (activate)
        command = UNIQUE_ACTIVATE;
      else if (foo)
        {
          command = COMMAND_FOO;
          
          message = unique_message_data_new ();
          unique_message_data_set (message, (const guchar *) "bar", 3);
        }
      else
        command = COMMAND_BAR;
      
      if (message)
        {
          response = unique_app_send_message (app, command, message);
          unique_message_data_free (message);
        }
      else
        response = unique_app_send_message (app, command, NULL);

      g_print ("Response code: %d\n", response);

      gdk_notify_startup_complete ();
      
      g_object_unref (app);

      return (response == UNIQUE_RESPONSE_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
    }
  else
    {
      main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
      g_signal_connect (main_window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
      gtk_window_set_title (GTK_WINDOW (main_window), "Test GtkUnique");
      gtk_window_set_default_size (GTK_WINDOW (main_window), 400, 300);
      gtk_container_set_border_width (GTK_CONTAINER (main_window), 12);

      unique_app_watch_window (app, GTK_WINDOW (main_window));
      g_signal_connect (app, "message-received",
                        G_CALLBACK (app_message_cb), NULL);

      gtk_widget_show (main_window);
    }
  
  gtk_main ();

  g_object_unref (app);

  return EXIT_SUCCESS;
}
