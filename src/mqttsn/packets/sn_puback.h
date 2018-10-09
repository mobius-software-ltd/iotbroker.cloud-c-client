/*
 * sn_puback.h
 *
 *  Created on: Aug 17, 2018
 *      Author: konstantinnosach
 */

#ifndef MQTTSN_PACKETS_SN_PUBACK_H_
#define MQTTSN_PACKETS_SN_PUBACK_H_

struct SnPuback {

    unsigned short topic_id;
    unsigned short  message_id;
    enum SnReturnCode code;
};

#endif /* MQTTSN_PACKETS_SN_PUBACK_H_ */
