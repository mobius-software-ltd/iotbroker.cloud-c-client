/*
 * coap_message.h
 *
 *  Created on: Oct 11, 2018
 *      Author: konstantinnosach
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
	const char * payload;

};



#endif /* SRC_COAP_PACKETS_COAP_MESSAGE_H_ */
