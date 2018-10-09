/*
 * sn_will_topic_upd.h
 *
 *  Created on: Aug 17, 2018
 *      Author: konstantinnosach
 */

#ifndef MQTTSN_PACKETS_SN_WILL_TOPIC_UPD_H_
#define MQTTSN_PACKETS_SN_WILL_TOPIC_UPD_H_

#include "sn_topic.h"

struct SnWillTopicUpd {

    int retain;
    struct SnTopic * topic;

};

#endif /* MQTTSN_PACKETS_SN_WILL_TOPIC_UPD_H_ */
