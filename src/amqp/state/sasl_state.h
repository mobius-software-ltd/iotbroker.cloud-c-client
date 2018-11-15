/*
 * sasl_state.h
 *
 *  Created on: Oct 26, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_STATE_SASL_STATE_H_
#define SRC_AMQP_STATE_SASL_STATE_H_

enum SASLState
{
	STATE_NONE,
	MECHANISMS_SENT,
	INIT_RECEIVED,
	CHALLENGE_SENT,
	RESPONSE_RECEIVED,
	NEGOTIATED
};

#endif /* SRC_AMQP_STATE_SASL_STATE_H_ */
