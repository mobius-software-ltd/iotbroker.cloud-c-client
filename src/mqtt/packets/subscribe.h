/*
 * subscribe.h
 *
 *  Created on: Jun 23, 2018
 *      Author: konstantinnosach
 */
#include "topic.h"

#ifndef SUBSCRIBE_H_
#define SUBSCRIBE_H_

struct Subscribe {
	struct Topic * topics;
	int topics_number;
	unsigned short packet_id;
	time_t time_stamp;
};

#endif /* SUBSCRIBE_H_ */
