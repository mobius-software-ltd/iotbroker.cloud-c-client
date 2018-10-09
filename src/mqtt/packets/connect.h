#ifndef MQTT_PACKETS_CONNECT_H_
#define MQTT_PACKETS_CONNECT_H_

#include "topic.h"

struct Will {
	struct Topic topic;
	const char * content;
	int retain; //boolean
};

struct Connect {

	const char * username;
	const char * password;
	const char * client_id;

	int clean_session; //boolean
	int will_flag;
	unsigned short keepalive;
	unsigned short protocol_level;

	struct Will will;
};

#endif /* MQTT_PACKETS_CONNECT_H_ */
