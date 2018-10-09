/*
 * mqtt_listener.h
 *
 *  Created on: Jul 24, 2018
 *      Author: konstantinnosach
 */
#ifndef MQTT_LISTENER_H_
#define MQTT_LISTENER_H_

#include "mqtt/packets/publish.h"
#include "account.h"

typedef void (*ConnectionSuccessful)(void);
typedef void (*SubscribeSuccessful)(void);
typedef void (*GetPublish)(const char * content, const char * topic_name, int qos, int retain, int dup);
typedef void (*SendConnect)(struct Account * account);
typedef void (*SendSubscribe)(const char * topic_name, int qos);
typedef void (*SendUnsubscribe)(const char * topic_name);
typedef void (*SendDisconnect)(void);
typedef void (*SendMessage)(const char * content, const char * topic_name, int qos, int retain, int dup);

struct MqttListener {

	ConnectionSuccessful  cs_pt;
	SubscribeSuccessful  subs_pt;
	GetPublish get_pub_pt;
	SendConnect send_connect;
	SendSubscribe send_sub;
	SendDisconnect send_disconnect;
	SendMessage send_message;
	SendUnsubscribe send_unsubscribe;
};

#endif /* MQTT_LISTENER_H_ */
