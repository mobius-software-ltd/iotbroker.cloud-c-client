/*
 * amqp_flow.h
 *
 *  Created on: Oct 26, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_PACKETS_AMQP_FLOW_H_
#define SRC_AMQP_PACKETS_AMQP_FLOW_H_

struct AmqpFlow {

	long next_incoming_id;
	long incoming_window;
	long next_outgoing_id;
	long outgoing_window;
	long handle;
	long delivery_count;
	long link_credit;
	long avaliable;
	int drain;
	int echo;
	struct AmqpSymbolVoidEntry * properties;
};

#endif /* SRC_AMQP_PACKETS_AMQP_FLOW_H_ */
