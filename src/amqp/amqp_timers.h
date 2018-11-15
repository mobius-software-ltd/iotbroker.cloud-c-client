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
