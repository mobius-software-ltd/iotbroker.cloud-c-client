/*
 * amqp_parser.h
 *
 *  Created on: Oct 31, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_AMQP_PARSER_H_
#define SRC_AMQP_AMQP_PARSER_H_

#include "packets/amqp_header.h"

char * amqp_encode(struct AmqpHeader * header, int * length);
struct AmqpHeader * amqp_decode(char * data, int readable_bytes);

#endif /* SRC_AMQP_AMQP_PARSER_H_ */
