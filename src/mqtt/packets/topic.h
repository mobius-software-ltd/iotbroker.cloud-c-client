/*
 * topic.h
 *
 *  Created on: Jul 31, 2018
 *      Author: konstantinnosach
 */

#ifndef MQTT_PACKETS_TOPIC_H_
#define MQTT_PACKETS_TOPIC_H_

enum QoS {
	AT_MOST_ONCE = 0, AT_LEAST_ONCE = 1, EXACTLY_ONCE = 2
};

struct Topic {
	const char * topic_name;
	enum QoS qos;
};

#endif /* MQTT_PACKETS_TOPIC_H_ */
