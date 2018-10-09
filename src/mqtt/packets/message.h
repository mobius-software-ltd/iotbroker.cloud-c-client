/*
 * message.c
 *
 *  Created on: Jun 23, 2018
 *      Author: konstantinnosach
 */
#ifndef MQTT_PACKETS_MESSAGE_H_
#define MQTT_PACKETS_MESSAGE_H_

enum MessageType {
	CONNECT = 1,
	CONNACK = 2,
	PUBLISH = 3,
	PUBACK = 4,
	PUBREC = 5,
	PUBREL = 6,
	PUBCOMP = 7,
	SUBSCRIBE = 8,
	SUBACK = 9,
	UNSUBSCRIBE = 10,
	UNSUBACK = 11,
	PINGREQ = 12,
	PINGRESP = 13,
	DISCONNECT = 14
};

struct Message {

	enum MessageType message_type;
	void * packet;
};

#endif /* MQTT_PACKETS_MESSAGE_H_ */
