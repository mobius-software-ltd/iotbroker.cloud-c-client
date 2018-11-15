/*
 * amqp_begin.h
 *
 *  Created on: Oct 26, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_PACKETS_AMQP_BEGIN_H_
#define SRC_AMQP_PACKETS_AMQP_BEGIN_H_

#include "../wrappers/amqp_symbol.h"

struct AmqpBegin {
	int * remote_channel;
	long * next_outgoing_id;
	long * incoming_window;
	long * outgoing_window;
	long * handle_max;
	struct AmqpSymbol * offered_capabilities;
	struct AmqpSymbol * desired_capabilities;
	struct AmqpSymbolVoidEntry * properties;
};

#endif /* SRC_AMQP_PACKETS_AMQP_BEGIN_H_ */
