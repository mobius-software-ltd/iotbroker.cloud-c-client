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

#ifndef MQTTSN_PACKETS_SN_TOPIC_H_
#define MQTTSN_PACKETS_SN_TOPIC_H_

enum SnTopicType { NAMED = 0, ID = 1, SHORT = 2 };

enum SnQoS { SN_AT_MOST_ONCE = 0, SN_AT_LEAST_ONCE = 1, SN_EXACTLY_ONCE = 2, SN_LEVEL_ONE = 3 };

struct SnTopic {

	unsigned short id;
	const char * value;
	enum SnQoS qos;
	enum SnTopicType topic_type;
};


#endif /* MQTTSN_PACKETS_SN_TOPIC_H_ */
