/*
 * message_header.h
 *
 *  Created on: Oct 26, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_SECTIONS_MESSAGE_HEADER_H_
#define SRC_AMQP_SECTIONS_MESSAGE_HEADER_H_

struct MessageHeader {

	int durable;
	short priority;
	long milliseconds;
	int first_aquirer;
	long delivery_count;
};

#endif /* SRC_AMQP_SECTIONS_MESSAGE_HEADER_H_ */
