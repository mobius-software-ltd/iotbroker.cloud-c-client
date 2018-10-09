/*
 * sn_suback.h
 *
 *  Created on: Aug 17, 2018
 *      Author: konstantinnosach
 */

#ifndef MQTTSN_PACKETS_SN_SUBACK_H_
#define MQTTSN_PACKETS_SN_SUBACK_H_

#include "sn_topic.h"

struct SnSuback {

    unsigned short topic_id;
    unsigned short message_id;
    enum SnReturnCode code;
    enum SnQoS allowedQos;
};


#endif /* MQTTSN_PACKETS_SN_SUBACK_H_ */
