/*
 * amqp_outcome.h
 *
 *  Created on: Oct 26, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_TLV_AMQP_OUTCOME_H_
#define SRC_AMQP_TLV_AMQP_OUTCOME_H_

#include "../avps/state_code.h"

struct AmqpOutcome {

	enum StateCode code;
	void * delivery_state;

};

#endif /* SRC_AMQP_TLV_AMQP_OUTCOME_H_ */
