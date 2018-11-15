/*
 * SaslInit.h
 *
 *  Created on: Oct 26, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_PACKETS_SASL_INIT_H_
#define SRC_AMQP_PACKETS_SASL_INIT_H_

struct SaslInit {

	struct AmqpSymbol * mechanism;
	char * initial_response;
	int initial_response_length;
	char * host_name;
};

#endif /* SRC_AMQP_PACKETS_SASL_INIT_H_ */
