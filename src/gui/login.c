/**
* Mobius Software LTD
* Copyright 2015-2018, Mobius Software LTD
*
* This is free software; you can redistribute it and/or modify it
* under the terms of the GNU Lesser General Public License as
* published by the Free Software Foundation; either version 2.1 of
* the License, or (at your option) any later version.
*
* This software is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this software; if not, write to the Free
* Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
* 02110-1301 USA, or see the FSF site: http://www.fsf.org.
*/

#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "../account.h"
#include "../dal/cdal.h"
#include "../mqtt/mqtt_client.h"
#include "../mqttsn/mqtt_sn_client.h"
#include "../coap/coap_client.h"
#include "../amqp/amqp_client.h"
#include "main_w.h"
#include "loading.h"

GtkWidget * mqttWidgets[36];
//GtkWidget * box = NULL;
enum Protocol current_protocol = MQTT;
const int nWidget[] = { 0, 1, 2, 3, 4, 5 };
static const char *PROTOCOLS_STRING[] = {"MQTT", "MQTT-SN", "COAP", "AMQP", "WEBSOCKETS"};

struct MqttListener * mqtt_listener;
static struct Account * account;
static GtkApplication* app;
GtkWidget * login_window = NULL;

void activate_login_window();

static void connection_success(void);

void quit_to_list_accounts (GtkWidget *widget, GdkEvent  *event, gpointer data)
{
	gtk_widget_destroy(login_window);
	reload_account_list_window();
}

static GtkWidget * add_image(GtkWidget * label, GtkWidget * grid, gint x, gint y, char* image_name) {

	label = gtk_label_new ("           ");
	gtk_widget_set_hexpand (label, FALSE);
	gtk_widget_set_name (label, image_name);
	gtk_grid_attach (GTK_GRID (grid), label, x, y, 1, 1);
	return label;
}

void warning_window_handler (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	gtk_widget_destroy(widget);
	//gtk_widget_set_sensitive(GTK_WIDGET (data), TRUE);
}

static void above_button(GtkWidget *window, gpointer data)
{
    GdkCursor * c = gdk_cursor_new_for_display(gdk_display_get_default(), GDK_HAND2);
    gdk_window_set_cursor(gtk_widget_get_window(login_window), c);

}
static void out_of_button(GtkWidget *window, gpointer data)
{
    gdk_window_set_cursor(gtk_widget_get_window(login_window), NULL);
}

static void print_mqtt_sn_box() {

	for (int i = 0; i < G_N_ELEMENTS(mqttWidgets); i++) {
		gtk_widget_show(mqttWidgets[i]);
	}
	for (int i = 0; i < G_N_ELEMENTS(nWidget); i++) {
		gtk_widget_hide(mqttWidgets[nWidget[i]]);
	}
}

static void print_amqp_box() {

	for (int i = 0; i < G_N_ELEMENTS(mqttWidgets); i++) {
		gtk_widget_hide(mqttWidgets[i]);
	}

	for(int i = 0; i < 9; i++ )
		gtk_widget_show(mqttWidgets[i]);

	for(int i = 27; i < 36; i++ )
		gtk_widget_show(mqttWidgets[i]);

	for(int i = 12; i < 15; i++ )
		gtk_widget_show(mqttWidgets[i]);

}

static void print_coap_box() {

	for (int i = 0; i < G_N_ELEMENTS(mqttWidgets); i++) {
		gtk_widget_hide(mqttWidgets[i]);
	}
	gtk_widget_show(mqttWidgets[6]);
	gtk_widget_show(mqttWidgets[7]);
	gtk_widget_show(mqttWidgets[8]);
	gtk_widget_show(mqttWidgets[12]);
	gtk_widget_show(mqttWidgets[13]);
	gtk_widget_show(mqttWidgets[14]);

	for(int i = 27; i < 36; i++ )
		gtk_widget_show(mqttWidgets[i]);
}

static void show_warn_window(GtkWidget * curr_widget,const gchar * warning_string) {
	GtkWidget *warning_window;
	warning_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (warning_window), "Warning");
	gtk_window_set_resizable (GTK_WINDOW (warning_window), FALSE);
	gtk_widget_set_size_request (warning_window, 280, 280);
	gtk_container_add(GTK_CONTAINER(warning_window), gtk_label_new(warning_string));
	GtkWidget * parent = gtk_widget_get_parent(curr_widget);
	parent = gtk_widget_get_parent(parent);
	parent = gtk_widget_get_parent(parent);
	parent = gtk_widget_get_parent(parent);
	//gtk_widget_set_sensitive(GTK_WIDGET(parent), FALSE);
	g_signal_connect(G_OBJECT(warning_window), "delete_event", G_CALLBACK(warning_window_handler), parent);
	gtk_widget_show_all(warning_window);
}

char *get_text_of_textview(GtkWidget *text_view) {
    GtkTextIter start, end;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer((GtkTextView *)text_view);
    gchar *text;
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
    return text;
}

static void save_cert_button_handle(GtkWidget *widget, gpointer data) {

	char *text = get_text_of_textview (GTK_WIDGET(data));
	gtk_entry_set_text(GTK_ENTRY(mqttWidgets[32]), text);
	GtkWidget * parent = gtk_widget_get_parent(GTK_WIDGET(data));
	parent = gtk_widget_get_parent(parent);
	parent = gtk_widget_get_parent(parent);
	gtk_widget_destroy(GTK_WIDGET(parent));

}

static void save_will_button_handle(GtkWidget *widget, gpointer data) {

	char *text = get_text_of_textview (GTK_WIDGET(data));
	gtk_entry_set_text(GTK_ENTRY(mqttWidgets[17]), text);
	GtkWidget * parent = gtk_widget_get_parent(GTK_WIDGET(data));
	parent = gtk_widget_get_parent(parent);
	parent = gtk_widget_get_parent(parent);
	gtk_widget_destroy(GTK_WIDGET(parent));

}

static void show_dialog_window(GtkWidget *widget, GdkEvent  *event, gpointer user_data) {

	char * field =  (char*) user_data;

	GtkWidget * scrolled_window = gtk_scrolled_window_new (NULL,NULL);
	gtk_container_set_border_width (GTK_CONTAINER (scrolled_window), 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

	GtkWidget *_button, *text_view;
	GtkWidget *dialog_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (dialog_window), "Enter data");
	gtk_window_set_resizable (GTK_WINDOW (dialog_window), FALSE);
	gtk_widget_set_size_request (dialog_window, 700, 500);

	GtkWidget * _box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);

	text_view = gtk_text_view_new ();
	gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
	gtk_box_pack_start(GTK_BOX(_box), GTK_WIDGET(scrolled_window), TRUE, TRUE, 1);

	_button = gtk_button_new_with_label("OK");
	if(strcmp("cert",field) == 0)
	{
		g_signal_connect(_button, "clicked", G_CALLBACK (save_cert_button_handle), text_view);
	}
	else
	{
		g_signal_connect(_button, "clicked", G_CALLBACK (save_will_button_handle), text_view);
	}

	gtk_box_pack_end(GTK_BOX(_box), GTK_WIDGET(_button), FALSE, TRUE, 1);
	gtk_container_add(GTK_CONTAINER(dialog_window), _box);

	g_signal_connect(G_OBJECT(dialog_window), "delete_event", G_CALLBACK(warning_window_handler), widget);
	gtk_widget_show_all(dialog_window);
}


static void login_button_handle(GtkWidget *widget, gpointer data) {

	account = malloc (sizeof (struct Account));
	GtkWidget * grid = data;
	GtkWidget * curr_widget;
	//Protocol
	curr_widget = gtk_grid_get_child_at(GTK_GRID(grid), 2, 3);
	gint chooser_index = gtk_combo_box_get_active(GTK_COMBO_BOX(curr_widget));
	switch(chooser_index) {
	case 0:
		account->protocol = MQTT;
		break;
	case 1:
		account->protocol = MQTT_SN;
		break;
	case 2:
		account->protocol = COAP;
		break;
	case 3:
		account->protocol = AMQP;
		break;
	case 4:
		account->protocol = WEBSOCKETS;
	}
	//username
	curr_widget = gtk_grid_get_child_at(GTK_GRID(grid), 2, 4);
	const gchar * username = gtk_entry_get_text(GTK_ENTRY(curr_widget));
	if(strlen(username) == 0 || strcmp("Enter User Name",username) ==0) {
		if(current_protocol == MQTT || current_protocol == WEBSOCKETS || current_protocol == AMQP) {
			show_warn_window(curr_widget, "Please enter user name");
			return;
		} else {
			account->username = NULL;
		}
	} else {
		account->username = username;
	}
	//password
	curr_widget = gtk_grid_get_child_at(GTK_GRID(grid), 2, 5);
	const gchar * password = gtk_entry_get_text(GTK_ENTRY(curr_widget));
	if(strlen(password) == 0 || strcmp("Enter Password",password) ==0) {
		if( current_protocol == MQTT || current_protocol == WEBSOCKETS || current_protocol == AMQP) {
			show_warn_window(curr_widget, "Please enter password");
			return;
		} else {
			account->password = NULL;
		}
	} else {
		account->password = password;
	}

	//clientID
	curr_widget = gtk_grid_get_child_at(GTK_GRID(grid), 2, 6);
	const gchar * client_id = gtk_entry_get_text(GTK_ENTRY(curr_widget));
	if((strlen(client_id) == 0 || strcmp("Enter Client ID",client_id) ==0) && current_protocol != COAP) {
		show_warn_window(curr_widget, "Please enter Client ID");
		return;
	}
	else
	{
		account->client_id = client_id;
	}
	//host
	curr_widget = gtk_grid_get_child_at(GTK_GRID(grid), 2, 7);
	const gchar * server_host = gtk_entry_get_text(GTK_ENTRY(curr_widget));
	if(strlen(server_host) == 0 || strcmp("Enter Server Host",server_host) ==0) {
		show_warn_window(curr_widget, "Please enter host");
		return;
	}
	else
	{
		account->server_host = server_host;
	}
	//port
	curr_widget = gtk_grid_get_child_at(GTK_GRID(grid), 2, 8);
	const gchar * server_port = gtk_entry_get_text(GTK_ENTRY(curr_widget));
	if(strlen(server_port) == 0 || strcmp("Enter Server Port",server_port) ==0) {
		show_warn_window(curr_widget, "Please enter port");
		return;
	} else {
		int port = atoi(server_port);
		if(port < 1 || port > G_MAXUINT16) {
			show_warn_window(curr_widget, "Port must be > 0 and < 65535");
			return;
		}
		else
			account->server_port = port;
	}
	//clean session
	curr_widget = gtk_grid_get_child_at(GTK_GRID(grid), 2, 12);
	account->clean_session = gtk_switch_get_state(GTK_SWITCH (curr_widget));
	//keepalive
	curr_widget = gtk_grid_get_child_at(GTK_GRID(grid), 2, 13);
	const gchar * keepalive_string = gtk_entry_get_text(GTK_ENTRY(curr_widget));
	if((strlen(keepalive_string) == 0 || strcmp("Enter Keepalive",keepalive_string) ==0) && (current_protocol == MQTT || current_protocol == MQTT_SN || current_protocol == WEBSOCKETS)) {
		show_warn_window(curr_widget, "Please enter keepalive");
		return;
	} else {
		int keepalive = atoi(keepalive_string);
		if(keepalive < 0 || keepalive > G_MAXUINT16) {
			show_warn_window(curr_widget, "keepalive must be >= 0 and < 65535");
			return;
		}
		else
			account->keep_alive = keepalive;
	}
	//will
	curr_widget = gtk_grid_get_child_at(GTK_GRID(grid), 2, 14);
	const gchar * will = gtk_entry_get_text(GTK_ENTRY(curr_widget));
	if((strlen(will) == 0 || strcmp("Enter Will",will) ==0)) {
		account->will = NULL;
	}
	else {
		account->will = will;
	}
	//will topic
	curr_widget = gtk_grid_get_child_at(GTK_GRID(grid), 2, 15);
	const gchar * will_topic = gtk_entry_get_text(GTK_ENTRY(curr_widget));
	if((strlen(will_topic) == 0 || strcmp("Enter Will topic",will_topic) ==0)) {
		account->will_topic = NULL;
	}
	else {
		account->will_topic = will_topic;
	}

	if((account->will == NULL && account->will_topic != NULL) || (account->will != NULL && account->will_topic == NULL)) {
		show_warn_window(curr_widget, "Will and will topic both MUST be present or absent");
		return;
	}
	//Retain
	curr_widget = gtk_grid_get_child_at(GTK_GRID(grid), 2, 16);
	account->is_retain = gtk_switch_get_state(GTK_SWITCH (curr_widget));
	//QoS
	curr_widget = gtk_grid_get_child_at(GTK_GRID(grid), 2, 17);
	account->qos = gtk_spin_button_get_value(GTK_SPIN_BUTTON (curr_widget));
	account->is_default = 1;
	//secure enabled
	curr_widget = gtk_grid_get_child_at(GTK_GRID(grid), 2, 20);
	account->is_secure = gtk_switch_get_state(GTK_SWITCH (curr_widget));
	//certificate
	curr_widget = gtk_grid_get_child_at(GTK_GRID(grid), 2, 21);
	const gchar * cert = gtk_entry_get_text(GTK_ENTRY(curr_widget));
	if((strlen(cert) == 0 || strcmp("Certificate",cert) ==0))
		account->certificate = NULL;
	else
		account->certificate = cert;
	//password
	curr_widget = gtk_grid_get_child_at(GTK_GRID(grid), 2, 22);
	const gchar * cert_pass = gtk_entry_get_text(GTK_ENTRY(curr_widget));
	if((strlen(cert_pass) == 0 || strcmp("Password",cert_pass) ==0))
		account->certificate_password = NULL;
	else
		account->certificate_password = cert_pass;

	mqtt_listener = malloc (sizeof (struct MqttListener));
	mqtt_listener->cs_pt = connection_success;


	switch (current_protocol) {
		case MQTT : {
			if (init_mqtt_client(account, mqtt_listener) != 0) {
				//TODO WARNING WINDOW CONNECTION FAILED
				printf("MQTT client : connection failed!!!\n");
				return;
			}
			break;
		}
		case MQTT_SN : {
			if (init_mqtt_sn_client(account, mqtt_listener) != 0) {
				//TODO WARNING WINDOW CONNECTION FAILED
				printf("MQTT-SN client : connection failed!!!\n");
				return;
			}
			break;
		}
		case COAP: {
			if (init_coap_client(account, mqtt_listener) != 0) {
				printf("COAP client : connection failed!!!\n");
			    return;
			}

			break;
		}
		case AMQP : {
			if (init_amqp_client(account, mqtt_listener) != 0) {
				printf("AMQP client : connection failed!!!\n");
				return;
			}

			break;
		}
		case WEBSOCKETS : {
			if (init_mqtt_client(account, mqtt_listener) != 0) {
				//TODO WARNING WINDOW CONNECTION FAILED
				printf("MQTT-WS client : connection failed!!!\n");
				return;
			}
			break;
		}
		default : {
			printf("Unsupported protocol : %i \n", current_protocol);
			exit(1);
		}
	}
	mqtt_listener->send_connect(account);
}

static void choose_box(GtkWidget *widget, GtkWidget *box) {
	gint chooser_index = gtk_combo_box_get_active(GTK_COMBO_BOX(box));
	switch (chooser_index) {
	case 0:
		activate_login_window(NULL);
		current_protocol = MQTT;
		break;

	case 1:
		print_mqtt_sn_box();
		current_protocol = MQTT_SN;
		break;

	case 2:
		print_coap_box();
		current_protocol = COAP;
		break;

	case 3:
		print_amqp_box();
		current_protocol = AMQP;
		break;
	case 4:
		activate_login_window(NULL);
		current_protocol = WEBSOCKETS;
		break;

	}
}

void activate_login_window(GtkApplication* application) {

	if (application != NULL) {
		app = application;
		login_window = gtk_application_window_new (app);
		gtk_widget_set_name (login_window, "login_window");
		gtk_window_set_title (GTK_WINDOW (login_window), "IOT Broker C client");
		gtk_window_set_resizable (GTK_WINDOW (login_window), FALSE);
		gtk_window_set_default_size (GTK_WINDOW (login_window), 280, 400);
		g_signal_connect(G_OBJECT(login_window), "delete_event", G_CALLBACK(quit_to_list_accounts), mqtt_listener);

		GtkWidget *box, *grid, *separator, *label, *chooser, *entry, *switcher, *spin, *button;

		int i = 0;
		box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
		grid = gtk_grid_new();
		//gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
		gtk_grid_set_row_spacing(GTK_GRID(grid), 2);
		gtk_widget_set_name (grid, "white_grid");

		gtk_container_add(GTK_CONTAINER(box), grid);

		label = gtk_label_new("registration info:");
		gtk_widget_set_name (label, "login_label");
		gtk_widget_set_halign (label, GTK_ALIGN_FILL);
		gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 3, 1);

		separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
		gtk_grid_attach(GTK_GRID(grid), separator, 0, 2, 3, 1);

		add_image(label, grid, 0, 3, "settings");

		label = gtk_label_new("Protocol");
		gtk_widget_set_halign (label, GTK_ALIGN_START);
		gtk_grid_attach(GTK_GRID(grid), label, 1, 3, 1, 1);

		chooser = gtk_combo_box_text_new();

		for (int i = 0; i < G_N_ELEMENTS (PROTOCOLS_STRING); i++)
			gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(chooser), PROTOCOLS_STRING[i]);

		gtk_combo_box_set_active(GTK_COMBO_BOX(chooser), 0);

		gtk_grid_attach(GTK_GRID(grid), chooser, 2, 3, 1, 1);

		g_signal_connect(GTK_COMBO_BOX (chooser), "changed",
				G_CALLBACK (choose_box), GTK_COMBO_BOX (chooser));

		mqttWidgets[i++] = add_image(label, grid, 0, 4, "username");

		label = gtk_label_new("Username");
		gtk_widget_set_halign (label, GTK_ALIGN_START);
		gtk_grid_attach(GTK_GRID(grid), label, 1, 4, 1, 1);
		mqttWidgets[i++] = label;

		entry = gtk_entry_new();
		gtk_entry_set_max_length(GTK_ENTRY(entry), 50);
		gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Enter User Name");
		gtk_grid_attach(GTK_GRID(grid), entry, 2, 4, 1, 1);
		mqttWidgets[i++] = entry;

		mqttWidgets[i++] = add_image(label, grid, 0, 5, "password");

		label = gtk_label_new("Password");
		gtk_widget_set_halign (label, GTK_ALIGN_START);
		gtk_grid_attach(GTK_GRID(grid), label, 1, 5, 1, 1);
		mqttWidgets[i++] = label;

		entry = gtk_entry_new();
		gtk_entry_set_visibility(GTK_ENTRY(entry),FALSE);
		gtk_entry_set_max_length(GTK_ENTRY(entry), 50);
		gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Enter Password");
		gtk_grid_attach(GTK_GRID(grid), entry, 2, 5, 1, 1);
		mqttWidgets[i++] = entry;

		mqttWidgets[i++] = add_image(label, grid, 0, 6, "client_id");

		label = gtk_label_new("Client ID");
		gtk_widget_set_halign (label, GTK_ALIGN_START);
		gtk_grid_attach(GTK_GRID(grid), label, 1, 6, 1, 1);
		mqttWidgets[i++] = label;

		entry = gtk_entry_new();
		gtk_entry_set_max_length(GTK_ENTRY(entry), 50);
		gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Enter Client ID");
		gtk_grid_attach(GTK_GRID(grid), entry, 2, 6, 1, 1);
		mqttWidgets[i++] = entry;

		add_image(label, grid, 0, 7, "host");

		label = gtk_label_new("Server host      ");
		gtk_widget_set_halign (label, GTK_ALIGN_START);
		gtk_grid_attach(GTK_GRID(grid), label, 1, 7, 1, 1);

		entry = gtk_entry_new();
		gtk_entry_set_max_length(GTK_ENTRY(entry), 50);
		gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Enter Server Host");
		gtk_grid_attach(GTK_GRID(grid), entry, 2, 7, 1, 1);

		add_image(label, grid, 0, 8, "host");

		label = gtk_label_new("Port");
		gtk_widget_set_halign (label, GTK_ALIGN_START);
		gtk_grid_attach(GTK_GRID(grid), label, 1, 8, 1, 1);

		entry = gtk_entry_new();
		gtk_entry_set_max_length(GTK_ENTRY(entry), 50);
		gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Enter Server Port");
		gtk_grid_attach(GTK_GRID(grid), entry, 2, 8, 1, 1);

		label = gtk_label_new("settings");
		gtk_widget_set_name (label, "login_label");
		gtk_widget_set_halign (label, GTK_ALIGN_FILL);
		gtk_grid_attach(GTK_GRID(grid), label, 0, 10, 3, 1);

		separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
		gtk_grid_attach(GTK_GRID(grid), separator, 0, 11, 3, 1);

		mqttWidgets[i++] = add_image(label, grid, 0, 12, "clean_session");

		label = gtk_label_new("Clean session");
		gtk_widget_set_halign (label, GTK_ALIGN_START);
		gtk_grid_attach(GTK_GRID(grid), label, 1, 12, 1, 1);
		mqttWidgets[i++] = label;

		switcher = gtk_switch_new();
		gtk_grid_attach(GTK_GRID(grid), switcher, 2, 12, 1, 1);
		mqttWidgets[i++] = switcher;

		mqttWidgets[i++] = add_image(label, grid, 0, 13, "keepalive");

		label = gtk_label_new("Keepalive");
		gtk_widget_set_halign (label, GTK_ALIGN_START);
		gtk_grid_attach(GTK_GRID(grid), label, 1, 13, 1, 1);
		mqttWidgets[i++] = label;

		entry = gtk_entry_new();
		gtk_entry_set_max_length(GTK_ENTRY(entry), 50);
		gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Enter Keepalive");
		gtk_grid_attach(GTK_GRID(grid), entry, 2, 13, 1, 1);
		mqttWidgets[i++] = entry;

		mqttWidgets[i++] = add_image(label, grid, 0, 14, "settings");

		label = gtk_label_new("Will");
		gtk_widget_set_halign (label, GTK_ALIGN_START);
		gtk_grid_attach(GTK_GRID(grid), label, 1, 14, 1, 1);
		mqttWidgets[i++] = label;

		entry = gtk_entry_new();
		g_signal_connect(entry, "button-press-event", G_CALLBACK (show_dialog_window), "will");
		gtk_entry_set_max_length(GTK_ENTRY(entry), 50);
		gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Enter Will");
		gtk_grid_attach(GTK_GRID(grid), entry, 2, 14, 1, 1);
		mqttWidgets[i++] = entry;

		mqttWidgets[i++] = add_image(label, grid, 0, 15, "settings");

		label = gtk_label_new("Will topic");
		gtk_widget_set_halign (label, GTK_ALIGN_START);
		gtk_grid_attach(GTK_GRID(grid), label, 1, 15, 1, 1);
		mqttWidgets[i++] = label;

		entry = gtk_entry_new();
		gtk_entry_set_max_length(GTK_ENTRY(entry), 50);
		gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Enter Will topic");
		gtk_grid_attach(GTK_GRID(grid), entry, 2, 15, 1, 1);
		mqttWidgets[i++] = entry;

		mqttWidgets[i++] = add_image(label, grid, 0, 16, "settings");

		label = gtk_label_new("Retain");
		gtk_widget_set_halign (label, GTK_ALIGN_START);
		gtk_grid_attach(GTK_GRID(grid), label, 1, 16, 1, 1);
		mqttWidgets[i++] = label;

		switcher = gtk_switch_new();
		gtk_grid_attach(GTK_GRID(grid), switcher, 2, 16, 1, 1);
		mqttWidgets[i++] = switcher;

		mqttWidgets[i++] = add_image(label, grid, 0, 17, "settings");

		label = gtk_label_new("QoS");
		gtk_widget_set_halign (label, GTK_ALIGN_START);
		gtk_grid_attach(GTK_GRID(grid), label, 1, 17, 1, 1);
		mqttWidgets[i++] = label;

		spin = gtk_spin_button_new_with_range(0, 2, 1);
		gtk_grid_attach(GTK_GRID(grid), spin, 2, 17, 1, 1);
		mqttWidgets[i++] = spin;

		//TLS
		label = gtk_label_new("security");
		gtk_widget_set_name (label, "login_label");
		gtk_widget_set_halign (label, GTK_ALIGN_FILL);
		gtk_grid_attach(GTK_GRID(grid), label, 0, 18, 3, 1);

		separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
		gtk_grid_attach(GTK_GRID(grid), separator, 0, 19, 3, 1);

		mqttWidgets[i++] = add_image(label, grid, 0, 20, "settings");

		label = gtk_label_new("Enabled");
		gtk_widget_set_halign (label, GTK_ALIGN_START);
		gtk_grid_attach(GTK_GRID(grid), label, 1, 20, 1, 1);
		mqttWidgets[i++] = label;

		switcher = gtk_switch_new();
		gtk_grid_attach(GTK_GRID(grid), switcher, 2, 20, 1, 1);
		mqttWidgets[i++] = switcher;

		mqttWidgets[i++] = add_image(label, grid, 0, 21, "settings");

		label = gtk_label_new("Certificate");
		gtk_widget_set_halign (label, GTK_ALIGN_START);
		gtk_grid_attach(GTK_GRID(grid), label, 1, 21, 1, 1);
		mqttWidgets[i++] = label;

		entry = gtk_entry_new();
		g_signal_connect(entry, "button-press-event", G_CALLBACK (show_dialog_window), "cert");
		gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Certificate");
		gtk_grid_attach(GTK_GRID(grid), entry, 2, 21, 1, 1);
		mqttWidgets[i++] = entry;

		mqttWidgets[i++] = add_image(label, grid, 0, 22, "settings");

		label = gtk_label_new("Password");
		gtk_widget_set_halign (label, GTK_ALIGN_START);
		gtk_grid_attach(GTK_GRID(grid), label, 1, 22, 1, 1);
		mqttWidgets[i++] = label;

		entry = gtk_entry_new();
		gtk_entry_set_max_length(GTK_ENTRY(entry), 50);
		gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Password");
		gtk_grid_attach(GTK_GRID(grid), entry, 2, 22, 1, 1);
		mqttWidgets[i++] = entry;

		button = gtk_button_new_with_label("\nLog In\n");
		g_signal_connect(G_OBJECT(button), "enter-notify-event", G_CALLBACK(above_button), NULL);
		g_signal_connect(G_OBJECT(button), "leave-notify-event", G_CALLBACK(out_of_button), NULL);
		gtk_widget_set_name (button, "log_in");
		g_signal_connect(button, "clicked", G_CALLBACK (login_button_handle), grid);

		gtk_box_pack_end(GTK_BOX(box), GTK_WIDGET(button), FALSE, TRUE, 1);

		gtk_container_add(GTK_CONTAINER(login_window), box);
		gtk_widget_show_all(login_window);
	} else {
		for (int i = 0; i < G_N_ELEMENTS(mqttWidgets); i++) {
			gtk_widget_show(mqttWidgets[i]);
		}
	}
}

static void connection_success() {
		struct MqttModel mqtt_model;
		mqtt_model.account = account;
		mqtt_model.save_acc_pt = save_changes;
		mqtt_model.save_acc_pt(account);
		//and open common window
		gtk_widget_hide(login_window);
		activate_main_window(app, current_protocol, mqtt_listener, account);
}
