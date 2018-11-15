/*
 * connection_state.h
 *
 *  Created on: Oct 26, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_STATE_CONNECTION_STATE_H_
#define SRC_AMQP_STATE_CONNECTION_STATE_H_

enum ConnectionState {
	START,
	HDR_CRV,
	YDR_SENT,
	HDR_EXCH,
	OPEN_PIPE,
	OC_PIPE,
	OPEN_RCVD,
	OPEN_SENT,
	CLOSE_PIPE,
	OPENED,
	CLOSE_RCVD,
	CLOSE_SENT,
	DISCARDING,
	END
};

#endif /* SRC_AMQP_STATE_CONNECTION_STATE_H_ */
