/*
 * amqp_transfer.h
 *
 *  Created on: Oct 26, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_PACKETS_AMQP_TRANSFER_H_
#define SRC_AMQP_PACKETS_AMQP_TRANSFER_H_

#include "../avps/receive_code.h"
#include "../packets/amqp_state.h"
#include "../wrappers/amqp_message_format.h"

struct AmqpTransfer {

	long * handle;
	long * delivery_id;
	char * delivery_tag;
	int * delivery_tag_length;
	struct AmqpMessageFormat * message_format;
	int * settled;
	int * more;
	enum ReceiveCode * rcv_settle_mode;
	struct AmqpState * state;
	int * resume;
	int * aborted;
	int * batchable;
	struct SectionEntry * sections;
	int * section_number;

};

#endif /* SRC_AMQP_PACKETS_AMQP_TRANSFER_H_ */
