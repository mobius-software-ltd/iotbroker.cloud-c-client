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

#ifndef MQTTSN_MQTT_SN_TIMERS_H_
#define MQTTSN_MQTT_SN_TIMERS_H_

#include "../mqttsn/packets/sn_message.h"

void start_mqtt_sn_ping_timer(unsigned int delay);
void sn_continue_ping_timer();
void sn_stop_ping_timer();
void sn_start_connect_timer();
void sn_stop_connect_timer();
void sn_start_message_timer();
void sn_stop_message_timer();
void sn_stop_all_timers();

void sn_add_message_in_map(struct SnMessage * message);
struct SnMessage * sn_get_message_from_map(unsigned short packet_id);
void sn_remove_message_from_map (unsigned short packet_id);

unsigned short sn_get_topic_id_from_map(const char * topic_name);
char * sn_get_topic_name_from_map(unsigned short topic_id);
void sn_add_topic_id_in_map(const char * topic_name, unsigned short topic_id);
void sn_add_topic_name_in_map(unsigned short topic_id, const char * topic_name);

#endif /* MQTTSN_MQTT_SN_TIMERS_H_ */
