/*
 * amqp_state.h
 *
 *  Created on: Oct 26, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_PACKETS_AMQP_STATE_H_
#define SRC_AMQP_PACKETS_AMQP_STATE_H_

#include "../avps/state_code.h"

struct AmqpState {

	void * amqp_state;
	enum StateCode code;
};

#endif /* SRC_AMQP_PACKETS_AMQP_STATE_H_ */
