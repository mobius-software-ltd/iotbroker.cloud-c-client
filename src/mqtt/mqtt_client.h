/**
* Mobius Software LTD
* Copyright 2015-2018, Mobius Software LTD
*
* This is free software; you can redistribute it and/or modify it
* under the terms of the GNU Lesser General Public License as
* published by the Free Software Foundation; either version 2.1 of
* the License, or (at your option) any later version.
*
* This software is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this software; if not, write to the Free
* Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
* 02110-1301 USA, or see the FSF site: http://www.fsf.org.
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
