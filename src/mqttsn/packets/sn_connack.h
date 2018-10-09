/*
 * sn_connack.h
 *
 *  Created on: Aug 14, 2018
 *      Author: konstantinnosach
 */

#ifndef MQTTSN_PACKETS_SN_CONNACK_H_
#define MQTTSN_PACKETS_SN_CONNACK_H_

#include "sn_return_code.h"

struct SnConnack {

	enum SnReturnCode return_code;
};


#endif /* MQTTSN_PACKETS_SN_CONNACK_H_ */
