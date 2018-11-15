/*
 * sasl_mechanisms.h
 *
 *  Created on: Oct 26, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_PACKETS_SASL_MECHANISMS_H_
#define SRC_AMQP_PACKETS_SASL_MECHANISMS_H_

struct SaslMechanisms {

	struct AmqpSymbol * mechanisms;
};

#endif /* SRC_AMQP_PACKETS_SASL_MECHANISMS_H_ */
