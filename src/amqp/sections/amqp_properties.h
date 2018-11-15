/*
 * amqp_properties.h
 *
 *  Created on: Oct 26, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_SECTIONS_AMQP_PROPERTIES_H_
#define SRC_AMQP_SECTIONS_AMQP_PROPERTIES_H_

#include <time.h>

struct AmqpProperties {

	struct MessageId message_id;
	char * user_id;
	char * to;
	char * subject;
	char * reply_to;
	char * correlation_id;
	char * content_type;
	char * content_encoding;
	time_t absolute_expiry_time;
	time_t creation_time;
	char * group_id;
	long group_sequence;
	char * reply_to_group_id;
};

#endif /* SRC_AMQP_SECTIONS_AMQP_PROPERTIES_H_ */
