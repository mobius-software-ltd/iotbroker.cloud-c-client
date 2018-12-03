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
#include "mqtt/packets/topic.h"
#ifndef ACCOUNT_H_
#define ACCOUNT_H_

enum Protocol {
	MQTT = 0,
	MQTT_SN = 1,
	COAP = 2,
	AMQP = 3,
	WEBSOCKETS = 4
};

struct Account {
        int id;
        enum Protocol protocol;
        const char * username;
        const char * password;
        const char * client_id;
        const char * server_host;
        int server_port;
        int clean_session;
        int keep_alive;
        const char * will_topic;
        int is_retain;
        enum QoS qos;
        int is_default;
        const char * will;

    	int is_secure;
    	const char * certificate;
    	const char * certificate_password;
};


#endif /* ACCOUNT_H_ */
