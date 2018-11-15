/*
 * uuid_id.h
 *
 *  Created on: Oct 26, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_WRAPPERS_UUID_ID_H_
#define SRC_AMQP_WRAPPERS_UUID_ID_H_
#include <uuid/uuid.h>


struct UuidId {
	uuid_t id;
};

#endif /* SRC_AMQP_WRAPPERS_UUID_ID_H_ */
