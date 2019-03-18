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
#ifndef MQTT_LISTENER_H_
#define MQTT_LISTENER_H_

#include "mqtt/packets/publish.h"
#include "account.h"

typedef void (*ConnectionSuccessful)(void);
typedef void (*ConnectionUnsuccessful)(int cause);
typedef void (*SubscribeSuccessful)(void);
typedef void (*GetPublish)(const char * content, const char * topic_name, int qos, int retain, int dup);
typedef void (*SendConnect)(struct Account * account);
typedef void (*SendSubscribe)(const char * topic_name, int qos);
typedef void (*SendUnsubscribe)(const char * topic_name);
typedef void (*SendDisconnect)(void);
typedef void (*SendMessage)(const char * content, const char * topic_name, int qos, int retain, int dup);

struct MqttListener {

	ConnectionSuccessful  cs_pt;
	ConnectionUnsuccessful  cu_pt;
	SubscribeSuccessful  subs_pt;
	GetPublish get_pub_pt;
	SendConnect send_connect;
	SendSubscribe send_sub;
	SendDisconnect send_disconnect;
	SendMessage send_message;
	SendUnsubscribe send_unsubscribe;
};

#endif /* MQTT_LISTENER_H_ */
