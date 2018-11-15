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
#include "../account.h"
#include "../mqtt_listener.h"


void activate_main_window(GtkApplication* application, enum Protocol protocol, struct MqttListener * listener);
void get_message(struct Publish publish);
void subscribe_success(void);
void update_messages_window (const char * content, const char * topic_name, int qos, int retain, int dup, int is_incoming);
void remove_topic_from_list_box(const char * topic_name);
void add_topics_to_list_box (const char * topic_name, int qos);
