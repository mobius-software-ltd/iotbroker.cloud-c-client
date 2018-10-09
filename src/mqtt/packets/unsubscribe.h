/*
 * unsubscribe.h
 *
 *  Created on: Jun 25, 2018
 *      Author: konstantinnosach
 */
#include "topic.h"

#ifndef UNSUBSCRIBE_H_
#define UNSUBSCRIBE_H_

struct Unsubscribe {
	int topics_number;
	struct Topic * topics;
	unsigned short packet_id;
	time_t time_stamp;
};

#endif /* UNSUBSCRIBE_H_ */
