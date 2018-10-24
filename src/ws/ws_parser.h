/*
 * ws_parser.h
 *
 *  Created on: Oct 24, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_WS_WS_PARSER_H_
#define SRC_WS_WS_PARSER_H_

#include "../mqtt/packets/message.h"

char * ws_encode(struct Message * message);
struct Message * ws_decode(char * data);

#endif /* SRC_WS_WS_PARSER_H_ */
