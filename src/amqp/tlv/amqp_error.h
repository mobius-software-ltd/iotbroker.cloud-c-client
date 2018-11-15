/*
 * amqp_error.h
 *
 *  Created on: Oct 26, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_TLV_AMQP_ERROR_H_
#define SRC_AMQP_TLV_AMQP_ERROR_H_

#include "../avps/error_code.h"

struct AmqpError {

	enum ErrorCode * condition;
	char * description;
	struct AmqpSymbolVoidEntry * info;
};


#endif /* SRC_AMQP_TLV_AMQP_ERROR_H_ */
