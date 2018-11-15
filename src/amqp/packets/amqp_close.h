/*
 * amqp_close.h
 *
 *  Created on: Oct 26, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_PACKETS_AMQP_CLOSE_H_
#define SRC_AMQP_PACKETS_AMQP_CLOSE_H_

#include "../tlv/amqp_error.h"

struct AmqpClose {

	struct AmqpError error;
};


#endif /* SRC_AMQP_PACKETS_AMQP_CLOSE_H_ */
