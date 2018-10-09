/*
 * modify_background.c
 *
 *  Created on: Aug 11, 2018
 *      Author: konstantinnosach
 */
#include <gtk/gtk.h>

void style_css(void){

    GtkCssProvider *provider;
    GdkDisplay *display;
    GdkScreen *screen;

    provider = gtk_css_provider_new ();
    display = gdk_display_get_default ();
    screen = gdk_display_get_default_screen (display);
    gtk_style_context_add_provider_for_screen (screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    const gchar *my_css_file = "./style.css";
    GError *error = 0;

    gtk_css_provider_load_from_file(provider, g_file_new_for_path(my_css_file), &error);
    g_object_unref (provider);
}
