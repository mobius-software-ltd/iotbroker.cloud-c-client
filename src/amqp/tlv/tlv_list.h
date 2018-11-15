/*
 * tlv_list.h
 *
 *  Created on: Oct 29, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_TLV_TLV_LIST_H_
#define SRC_AMQP_TLV_TLV_LIST_H_

#include "tlv_constructor.h"

struct TlvList {

	enum AmqpType code;

	struct TlvAmqp * list;
	struct TlvConstructor constructor;
	//int list_size;
	int width;
	int size;
	int count;

	int length;
	char * bytes;
};

#endif /* SRC_AMQP_TLV_TLV_LIST_H_ */
