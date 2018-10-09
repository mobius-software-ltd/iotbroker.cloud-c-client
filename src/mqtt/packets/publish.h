/*
 * publish.h
 *
 *  Created on: Jun 23, 2018
 *      Author: konstantinnosach
 */
#ifndef MQTT_PACKETS_PUBLISH_H_
#define MQTT_PACKETS_PUBLISH_H_

#include "topic.h"

struct Publish {
	struct Topic topic;
	const char * content;
	int retain;
	int dup;
	int is_incoming;
	unsigned short packet_id;
	time_t time_stamp;
};

#endif /* MQTT_PACKETS_PUBLISH_H_ */
