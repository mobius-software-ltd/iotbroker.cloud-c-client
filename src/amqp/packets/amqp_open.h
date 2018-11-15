/*
 * amqp_open.h
 *
 *  Created on: Oct 26, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_PACKETS_AMQP_OPEN_H_
#define SRC_AMQP_PACKETS_AMQP_OPEN_H_

struct AmqpOpen {

	char * container_id;
	char * hostname;
	long * max_frame_size;
	int * channel_max;
	long  * idle_timeout;
	struct AmqpSymbol * outgoing_locales;
	struct AmqpSymbol * incoming_locales;
	struct AmqpSymbol * offered_capabilities;
	struct AmqpSymbol * desired_capabilities;
	struct AmqpSymbolVoidEntry * properties;

};

#endif /* SRC_AMQP_PACKETS_AMQP_OPEN_H_ */
