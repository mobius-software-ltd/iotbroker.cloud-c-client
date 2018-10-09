/*
 * sn_publish.h
 *
 *  Created on: Aug 17, 2018
 *      Author: konstantinnosach
 */

#ifndef MQTTSN_PACKETS_SN_PUBLISH_H_
#define MQTTSN_PACKETS_SN_PUBLISH_H_

#include "sn_topic.h"

struct SnPublish {

    unsigned short message_id;
    struct SnTopic topic;
    const char * data;
    int dup;
    int retain;
    int is_incoming;

};

#endif /* MQTTSN_PACKETS_SN_PUBLISH_H_ */
