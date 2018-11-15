/*
 * state_code.h
 *
 *  Created on: Oct 26, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_AVPS_STATE_CODE_H_
#define SRC_AMQP_AVPS_STATE_CODE_H_

enum StateCode {

	AMQP_RECEIVED = 0x23,
	AMQP_ACCEPTED = 0x24,
	AMQP_REJECTED = 0x25,
	AMQP_RELEASED = 0x26,
	AMQP_MODIFIED = 0x27
};

#endif /* SRC_AMQP_AVPS_STATE_CODE_H_ */
