/*
 * sn_subscribe.h
 *
 *  Created on: Aug 17, 2018
 *      Author: konstantinnosach
 */

#ifndef MQTTSN_PACKETS_SN_SUBSCRIBE_H_
#define MQTTSN_PACKETS_SN_SUBSCRIBE_H_

struct SnSubscribe {

    unsigned short message_id;
    struct SnTopic * topic;
    int dup;
};


#endif /* MQTTSN_PACKETS_SN_SUBSCRIBE_H_ */
