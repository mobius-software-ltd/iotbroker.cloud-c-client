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
