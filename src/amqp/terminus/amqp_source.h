/*
 * amqp_source.h
 *
 *  Created on: Oct 26, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_TERMINUS_AMQP_SOURCE_H_
#define SRC_AMQP_TERMINUS_AMQP_SOURCE_H_

#include "../avps/terminus_durability.h"
#include "../avps/terminus_expiry_policy.h"
#include "../avps/distribution_mode.h"
#include "../tlv/amqp_outcome.h"

struct AmqpSource {

	char * address;
	enum TerminusDurability * durable;
	enum TerminusExpiryPolicy * expiry_period;
	long * timeout;
	int * dynamic;
	struct AmqpSymbolVoidEntry * dynamic_node_properties;
	enum DistributionMode * distribution_mode;
	struct AmqpSymbolVoidEntry * filter;
	struct AmqpOutcome * default_outcome;
	struct AmqpSymbol * outcomes;
	struct AmqpSymbol * capabilities;

};

#endif /* SRC_AMQP_TERMINUS_AMQP_SOURCE_H_ */
