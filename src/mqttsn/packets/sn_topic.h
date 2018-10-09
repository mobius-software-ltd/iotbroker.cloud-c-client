/*
 * sn_topic.h
 *
 *  Created on: Aug 17, 2018
 *      Author: konstantinnosach
 */

#ifndef MQTTSN_PACKETS_SN_TOPIC_H_
#define MQTTSN_PACKETS_SN_TOPIC_H_

enum SnTopicType { NAMED = 0, ID = 1, SHORT = 2 };

enum SnQoS { SN_AT_MOST_ONCE = 0, SN_AT_LEAST_ONCE = 1, SN_EXACTLY_ONCE = 2, SN_LEVEL_ONE = 3 };

struct SnTopic {

	unsigned short id;
	const char * value;
	enum SnQoS qos;
	enum SnTopicType topic_type;
};


#endif /* MQTTSN_PACKETS_SN_TOPIC_H_ */
