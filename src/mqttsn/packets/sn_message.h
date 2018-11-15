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

#ifndef MQTTSN_PACKETS_SN_MESSAGE_H_
#define MQTTSN_PACKETS_SN_MESSAGE_H_

enum SnMessageType {

	SN_ADVERTISE = 0x00,
	SN_SEARCHGW = 0x01,
	SN_GWINFO = 0x02,
	SN_CONNECT = 0x04,
	SN_CONNACK = 0x05,
	SN_WILL_TOPIC_REQ = 0x06,
	SN_WILL_TOPIC = 0x07,
	SN_WILL_MSG_REQ = 0x08,
	SN_WILL_MSG = 0x09,
	SN_REGISTER = 0x0A,
	SN_REGACK = 0x0B,
	SN_PUBLISH = 0x0C,
	SN_PUBACK = 0x0D,
	SN_PUBCOMP = 0x0E,
	SN_PUBREC = 0x0F,
	SN_PUBREL = 0x10,
	SN_SUBSCRIBE = 0x12,
	SN_SUBACK = 0x13,
	SN_UNSUBSCRIBE = 0x14,
	SN_UNSUBACK = 0x15,
	SN_PINGREQ = 0x16,
	SN_PINGRESP = 0x17,
	SN_DISCONNECT = 0x18,
	SN_WILL_TOPIC_UPD = 0x1A,
	SN_WILL_TOPIC_RESP = 0x1B,
	SN_WILL_MSG_UPD = 0x1C,
	SN_WILL_MSG_RESP = 0x1D,
	SN_ENCAPSULATED = 0xFE
};

struct SnMessage {

	enum SnMessageType message_type;
	void * packet;
};

#endif /* MQTTSN_PACKETS_SN_MESSAGE_H_ */
