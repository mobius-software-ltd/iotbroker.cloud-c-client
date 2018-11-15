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

#ifndef MQTT_MQTT_TIMERS_H_
#define MQTT_MQTT_TIMERS_H_

#include "../mqtt/packets/message.h"

void start_mqtt_ping_timer(unsigned int delay);
void continue_ping_timer();
void stop_ping_timer();
void start_connect_timer();
void stop_connect_timer();
void start_message_timer();
void stop_message_timer();
void stop_all_timers();

void add_message_in_map(struct Message * message);
struct Message * get_message_from_map(unsigned short packet_id);
void remove_message_from_map (unsigned short packet_id);

#endif /* MQTT_MQTT_TIMERS_H_ */
