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
