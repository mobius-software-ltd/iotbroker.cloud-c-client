/*
 * amqp_header.h
 *
 *  Created on: Oct 26, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_PACKETS_AMQP_HEADER_H_
#define SRC_AMQP_PACKETS_AMQP_HEADER_H_

#include "../avps/header_code.h"

struct AmqpHeader {

	enum HeaderCode code;
	int doff;
	int type;
	int channel;
	void * packet;
};

#endif /* SRC_AMQP_PACKETS_AMQP_HEADER_H_ */
