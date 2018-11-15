/*
 * amqp_attach.h
 *
 *  Created on: Oct 26, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_PACKETS_AMQP_ATTACH_H_
#define SRC_AMQP_PACKETS_AMQP_ATTACH_H_

#include "../avps/role_code.h"
#include "../avps/send_code.h"
#include "../avps/receive_code.h"
#include "../terminus/amqp_source.h"
#include "../terminus/amqp_target.h"


struct AmqpAttach {

	const char * name;
	long * handle;
	enum RoleCode * role;
	enum SendCode * snd_settle_mode;
	enum ReceiveCode * rcv_settle_mode;
	struct AmqpSource * source;
	struct AmqpTarget * target;
	struct AmqpSymbolVoidEntry * unsettled;
	int * incomplete_unsettled;
	long * initial_delivery_count;
	long * max_message_size;
	struct AmqpSymbol * offered_capabilities;
	struct AmqpSymbol * desired_capabilities;
	struct AmqpSymbolVoidEntry * properties;

};

#endif /* SRC_AMQP_PACKETS_AMQP_ATTACH_H_ */
