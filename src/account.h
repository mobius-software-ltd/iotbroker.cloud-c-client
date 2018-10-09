/*
 * account.h
 *
 *  Created on: Jun 27, 2018
 *      Author: konstantinnosach
 */
#include "mqtt/packets/topic.h"
#ifndef ACCOUNT_H_
#define ACCOUNT_H_

enum Protocol {
	MQTT = 0,
	MQTT_SN = 1,
	COAP = 2,
	AMQP = 3,

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
};


#endif /* ACCOUNT_H_ */
