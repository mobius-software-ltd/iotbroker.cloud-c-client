/*
 * amqp_property.h
 *
 *  Created on: Oct 26, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_PACKETS_AMQP_SYMBOL_VOID_ENTRY_H_
#define SRC_AMQP_PACKETS_AMQP_SYMBOL_VOID_ENTRY_H_

struct AmqpSymbolVoidEntry {

	struct AmqpSymbol * amqp_symbol;
	void * value;
};

#endif /* SRC_AMQP_PACKETS_AMQP_SYMBOL_VOID_ENTRY_H_ */
