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
