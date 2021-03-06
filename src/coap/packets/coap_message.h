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

#ifndef SRC_COAP_PACKETS_COAP_MESSAGE_H_
#define SRC_COAP_PACKETS_COAP_MESSAGE_H_

#include "../coap_enum.h"
#include "coap_option.h"

struct CoapMessage {

	unsigned int version;
	enum CoapType type;
	enum CoapCode code;
	unsigned short message_id;
	char * token;
	int token_length;
	struct CoapOption * options;
	int options_amount;
	char * payload;
	time_t time_stamp;

};



#endif /* SRC_COAP_PACKETS_COAP_MESSAGE_H_ */
