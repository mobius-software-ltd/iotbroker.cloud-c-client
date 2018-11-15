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
#include <unistd.h>
#include <string.h>
#include "common.h"
#include "login.h"
#include "../dal/cdal.h"
#include "../account.h"
#include "../mqtt/mqtt_client.h"
#include "../mqttsn/mqtt_sn_client.h"
#include "../coap/coap_client.h"
#include "../amqp/amqp_client.h"
#include "../mqtt_listener.h"
#include "main_w.h"
#include "gui_color.h"


GtkWidget * account_list_window;
GtkWidget * loading_window;
GtkApplication * app;

struct MqttModel * retrieve_accounts();
static void show_account_list_window(struct MqttModel * model);
static void add_new_account_handler(GtkButton *button, gpointer user_data);
static void connection_success();

struct MqttListener * mqtt_listener;
enum Protocol current_protocol;
struct Account * account;

static void show_app_window(){

	struct MqttModel * model = retrieve_accounts();
	if(model == NULL) {
		//create new one in new account window
		gtk_widget_destroy(loading_window);
		activate_login_window(app);
	} else {
		gtk_widget_destroy(loading_window);
		//create new window with list of accounts and close old one
		show_account_list_window(model);
	}
}

static gboolean fill (gpointer user_data)
{

  GtkWidget *progress_bar = user_data;

  /*Get the current progress*/
  gdouble fraction;
  fraction = gtk_progress_bar_get_fraction (GTK_PROGRESS_BAR (progress_bar));

  /*Increase the bar by 10% each time this function is called*/
  fraction += 0.1;

  /*Fill in the bar with the new fraction*/
  gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress_bar), fraction);

  /*Ensures that the fraction stays below 1.0*/
  if (fraction < 1.0)
    return TRUE;
  show_app_window();
  return FALSE;
}


static void remove_account_button_handle (GtkWidget *widget, gpointer data) {

	const char * str = gtk_widget_get_name(GTK_WIDGET(data));
	remove_account_from_db(atoi(str));
	gtk_widget_destroy(data);

}



void activate_main_window_default (GtkButton * button, struct Account * _account) {

	account = _account;
	set_account_default(account->id);
	current_protocol = account->protocol;

	mqtt_listener = malloc (sizeof (struct MqttListener));
	mqtt_listener->cs_pt = connection_success;

	if(current_protocol == MQTT) {
		if (init_mqtt_client(account, mqtt_listener) != 0) {
			printf("MQTT client: CONNECTION FAILED!!!\n");
			//TODO WARNING WINDOW CONNECTION FAILED
			return;
		}
	} else if (current_protocol == MQTT_SN){
		if (init_mqtt_sn_client(account, mqtt_listener) != 0) {
			printf("MQTT-SN client: CONNECTION FAILED!!!\n");
			//TODO WARNING WINDOW CONNECTION FAILED
			return;
		}

	} else if(current_protocol == COAP) {
		if (init_coap_client(account, mqtt_listener) != 0) {
			printf("COAP client: CONNECTION FAILED!!!\n");
			//TODO WARNING WINDOW CONNECTION FAILED
			return;
		}
	} else if(current_protocol == AMQP) {
		if (init_amqp_client(account, mqtt_listener) != 0) {
			printf("AMQP client: CONNECTION FAILED!!!\n");
			//TODO WARNING WINDOW CONNECTION FAILED
			return;
		}
	} else if(current_protocol == WEBSOCKETS) {
		if (init_mqtt_client(account, mqtt_listener) != 0) {
			printf("MQTT-WS client: CONNECTION FAILED!!!\n");
			//TODO WARNING WINDOW CONNECTION FAILED
			return;
		}
	} else {
		printf("Error: unsupported protocol : %i \n", current_protocol);
		exit(1);
	}
	mqtt_listener->send_connect(account);
}

void activate_loading_window (GtkApplication* _app, gpointer user_data) {

	app = _app;

	GtkWidget * image;
	GtkWidget * box;
	GtkWidget * progress_bar;
	double fraction = 0.0;

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);

	loading_window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (loading_window), "IOT Broker C client");
	//gtk_window_set_default_size (GTK_WINDOW (loading_window), 400, 550);
	gtk_window_set_default_size (GTK_WINDOW (loading_window), 330, 450);
	g_signal_connect(G_OBJECT(loading_window), "delete_event", G_CALLBACK(quit), mqtt_listener);
	progress_bar = gtk_progress_bar_new ();
	gtk_box_pack_end (GTK_BOX(box), progress_bar, TRUE, TRUE, 1);
	image = gtk_image_new_from_file ("./images/ic_loading_text.png");
	gtk_box_pack_end (GTK_BOX(box), image, TRUE, TRUE, 1);
	image = gtk_image_new_from_file ("./images/logo.png");
	gtk_box_pack_end (GTK_BOX(box), image, TRUE, TRUE, 1);
	gtk_container_add (GTK_CONTAINER (loading_window), box);
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress_bar), fraction);
	g_timeout_add (200, fill, GTK_PROGRESS_BAR (progress_bar));
	gtk_widget_show_all (loading_window);

}

struct MqttModel * retrieve_accounts() {
	struct MqttModel mqttModel;
	mqttModel.get_accounts_pt = get_accounts;
	return mqttModel.get_accounts_pt();
}

static void show_account_list_window(struct MqttModel * model) {

	GtkWidget *grid;
	GtkWidget * scrolled_window;
	account_list_window = gtk_application_window_new (app);

	gtk_window_set_title (GTK_WINDOW (account_list_window), "Accounts' list");
	gtk_window_set_resizable (GTK_WINDOW (account_list_window), FALSE);
	gtk_widget_set_size_request (account_list_window, 400, 550);

	scrolled_window = gtk_scrolled_window_new (NULL,NULL);
	gtk_container_set_border_width (GTK_CONTAINER (scrolled_window), 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

	GtkWidget * box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	//add label

	GtkWidget * label = gtk_label_new("\nPlease select account\n");
	gtk_widget_set_name (label, "label_select_account");
	gtk_box_pack_start (GTK_BOX(box), label, FALSE, FALSE, 0);

	GtkWidget * button = NULL;
	char * padding = " client account details                            \n";
	char * sn_padding = " client account details                       \n";
	char * ws_padding = " client account details            \n";

	for(int i = 0; i < model -> account_size; i++) {
		char str [256] = {};
		enum Protocol protocol = model-> account -> protocol;
		char * protocol_string = NULL;
		switch (protocol) {
		case MQTT: protocol_string    = "MQTT";
		break;
		case MQTT_SN: protocol_string = "MQTT-SN";
		break;
		case COAP: protocol_string = "COAP";
		break;
		case AMQP: protocol_string = "AMQP";
		break;
		case WEBSOCKETS: protocol_string = "WEBSOCKETS";
		break;
		}
		strcat(str, protocol_string);
		if(protocol == MQTT_SN) {
			strcat(str, sn_padding);
		} else if (protocol == WEBSOCKETS) {
			strcat(str, ws_padding);
		} else {
			strcat(str, padding);
		}

		strcat(str, model-> account -> client_id);
		strcat(str, "\n");

		strcat(str, model-> account -> server_host);
		strcat(str, ":");
		char port_string [256];
		sprintf(port_string, "%d", model -> account -> server_port);
		strcat(str, port_string);
		button = gtk_button_new_with_label(str);
		//gtk_widget_set_halign (button, GTK_ALIGN_START);
		gtk_widget_set_hexpand(button, TRUE);
		gtk_widget_set_name (button, "account");
		g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(activate_main_window_default), model -> account);

		grid = gtk_grid_new ();

		char account_id_string [5];
		sprintf(account_id_string, "%d", model-> account ->id);

		gtk_widget_set_name (grid, account_id_string);

		gtk_box_pack_start (GTK_BOX(box), GTK_WIDGET(grid), FALSE, TRUE, 1);
		gtk_grid_attach (GTK_GRID (grid), button, 0, 0, 1, 1);
		button = gtk_button_new_with_label("       ");
		gtk_widget_set_hexpand (button, FALSE);
		gtk_widget_set_name (button, "remove_account");
		g_signal_connect(button, "clicked", G_CALLBACK (remove_account_button_handle), grid);
		gtk_grid_attach (GTK_GRID (grid), button, 1, 0, 1, 1);

		model->account++;
	}

	gtk_container_add(GTK_CONTAINER (scrolled_window), box);

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	gtk_box_pack_start (GTK_BOX(box), scrolled_window, TRUE, TRUE, 1);
	button = gtk_button_new_with_label("\nAdd new account\n");
	gtk_widget_set_name (button, "add_account");

	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(add_new_account_handler), app);

	gtk_box_pack_start (GTK_BOX(box), button, FALSE, FALSE, 1);
	gtk_container_add(GTK_CONTAINER (account_list_window), box);
	gtk_widget_show_all (account_list_window);
}

static void add_new_account_handler(GtkButton *button, gpointer user_data) {
	gtk_widget_hide(account_list_window);
	activate_login_window(user_data);
}

static void connection_success() {
	gtk_widget_hide(account_list_window);
	activate_main_window(app, current_protocol, mqtt_listener);
}

void reload_account_list_window() {
	gtk_widget_show_all(account_list_window);
}
