/*
 * app.c
 *
 *  Created on: Jun 27, 2018
 *      Author: konstantinnosach
 */
#include <gtk/gtk.h>
#include <stdlib.h>
#include "loading.h"
#include "gui_color.h"

GtkWidget * login_window;
GtkWidget * main_window;
GtkWidget * loading_window;

int main (int argc, char **argv)
{
  gtk_init(&argc, &argv);
  style_css();
  GtkApplication *app;
  int status;
  app = gtk_application_new ("com.mobius-software.iotbroker.cclient", G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (activate_loading_window), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}


