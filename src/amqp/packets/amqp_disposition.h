/*
 * amqp_disposition.h
 *
 *  Created on: Oct 26, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_PACKETS_AMQP_DISPOSITION_H_
#define SRC_AMQP_PACKETS_AMQP_DISPOSITION_H_

#include "../avps/role_code.h"
#include "../packets/amqp_state.h"

struct AmqpDisposition {

	enum RoleCode * role;
	long * first;
	long * last;
	int * settled;
	struct AmqpState * state;
	int * batchable;

};


#endif /* SRC_AMQP_PACKETS_AMQP_DISPOSITION_H_ */
