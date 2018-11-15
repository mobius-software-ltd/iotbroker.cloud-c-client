/*
 * tlv_tlv_entry.h
 *
 *  Created on: Nov 2, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_SECTIONS_TLV_TLV_ENTRY_H_
#define SRC_AMQP_SECTIONS_TLV_TLV_ENTRY_H_

struct TlvTlvEntry {

	struct TlvAmqp * key;
	struct TlvAmqp * value;

};

#endif /* SRC_AMQP_SECTIONS_TLV_TLV_ENTRY_H_ */
