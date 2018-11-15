/*
 * sasl_outcome.h
 *
 *  Created on: Oct 26, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_PACKETS_SASL_OUTCOME_H_
#define SRC_AMQP_PACKETS_SASL_OUTCOME_H_

#include "../avps/outcome_code.h"

struct SaslOutcome {

	enum OutcomeCode outcome_code;
	char * additional_data;
};


#endif /* SRC_AMQP_PACKETS_SASL_OUTCOME_H_ */
