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

#ifndef SRC_AMQP_PACKETS_AMQP_ATTACH_H_
#define SRC_AMQP_PACKETS_AMQP_ATTACH_H_

#include "../avps/role_code.h"
#include "../avps/send_code.h"
#include "../avps/receive_code.h"
#include "../terminus/amqp_source.h"
#include "../terminus/amqp_target.h"


struct AmqpAttach {

	const char * name;
	long * handle;
	enum RoleCode * role;
	enum SendCode * snd_settle_mode;
	enum ReceiveCode * rcv_settle_mode;
	struct AmqpSource * source;
	struct AmqpTarget * target;
	struct AmqpSymbolVoidEntry * unsettled;
	int * incomplete_unsettled;
	long * initial_delivery_count;
	long * max_message_size;
	struct AmqpSymbol * offered_capabilities;
	struct AmqpSymbol * desired_capabilities;
	struct AmqpSymbolVoidEntry * properties;

};

#endif /* SRC_AMQP_PACKETS_AMQP_ATTACH_H_ */
