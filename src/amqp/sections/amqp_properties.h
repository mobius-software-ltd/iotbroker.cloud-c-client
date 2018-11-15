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
#ifndef SRC_AMQP_SECTIONS_AMQP_PROPERTIES_H_
#define SRC_AMQP_SECTIONS_AMQP_PROPERTIES_H_

#include <time.h>

struct AmqpProperties {

	struct MessageId message_id;
	char * user_id;
	char * to;
	char * subject;
	char * reply_to;
	char * correlation_id;
	char * content_type;
	char * content_encoding;
	time_t absolute_expiry_time;
	time_t creation_time;
	char * group_id;
	long group_sequence;
	char * reply_to_group_id;
};

#endif /* SRC_AMQP_SECTIONS_AMQP_PROPERTIES_H_ */
