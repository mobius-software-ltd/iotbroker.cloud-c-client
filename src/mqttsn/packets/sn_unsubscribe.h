/*
 * sn_unsubscribe.h
 *
 *  Created on: Aug 17, 2018
 *      Author: konstantinnosach
 */

#ifndef MQTTSN_PACKETS_SN_UNSUBSCRIBE_H_
#define MQTTSN_PACKETS_SN_UNSUBSCRIBE_H_

struct SnUnsubscribe {

    unsigned short message_id;
    struct SnTopic * topic;

};


#endif /* MQTTSN_PACKETS_SN_UNSUBSCRIBE_H_ */
