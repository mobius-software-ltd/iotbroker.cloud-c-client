/*
 * coap_parser.h
 *
 *  Created on: Oct 11, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_COAP_COAP_PARSER_H_
#define SRC_COAP_COAP_PARSER_H_

#include "packets/coap_message.h"

char * coap_encode(struct CoapMessage * message, int * length);
struct CoapMessage * coap_decode(char * buf, int length);

#endif /* SRC_COAP_COAP_PARSER_H_ */
