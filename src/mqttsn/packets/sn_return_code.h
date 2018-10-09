/*
 * return_code.h
 *
 *  Created on: Aug 14, 2018
 *      Author: konstantinnosach
 */

#ifndef MQTTSN_PACKETS_SN_RETURN_CODE_H_
#define MQTTSN_PACKETS_SN_RETURN_CODE_H_

enum SnReturnCode {
	SN_ACCEPTED = 0,
	SN_CONGESTION = 1,
	SN_INVALID_TOPIC_ID = 2,
	SN_NOT_SUPPORTED = 3 };

#endif /* MQTTSN_PACKETS_SN_RETURN_CODE_H_ */
