/*
 * tlv_constructor.h
 *
 *  Created on: Oct 31, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_TLV_TLV_CONSTRUCTOR_H_
#define SRC_AMQP_TLV_TLV_CONSTRUCTOR_H_

#include "tlv_amqp.h"

struct TlvConstructor {

	enum AmqpType code;
	struct TlvAmqp * tlv;

	int length;
	char * bytes;
};

#endif /* SRC_AMQP_TLV_TLV_CONSTRUCTOR_H_ */
