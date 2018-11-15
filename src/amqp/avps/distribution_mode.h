/*
 * distribution_mode.h
 *
 *  Created on: Oct 26, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_AVPS_DISTRIBUTION_MODE_H_
#define SRC_AMQP_AVPS_DISTRIBUTION_MODE_H_

enum DistributionMode {

	MOVE, COPY
};

static inline char *string_distribution_mode(enum DistributionMode p)
{
    static char *strings[] = { "move", "copy" };

    return strings[p];
}

#endif /* SRC_AMQP_AVPS_DISTRIBUTION_MODE_H_ */
