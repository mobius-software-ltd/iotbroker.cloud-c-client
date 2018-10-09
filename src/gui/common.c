/*
 * common.c
 *
 *  Created on: Jun 27, 2018
 *      Author: konstantinnosach
 */
#include <stdlib.h>
#include <gtk/gtk.h>
#include "../mqtt_listener.h"

void quit (GtkWidget *widget, GdkEvent  *event, gpointer data)
{
	struct MqttListener * listener = (struct MqttListener * )data;
	listener->send_disconnect();
	exit(0);
}
