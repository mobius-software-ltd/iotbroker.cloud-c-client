/*
 * amqp_proto_header.h
 *
 *  Created on: Oct 26, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_PACKETS_AMQP_PROTO_HEADER_H_
#define SRC_AMQP_PACKETS_AMQP_PROTO_HEADER_H_

struct AmqpProtoHeader {

	char * protocol;
	int protocol_id;
	int version_major;
	int version_minor;
	int version_revision;

};

#endif /* SRC_AMQP_PACKETS_AMQP_PROTO_HEADER_H_ */
