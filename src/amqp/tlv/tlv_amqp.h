/*
 * tlv_amqp.h
 *
 *  Created on: Oct 29, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_TLV_TLV_AMQP_H_
#define SRC_AMQP_TLV_TLV_AMQP_H_

#include "../avps/amqp_type.h"
#include "../tlv/tlv_constructor.h"

struct TlvAmqp {

	enum AmqpType code;
	struct TlvConstructor * constructor;

	struct TlvAmqp * list;
	struct TlvTlvEntry * map;

	unsigned char * value_array;
	int * width;
	int * size;
	int * count;

	int length;
	char * bytes;

};

#endif /* SRC_AMQP_TLV_TLV_AMQP_H_ */
