/*
 * mqtt_client.h
 *
 *  Created on: Jul 20, 2018
 *      Author: konstantinnosach
 */

#ifndef MQTT_MQTT_CLIENT_H_
#define MQTT_MQTT_CLIENT_H_

#include "../mqtt_listener.h"
#include "../account.h"
#include "../mqtt/packets/message.h"

void send_disconnect(void);
int init_mqtt_client(struct Account * acc, struct MqttListener * listener);
void send_connect(struct Account * acc);
void send_diconnect();
void send_subscribe(const char * topic_name, enum QoS qos);
void send_unsubscribe(const char * topic_name);
//void send_message(const char * content, const char * topic_name, int qos, int retain, int dup);
void send_ping(void);
void encode_and_fire(struct Message * message);
void fin();

#endif /* MQTT_MQTT_CLIENT_H_ */
