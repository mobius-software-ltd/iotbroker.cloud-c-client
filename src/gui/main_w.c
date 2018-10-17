/*
 * main.c
 *
 *  Created on: Jul 24, 2018
 *      Author: konstantinnosach
 */

#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "loading.h"
#include "../account.h"
#include "../common_topic.h"
#include "../common_publish.h"
#include "../mqtt_listener.h"
#include "../dal/cdal.h"
#include "../mqtt/mqtt_client.h"
#include "../mqttsn/mqtt_sn_client.h"

void add_topic_button_handle();
void send_message_button_handle();
void get_message(const char * content, const char * topic_name, int qos, int retain, int dup);

static enum Protocol current_protocol;
static GtkWidget *main_window;
struct MqttListener * mqtt_listener;
struct Topic * current_topic = NULL;
struct MqttModel * mqtt_model;

GtkWidget *topics_box;
GtkWidget *messages_box;

static void add_settings_image(GtkWidget * label, GtkWidget * grid, gint x, gint y) {

	label = gtk_label_new ("           ");
	gtk_widget_set_hexpand (label, FALSE);
	gtk_widget_set_name (label, "settings_send");
	gtk_grid_attach (GTK_GRID (grid), label, x, y, 1, 1);
}

static GtkWidget* find_child(GtkWidget* parent, const gchar* name)
{

	if(parent == NULL)
		return NULL;

	if (strcmp(gtk_widget_get_name((GtkWidget*)parent), (gchar*)name) == 0) {
		return parent;
	}

	if (GTK_IS_BIN(parent)) {
			GtkWidget *child = gtk_bin_get_child(GTK_BIN(parent));
			return find_child(child, name);
	}

	if (GTK_IS_CONTAINER(parent)) {
			GList *children = gtk_container_get_children(GTK_CONTAINER(parent));
			while ((children = g_list_next(children)) != NULL) {
					GtkWidget* widget = find_child(children->data, name);
					if (widget != NULL) {
							return widget;
					}
			}
	}

	return NULL;
}

static void log_out_handle () {
	mqtt_listener->send_disconnect();
	reload_account_list_window();
	gtk_window_close (GTK_WINDOW(main_window));
}


static void usubscribe_button_handle (GtkWidget *widget, gpointer data){

	GtkWidget * curr_widget = gtk_grid_get_child_at(GTK_GRID(data), 0, 0);
	const char * topic_name = gtk_label_get_text(GTK_LABEL(curr_widget));
	curr_widget = gtk_grid_get_child_at(GTK_GRID(data), 2, 0);
	mqtt_listener->send_unsubscribe(topic_name);
}

void add_topics_to_list_box (const char * topic_name, int qos) {

	GtkWidget * button;
	GtkWidget * grid;
	GtkWidget * label;
	GtkWidget * separator;

	char qos_string [2] = {};
	sprintf(qos_string, "%d", qos);
	char str[7];
	strcpy(str, "QoS:");

	separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	grid = gtk_grid_new ();
	gtk_widget_set_name(grid, topic_name);
	gtk_widget_set_name(separator, topic_name);
	label = gtk_label_new (topic_name);
	gtk_widget_set_hexpand (label, TRUE);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (grid), label, 0, 0, 1, 1);

	label = gtk_label_new (strcat(str, qos_string));
	gtk_widget_set_name (label, "qos_label");

	gtk_widget_set_hexpand (label, FALSE);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (grid), label, 1, 0, 1, 1);

	button = gtk_button_new_with_label ("      ");
	gtk_widget_set_name (button, "delete");
	g_signal_connect(button, "clicked", G_CALLBACK (usubscribe_button_handle), grid);
	gtk_widget_set_hexpand (button, FALSE);
	gtk_grid_attach (GTK_GRID (grid), button, 2, 0, 1, 1);

	gtk_container_add(GTK_CONTAINER(topics_box), grid);
	gtk_container_add(GTK_CONTAINER(topics_box), separator);
	gtk_widget_show_all(topics_box);

}

static void warning_window_handler (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	gtk_widget_set_sensitive(GTK_WIDGET (data), TRUE);
}

static void show_warn_window(GtkWidget * curr_widget,const gchar * warning_string) {
	GtkWidget *warning_window;
	warning_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (warning_window), "Add topic");
	gtk_window_set_resizable (GTK_WINDOW (warning_window), FALSE);
	gtk_widget_set_size_request (warning_window, 280, 280);
	gtk_container_add(GTK_CONTAINER(warning_window), gtk_label_new(warning_string));
	GtkWidget * parent = gtk_widget_get_parent(curr_widget);
	parent = gtk_widget_get_parent(parent);
	parent = gtk_widget_get_parent(parent);
	parent = gtk_widget_get_parent(parent);
	gtk_widget_set_sensitive(GTK_WIDGET(parent), FALSE);
	g_signal_connect(G_OBJECT(warning_window), "delete_event", G_CALLBACK(warning_window_handler), parent);
	gtk_widget_show_all(warning_window);
}


void update_messages_window (const char * content, const char * topic_name, int qos, int retain, int dup, int is_incoming) {

	GtkWidget * grid = NULL;
	GtkWidget * label = NULL;
	char row1 [256] = {};
	char row2 [256] = {};
	if(is_incoming){
		strcat(row2, "     in\n");
	} else {
		strcat(row2, "    out\n");
	}
		strcat(row1, topic_name);
		strcat(row1, "\n");
		strcat(row1,content);

		strcat(row2, "  QOS : ");
		char * qos_string = malloc(sizeof (char)*2);
		sprintf(qos_string, "%d", qos);
		strcat(row2, qos_string);
		strcat(row2, "  ");

	grid = gtk_grid_new ();
	label = gtk_label_new (row1);
	gtk_widget_set_hexpand (label, TRUE);
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (grid), label, 0, 0, 1, 1);

	label = gtk_label_new (row2);
	gtk_widget_set_name (label, "messages_qos_label");

	gtk_widget_set_hexpand (label, FALSE);
	gtk_widget_set_halign (label, GTK_ALIGN_END);
	gtk_grid_attach (GTK_GRID (grid), label, 1, 0, 1, 1);


	gtk_box_pack_start (GTK_BOX(messages_box), GTK_WIDGET(grid), FALSE, TRUE, 1);

	gtk_widget_show_all(messages_box);

}

void activate_main_window(GtkApplication* app, enum Protocol protocol, struct MqttListener * listener) {

	mqtt_model = malloc (sizeof (struct MqttModel));
	mqtt_model->save_message_pt = save_message;
	mqtt_listener->get_pub_pt = get_message;

	  current_protocol = protocol;
	  mqtt_listener = listener;

	  GtkWidget *scrolled_window;

	  GtkWidget *grid;
	  GtkWidget *entry;
	  GtkWidget *stack;
	  GtkWidget *stack_switcher;
	  GtkWidget *box;
	  GtkWidget *button;
	  GtkWidget *label;
	  GtkWidget *frame;
	  GtkWidget *separator;

	  scrolled_window = gtk_scrolled_window_new (NULL,NULL);
	  gtk_container_set_border_width (GTK_CONTAINER (scrolled_window), 0);
	  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);


	  main_window = gtk_application_window_new (app);
	  gtk_window_set_title (GTK_WINDOW (main_window), "IOT Broker C client");
	  gtk_window_set_default_size (GTK_WINDOW (main_window), 280, 500);
	  gtk_window_set_resizable (GTK_WINDOW (main_window), FALSE);
	  //g_signal_connect(G_OBJECT(main_window), "delete_event", G_CALLBACK (log_out_handle), NULL);

	  stack_switcher = gtk_stack_switcher_new ();
//	  gtk_widget_set_vexpand (stack_switcher, TRUE);
//	  gtk_widget_set_hexpand (stack_switcher, TRUE);
	  //g_object_set(G_OBJECT(stack_switcher), "icon-size", GTK_ICON_SIZE_DIALOG, NULL);
	  gtk_widget_set_halign (GTK_WIDGET(stack_switcher), GTK_ALIGN_CENTER);

	  stack = gtk_stack_new ();
	  gtk_stack_set_transition_type(GTK_STACK(stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
	  gtk_stack_switcher_set_stack(GTK_STACK_SWITCHER(stack_switcher), GTK_STACK(stack));

	  //topics box
	  box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
	  //add label topics list label
	  label = gtk_label_new ("topics list:");
	  gtk_widget_set_halign (label, GTK_ALIGN_START);
	  gtk_box_pack_start (GTK_BOX(box), GTK_WIDGET(label), FALSE, FALSE, 0);

	  separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	  topics_box = gtk_list_box_new();
	  gtk_container_add(GTK_CONTAINER(topics_box), separator);
	  gtk_widget_set_name (topics_box, "white_box");
	  gtk_container_add(GTK_CONTAINER (scrolled_window), topics_box);
	  gtk_box_pack_start (GTK_BOX(box), GTK_WIDGET(scrolled_window), TRUE, TRUE, 0);
	  //add add new topics label
	  label = gtk_label_new ("add new topic:");
	  gtk_widget_set_halign (label, GTK_ALIGN_START);
	  gtk_box_pack_start (GTK_BOX(box), GTK_WIDGET(label), FALSE, FALSE, 0);

	  gtk_stack_add_titled (GTK_STACK(stack), box, "Topic List", "\nTopic List\n");

	  grid = gtk_grid_new ();
	  gtk_widget_set_name (grid, "white_grid");
	  //gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
	  gtk_box_pack_start (GTK_BOX(box), GTK_WIDGET(grid), FALSE, FALSE, 0);

	  label = gtk_label_new ("           ");
	  gtk_widget_set_hexpand (label, FALSE);
	  gtk_widget_set_name (label, "settings");
	  gtk_grid_attach (GTK_GRID (grid), label, 0, 0, 1, 1);

	  label = gtk_label_new ("Topic");
	  gtk_widget_set_hexpand (label, TRUE);
	  gtk_widget_set_halign (label, GTK_ALIGN_START);
	  gtk_grid_attach (GTK_GRID (grid), label, 1, 0, 1, 1);
	  entry = gtk_entry_new();
	  gtk_entry_set_placeholder_text(GTK_ENTRY (entry), "Enter topic");
	  gtk_grid_attach (GTK_GRID (grid), entry, 2, 0, 1, 1);

	  label = gtk_label_new ("");
	  gtk_widget_set_name (label, "settings");
	  gtk_grid_attach (GTK_GRID (grid), label, 0, 1, 1, 1);

	  label = gtk_label_new ("QoS");
	  gtk_widget_set_halign (label, GTK_ALIGN_START);
	  gtk_grid_attach (GTK_GRID (grid), label, 1, 1, 1, 1);
	  gtk_grid_attach (GTK_GRID (grid), gtk_spin_button_new_with_range (0, 2, 1), 2, 1, 1, 1);


	  button = gtk_button_new_with_label ("\nAdd\n");
	  gtk_widget_set_name (button, "add_topic");
	  g_signal_connect (button, "clicked", G_CALLBACK (add_topic_button_handle), grid);
	  gtk_box_pack_end (GTK_BOX(box), GTK_WIDGET(button), FALSE, FALSE, 0);

	  //send message box
	  box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

	  //gtk_widget_set_name (box, "white_box");
	  //add label topics list label
	  label = gtk_label_new ("send new message:");
	  gtk_widget_set_halign (label, GTK_ALIGN_START);
	  gtk_box_pack_start (GTK_BOX(box), GTK_WIDGET(label), FALSE, FALSE, 0);

	  grid = gtk_grid_new ();
	  gtk_grid_set_row_spacing(GTK_GRID(grid), 4);
	  gtk_widget_set_name (grid, "white_grid");
	  //gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
	  gtk_box_pack_start (GTK_BOX(box), GTK_WIDGET(grid), TRUE, TRUE, 1);
	  gtk_widget_set_hexpand (grid, TRUE);
	  gtk_stack_add_titled (GTK_STACK(stack), box, "Send Messa", "Send Messa");

	  add_settings_image(label, grid, 0, 0);

	  label = gtk_label_new ("Content");
	  gtk_widget_set_hexpand (label, TRUE);
	  gtk_widget_set_halign (label, GTK_ALIGN_START);
	  gtk_grid_attach (GTK_GRID (grid), label, 1, 0, 1, 1);
	  entry = gtk_entry_new();
	  gtk_entry_set_placeholder_text(GTK_ENTRY (entry), "Enter content");
	  gtk_grid_attach (GTK_GRID (grid), entry, 2, 0, 1, 1);

	  add_settings_image(label, grid, 0, 1);

	  label = gtk_label_new ("Topic");
	  gtk_widget_set_hexpand (label, TRUE);
	  gtk_widget_set_halign (label, GTK_ALIGN_START);
	  gtk_grid_attach (GTK_GRID (grid), label, 1, 1, 1, 1);
	  entry = gtk_entry_new();
	  gtk_entry_set_placeholder_text(GTK_ENTRY (entry), "Enter Topic");
	  gtk_grid_attach (GTK_GRID (grid), entry, 2, 1, 1, 1);

	  add_settings_image(label, grid, 0, 2);
	  label = gtk_label_new ("QoS");
	  gtk_widget_set_hexpand (label, TRUE);
	  gtk_widget_set_halign (label, GTK_ALIGN_START);
	  gtk_grid_attach (GTK_GRID (grid), label, 1, 2, 1, 1);
	  gtk_grid_attach (GTK_GRID (grid), gtk_spin_button_new_with_range (0, 2, 1), 2, 2, 1, 1);

	  if(current_protocol==MQTT || current_protocol==MQTT_SN) {

		  add_settings_image(label, grid, 0, 3);
		  label = gtk_label_new ("Retain");
		  gtk_widget_set_hexpand (label, TRUE);
		  gtk_widget_set_halign (label, GTK_ALIGN_START);
		  gtk_grid_attach (GTK_GRID (grid), label, 1, 3, 1, 1);
		  gtk_grid_attach (GTK_GRID (grid), gtk_switch_new (), 2, 3, 1, 1);

		  add_settings_image(label, grid, 0, 4);
		  label = gtk_label_new ("Duplicate");
		  gtk_widget_set_hexpand (label, TRUE);
		  gtk_widget_set_halign (label, GTK_ALIGN_START);
		  gtk_grid_attach (GTK_GRID (grid), label, 1, 4, 1, 1);
		  gtk_grid_attach (GTK_GRID (grid), gtk_switch_new (), 2, 4, 1, 1);
	  }

	  button = gtk_button_new_with_label ("\nSend\n");
	  gtk_widget_set_name (button, "send_message");
	  g_signal_connect (button, "clicked", G_CALLBACK (send_message_button_handle), grid);
	  gtk_box_pack_start (GTK_BOX(box), GTK_WIDGET(button), FALSE, FALSE, 1);

      //messages box

	  scrolled_window = gtk_scrolled_window_new (NULL,NULL);
	  gtk_container_set_border_width (GTK_CONTAINER (scrolled_window), 0);
	  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

	  box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
	  label = gtk_label_new ("messages list:");
	  gtk_widget_set_halign (label, GTK_ALIGN_START);
	  gtk_box_pack_start (GTK_BOX(box), GTK_WIDGET(label), FALSE, FALSE, 0);
	  messages_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	  gtk_widget_set_name (messages_box, "messages_box");
	  gtk_container_add(GTK_CONTAINER (scrolled_window), messages_box);
	  gtk_box_pack_start (GTK_BOX(box), GTK_WIDGET(scrolled_window), TRUE, TRUE, 0);

	  gtk_stack_add_titled (GTK_STACK(stack), box, "Messages", "Messages L");

	  frame = gtk_frame_new ("Log out");
	  gtk_stack_add_titled (GTK_STACK(stack), frame, " Log Out  ", " Log Out  ");
	  g_signal_connect (frame, "draw", G_CALLBACK (log_out_handle), NULL);

	  box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	  gtk_box_pack_end (GTK_BOX(box), GTK_WIDGET(stack_switcher), FALSE, TRUE, 0);
	  gtk_box_pack_start (GTK_BOX(box), GTK_WIDGET(stack), TRUE, TRUE, 0);
	  gtk_container_add (GTK_CONTAINER (main_window), box);
	  gtk_container_set_focus_child(GTK_CONTAINER (main_window), box);

	  //add topics from db
	  struct MqttModel * mqtt_model =  get_topics_from_db();
	  if(mqtt_model != NULL) {
		  for(int i = 0; i < mqtt_model->topic_size; i++) {
			  struct CommonTopic * common_topic = &(mqtt_model->topic[i]);
			  add_topics_to_list_box(common_topic->topic_name, common_topic->qos);
		  }
	  }

	  //add messages from db
	  mqtt_model =  get_messages_from_db();
	  if(mqtt_model != NULL) {
		  for(int i = 0; i < mqtt_model->publish_size; i++) {
			  struct CommonPublish * cp = &(mqtt_model->common_publish[i]);
			  update_messages_window(cp->content, cp->topic_name, cp->qos, cp->retain, cp->dup, cp->is_incoming);
		  }
	  }

	  gtk_widget_show_all(main_window);
}

void add_topic_button_handle(GtkWidget *widget, gpointer data) {

	GtkWidget * grid = data;

	//topic
	GtkWidget * curr_widget = gtk_grid_get_child_at(GTK_GRID(grid), 2, 0);
	const gchar * topic_name = gtk_entry_get_text(GTK_ENTRY(curr_widget));
	if(strlen(topic_name) == 0 || strcmp("Enter topic",topic_name) == 0) {
		show_warn_window(curr_widget, "Enter Topic");
		return;
	}

	curr_widget = gtk_grid_get_child_at(GTK_GRID(grid), 2, 1);
	int qos = gtk_spin_button_get_value(GTK_SPIN_BUTTON(curr_widget));

	mqtt_listener->send_sub(topic_name, qos);

}

void send_message_button_handle (GtkWidget *widget, gpointer data) {

	GtkWidget * grid = data;

	//content
	GtkWidget * curr_widget = gtk_grid_get_child_at(GTK_GRID(grid), 2, 0);
	const gchar * content = gtk_entry_get_text(GTK_ENTRY(curr_widget));
	if(strlen(content) == 0 || strcmp("Enter content",content) == 0) {
		show_warn_window(curr_widget, "Please enter content");
		return;
	}

	//topic
	curr_widget = gtk_grid_get_child_at(GTK_GRID(grid), 2, 1);
	const gchar * topic_name = gtk_entry_get_text(GTK_ENTRY(curr_widget));
	if(strlen(topic_name) == 0 || strcmp("Enter topic",topic_name) == 0) {
		show_warn_window(curr_widget, "Enter Topic");
		return;
	}

	curr_widget = gtk_grid_get_child_at(GTK_GRID(grid), 2, 2);
	int qos = gtk_spin_button_get_value(GTK_SPIN_BUTTON(curr_widget));

	int retain = 0;
	int dup = 0;
	if(current_protocol==MQTT || current_protocol==MQTT_SN) {
	curr_widget = gtk_grid_get_child_at(GTK_GRID(grid), 2, 3);
	retain = gtk_switch_get_state(GTK_SWITCH(curr_widget));

	curr_widget = gtk_grid_get_child_at(GTK_GRID(grid), 2, 4);
	dup = gtk_switch_get_state(GTK_SWITCH(curr_widget));
	}

	mqtt_listener->send_message(content, topic_name, qos, retain, dup);

}

void get_message(const char * content, const char * topic_name, int qos, int retain, int dup) {
	//store in db
	mqtt_model->save_message_pt(content, topic_name, qos, retain, dup, 1);
	//update gui
	update_messages_window(content, topic_name, qos, retain, dup, 1);
}

void remove_topic_from_list_box(const gchar* name)
{
	//destroy topic
	GtkWidget* child = find_child(topics_box, name);
	if(child != NULL)
		gtk_widget_destroy(child);
	//destroy separator
	child = find_child(topics_box, name);
	if(child != NULL)
		gtk_widget_destroy(child);
}


