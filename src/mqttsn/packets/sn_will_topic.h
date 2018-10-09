/*
 * will_topic.h
 *
 *  Created on: Aug 14, 2018
 *      Author: konstantinnosach
 */

#ifndef MQTTSN_PACKETS_SN_WILL_TOPIC_H_
#define MQTTSN_PACKETS_SN_WILL_TOPIC_H_

#include "sn_topic.h"

struct SnWillTopic {

	  int retain;
	  struct SnTopic * topic;
};

#endif /* MQTTSN_PACKETS_SN_WILL_TOPIC_H_ */
