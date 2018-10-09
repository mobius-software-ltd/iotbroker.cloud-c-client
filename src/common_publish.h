/*
 * common_publish.h
 *
 *  Created on: Oct 2, 2018
 *      Author: konstantinnosach
 */

#ifndef COMMON_PUBLISH_H_
#define COMMON_PUBLISH_H_

struct CommonPublish {
	const char * topic_name;
	const char * content;
	int qos;
	int retain;
	int dup;
	int is_incoming;
	unsigned short packet_id;
};

#endif /* COMMON_PUBLISH_H_ */
