/*
 * amqp_detach.h
 *
 *  Created on: Oct 26, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_PACKETS_AMQP_DETACH_H_
#define SRC_AMQP_PACKETS_AMQP_DETACH_H_

#include "../tlv/amqp_error.h"

struct AmqpDetach {

	long * handle;
	int * closed;
	struct AmqpError * error;

};


#endif /* SRC_AMQP_PACKETS_AMQP_DETACH_H_ */
