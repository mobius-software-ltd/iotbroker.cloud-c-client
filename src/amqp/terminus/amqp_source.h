/**
* Mobius Software LTD
* Copyright 2015-2018, Mobius Software LTD
*
* This is free software; you can redistribute it and/or modify it
* under the terms of the GNU Lesser General Public License as
* published by the Free Software Foundation; either version 2.1 of
* the License, or (at your option) any later version.
*
* This software is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this software; if not, write to the Free
* Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
* 02110-1301 USA, or see the FSF site: http://www.fsf.org.
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
