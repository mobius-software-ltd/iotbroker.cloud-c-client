/*
 * amqp_timers.h
 *
 *  Created on: Nov 7, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_AMQP_TIMERS_H_
#define SRC_AMQP_AMQP_TIMERS_H_

void start_amqp_ping_timer(unsigned int delay);
void amqp_stop_ping_timer();
void amqp_start_message_timer();
void amqp_stop_message_timer();
void amqp_stop_all_timers();

void amqp_start_connect_timer();
void amqp_stop_connect_timer();


void amqp_add_message_in_map_by_handler(unsigned short handler, struct AmqpHeader * message);
void amqp_add_message_in_map_by_delivery_id(unsigned short delivery_id, struct AmqpHeader * message);
struct AmqpHeader * amqp_get_message_from_map_by_handler(unsigned short handler);
struct AmqpHeader * amqp_get_message_from_map_by_delivery_id(unsigned short delivery_id);
void amqp_remove_message_from_map_handler (unsigned short handler);
void amqp_remove_message_from_map_delivery_id (unsigned short delivery_id);

void amqp_add_handler_in_map(const char * topic_name, unsigned short handler);
void amqp_add_handler_in_map_outgoing(const char * topic_name, unsigned short handler);
void amqp_add_topic_name_in_map(unsigned short handler, const char * topic_name);
unsigned short amqp_get_handler_from_map(const char * topic_name);
unsigned short amqp_get_handler_from_map_outgoing(const char * topic_name);
char * amqp_get_topic_name_from_map(unsigned short handler);

void amqp_remove_topic_name_from_map (char * topic_name);
void amqp_remove_topic_name_from_map_outgoing (char * topic_name);
void amqp_remove_handle_from_map (unsigned short packet_id);

#endif /* SRC_AMQP_AMQP_TIMERS_H_ */
