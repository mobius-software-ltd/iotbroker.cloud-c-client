/*
 * unwrapper.h
 *
 *  Created on: Nov 2, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_WRAPPERS_UNWRAPPER_H_
#define SRC_AMQP_WRAPPERS_UNWRAPPER_H_

char * unwrap_binary(struct TlvAmqp * tlv);
struct AmqpSymbol * unwrap_symbol(struct TlvAmqp * tlv);
void ** unwrap_array(struct TlvAmqp * tlv);
char * unwrap_string (struct TlvAmqp * tlv);
short * unwrap_ubyte(struct TlvAmqp * tlv);
long * unwrap_uint(struct TlvAmqp * tlv);
int * unwrap_ushort(struct TlvAmqp * tlv);
struct TlvTlvEntry * unwrap_map(struct TlvAmqp * tlv);
int * unwrap_bool(struct TlvAmqp * tlv);
unsigned long * unwrap_ulong(struct TlvAmqp * tlv);


#endif /* SRC_AMQP_WRAPPERS_UNWRAPPER_H_ */
