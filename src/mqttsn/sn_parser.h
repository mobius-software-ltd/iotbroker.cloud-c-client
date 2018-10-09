/*
 * sn_parser.h
 *
 *  Created on: Aug 17, 2018
 *      Author: konstantinnosach
 */

#ifndef MQTTSN_SN_PARSER_H_
#define MQTTSN_SN_PARSER_H_

char * sn_encode(struct SnMessage * message, int lenght);
struct SnMessage * sn_decode(char * buf);
int sn_get_length(struct SnMessage * sn_message);

#endif /* MQTTSN_SN_PARSER_H_ */
