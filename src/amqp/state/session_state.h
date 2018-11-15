/*
 * session_state.h
 *
 *  Created on: Oct 26, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_STATE_SESSION_STATE_H_
#define SRC_AMQP_STATE_SESSION_STATE_H_

enum SessionState
{
	UNMAPPED,
	BEGIN_SENT,
	BEGIN_RCVD,
	MAPPED,
	END_SENT,
	END_RCVD,
	DISCARDING
};

#endif /* SRC_AMQP_STATE_SESSION_STATE_H_ */
