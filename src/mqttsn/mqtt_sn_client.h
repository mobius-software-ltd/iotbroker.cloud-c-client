/*
 * mqtt_sn_client.h
 *
 *  Created on: Aug 22, 2018
 *      Author: konstantinnosach
 */

#ifndef MQTTSN_MQTT_SN_CLIENT_H_
#define MQTTSN_MQTT_SN_CLIENT_H_

#include "../mqtt_listener.h"
#include "../account.h"
#include "packets/sn_message.h"

int init_mqtt_sn_client(struct Account * acc, struct MqttListener * listener);
void send_sn_connect(struct Account * account);
void send_sn_subscribe(char * topic_name, int qos);
void sn_send_ping(void);
void send_sn_disconnect();
void sn_encode_and_fire(struct SnMessage * sn_message);

#endif /* MQTTSN_MQTT_SN_CLIENT_H_ */
