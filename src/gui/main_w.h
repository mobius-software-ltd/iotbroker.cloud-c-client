/*
 * main.h
 *
 *  Created on: Jul 24, 2018
 *      Author: konstantinnosach
 */
#include <gtk/gtk.h>
#include "../account.h"
#include "../mqtt_listener.h"


void activate_main_window(GtkApplication* application, enum Protocol protocol, struct MqttListener * listener);
void get_message(struct Publish publish);
void subscribe_success(void);
void update_messages_window (const char * content, const char * topic_name, int qos, int retain, int dup, int is_incoming);
void remove_topic_from_list_box(const char * topic_name);
void add_topics_to_list_box (const char * topic_name, int qos);
