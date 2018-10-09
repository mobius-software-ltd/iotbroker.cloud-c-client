/*
 * suback.h
 *
 *  Created on: Jun 25, 2018
 *      Author: konstantinnosach
 */

#ifndef MQTT_PACKETS_SUBACK_H_
#define MQTT_PACKETS_SUBACK_H_

enum SubackCode {
	ACCEPTED_QOS0 = 0, ACCEPTED_QOS1 = 1, ACCEPTED_QOS2 = 2, FAILURE = 128
};

struct Suback {
	unsigned short packet_id;
	int codes_number;
	enum SubackCode * codes;
};

#endif /* MQTT_PACKETS_SUBACK_H_ */
