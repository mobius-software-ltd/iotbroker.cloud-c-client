/*
 * parser.h
 *
 *  Created on: Jul 20, 2018
 *      Author: konstantinnosach
 */

#ifndef MQTT_PARSER_H_
#define MQTT_PARSER_H_

#include "packets/message.h"

int get_length(struct Message * message);
char * encode(struct Message * message, int lenght);
struct Message * decode(char * buf);

#endif /* MQTT_PARSER_H_ */
