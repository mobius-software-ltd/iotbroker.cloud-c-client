/*
 * coap_timers.h
 *
 *  Created on: Oct 16, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_COAP_COAP_TIMERS_H_
#define SRC_COAP_COAP_TIMERS_H_

#include "packets/coap_message.h"

void start_coap_ping_timer(unsigned int delay);
void coap_stop_ping_timer();
void coap_start_message_timer();
void coap_stop_message_timer();
void coap_stop_all_timers();

void coap_add_message_in_map(struct CoapMessage * message);
struct CoapMessage * coap_get_message_from_map(unsigned short packet_id);
void coap_remove_message_from_map (unsigned short packet_id);

#endif /* SRC_COAP_COAP_TIMERS_H_ */
