/*
 * mqtt_timers.h
 *
 *  Created on: Aug 10, 2018
 *      Author: konstantinnosach
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
