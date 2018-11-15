/*
 * amqp_target.h
 *
 *  Created on: Oct 26, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_TERMINUS_AMQP_TARGET_H_
#define SRC_AMQP_TERMINUS_AMQP_TARGET_H_

#include "../avps/terminus_durability.h"
#include "../avps/terminus_expiry_policy.h"

struct AmqpTarget {

	const char * address;
	enum TerminusDurability * durable;
	enum TerminusExpiryPolicy * expiry_period;
	long * timeout;
	int * dynamic;
	struct AmqpSymbolVoidEntry * dynamic_node_properties;
	struct AmqpSymbol * capabilities;
};

#endif /* SRC_AMQP_TERMINUS_AMQP_TARGET_H_ */
