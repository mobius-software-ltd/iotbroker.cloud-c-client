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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../helpers.h"
#include "../mqtt/packets/pubcomp.h"
#include "../mqtt/packets/connack.h"
#include "../mqtt/packets/connect.h"
#include "../mqtt/packets/message.h"
#include "../mqtt/packets/puback.h"
#include "../mqtt/packets/pubcomp.h"
#include "../mqtt/packets/publish.h"
#include "../mqtt/packets/pubrec.h"
#include "../mqtt/packets/pubrel.h"
#include "../mqtt/packets/suback.h"
#include "../mqtt/packets/subscribe.h"
#include "../mqtt/packets/unsuback.h"
#include "../mqtt/packets/unsubscribe.h"

struct LengthDetails decode_remaining_length(char buf[]);

const char PROTOCOL_NAME[] = "MQTT";
const char DEFAULT_PROTOCOL_LEVEL = 4;

int get_length(struct Message * message) {

	int length = 0;
	switch (message->message_type) {
	case CONNECT: {
		struct Connect c = *(struct Connect*) message->packet;
		length = 10;
		length += strlen(c.client_id) + 2;
		length +=
				c.will_flag != 0 ?
						strlen(c.will.content) + strlen(c.will.topic.topic_name)
								+ 4 :
						0;
		length += strlen(c.username) != 0 ? strlen(c.username) + 2 : 0;
		length += strlen(c.password) != 0 ? strlen(c.password) + 2 : 0;
		break;
	}
	case CONNACK:
	case PUBACK:
	case PUBREC:
	case PUBREL:
	case PUBCOMP:
	case UNSUBACK:
		length = 2;
		break;
	case PUBLISH: {
		struct Publish p = *(struct Publish*) message->packet;
		length = p.packet_id != 0 ? 2 : 0;
		length += strlen(p.topic.topic_name) + 2;
		length += strlen(p.content);
		return length;
	}
	case SUBSCRIBE: {
		struct Subscribe s = *(struct Subscribe*) message->packet;
		length = 2;
		for (int i = 0; i < s.topics_number; i++) {
			length += strlen(s.topics[i].topic_name) + 3;
		}

		break;
	}
	case SUBACK: {
		struct Suback sa = *(struct Suback*) message->packet;
		return 2 + sa.codes_number;
	}
	case UNSUBSCRIBE: {
		struct Unsubscribe us = *(struct Unsubscribe*) message->packet;
		length = 2;
		for (int i = 0; i < us.topics_number; i++)
			length += strlen(us.topics[i].topic_name) + 2;
		break;
	}
	case PINGREQ:
	case PINGRESP:
	case DISCONNECT:
		length = 0;
		break;
	}
	return length;
}

char * encode(struct Message * message, int length) {

	char * buf = NULL;
	buf = malloc(sizeof(char) * (length+2));
	int i = 0;
	unsigned short string_length = 0;
	switch (message->message_type) {
	case CONNECT: {
		struct Connect c = *(struct Connect*) message->packet;

		buf[i++] = message->message_type << 4;
		i = add_packet_length(length, buf);
		buf[i++] = 0;

		string_length = strlen(PROTOCOL_NAME);
		buf[i++] = string_length;
		memcpy(&buf[i], PROTOCOL_NAME, string_length);
		i += string_length;

		buf[i++] = DEFAULT_PROTOCOL_LEVEL;
		char contentFlags = 0;
		if (c.clean_session)
			contentFlags += 0x02;

		if (c.will_flag) {
			contentFlags += 0x04;
			contentFlags += ((char) c.will.topic.qos) << 3;
			if (c.will.retain)
				contentFlags += 0x20;
		}

		if (strlen(c.username) != 0)
			contentFlags += 0x40;

		if (strlen(c.password) != 0)
			contentFlags += 0x80;

		buf[i++] = contentFlags;

		i += add_short(&buf[i], c.keepalive);

		unsigned short client_id_length = strlen(c.client_id);
		i += add_short(&buf[i], client_id_length);

		memcpy(&buf[i], c.client_id, client_id_length);
		i += client_id_length;
		if (c.will_flag != 0) {
			string_length = strlen(c.will.topic.topic_name);
			if (string_length != 0) {
				i += add_short(&buf[i], string_length);
				memcpy(&buf[i], c.will.topic.topic_name, string_length);
				i += string_length;
			}

			string_length = strlen(c.will.content);
			if (string_length != 0) {
				i += add_short(&buf[i], string_length);
				memcpy(&buf[i], c.will.content, string_length);
				i += string_length;
			}
		}

		string_length = strlen(c.username);
		if (string_length != 0) {
			i += add_short(&buf[i], string_length);
			memcpy(&buf[i], c.username, string_length);
			i += string_length;
		}

		string_length = strlen(c.password);
		if (string_length != 0) {
			i += add_short(&buf[i], string_length);
			memcpy(&buf[i], c.password, string_length);
			i += string_length;
		}
		break;
	}
	case CONNACK: {
		struct Connack ca = *(struct Connack*) message->packet;
		buf[i++] = message->message_type << 4;
		buf[i++] = (char) get_length(message);
		buf[i++] = ca.session_present;
		buf[i++] = (char) ca.return_code;
		break;
	}
	case PUBLISH: {
		struct Publish p = *(struct Publish*) message->packet;
		char firstByte = message->message_type << 4;
		if (p.dup)
			firstByte += 0x08;

		firstByte += ((char) p.topic.qos) << 1;

		if (p.retain)
			firstByte += 0x01;

		buf[i] = firstByte;

		i = add_packet_length(length, buf);

		string_length = strlen(p.topic.topic_name);
		if (string_length != 0) {
			i += add_short(&buf[i], string_length);
			memcpy(&buf[i], p.topic.topic_name, string_length);
			i += string_length;
		}
		if (p.packet_id != 0)
			i += add_short(&buf[i], p.packet_id);

		memcpy(&buf[i], p.content, strlen(p.content));
		break;
	}
	case PUBACK: {
		struct Puback pa = *(struct Puback*) message->packet;
		buf[i++] = message->message_type << 4;
		buf[i++] = length;
		add_short(&buf[i], pa.packet_id);
		break;
	}
	case PUBREC: {
		struct Pubrec pr = *(struct Pubrec*) message->packet;
		buf[i++] = message->message_type << 4;
		buf[i++] = length;
		add_short(&buf[i], pr.packet_id);
		break;
	}
	case PUBREL: {
		struct Pubrel pl = *(struct Pubrel*) message->packet;
		buf[i++] = (message->message_type << 4) | 0x2;
		buf[i++] = length;
		add_short(&buf[i], pl.packet_id);
		break;
	}
	case PUBCOMP: {
		struct Pubcomp pc = *(struct Pubcomp*) message->packet;
		buf[i++] = message->message_type << 4;
		buf[i++] = length;
		add_short(&buf[i], pc.packet_id);
		break;
	}
	case SUBSCRIBE: {
		struct Subscribe s = *(struct Subscribe*) message->packet;
		buf[i++] = (message->message_type << 4) | 0x2;
		i = add_packet_length(length, buf);
		i += add_short(&buf[i], s.packet_id);
		for (int j = 0; j < s.topics_number; j++) {
			string_length = strlen(s.topics[j].topic_name);
			if (string_length != 0) {
				i += add_short(&buf[i], string_length);
				memcpy(&buf[i], s.topics[j].topic_name, string_length);
				i += string_length;
			}
			buf[i] = (char) s.topics[j].qos;
		}
		break;
	}
	case SUBACK: {
		struct Suback sa = *(struct Suback*) message->packet;
		buf[i++] = message->message_type << 4;
		i += add_short(&buf[i], sa.packet_id);
		for (int j = 0; j < sa.codes_number; j++)
			buf[i++] = sa.codes[j];

		break;
	}
	case UNSUBSCRIBE: {
		struct Unsubscribe us = *(struct Unsubscribe*) message->packet;
		buf[i++] = (message->message_type << 4) | 0x2;
		i = add_packet_length(length, buf);
		i += add_short(&buf[i], us.packet_id);
		for (int j = 0; j < us.topics_number; j++) {
			string_length = strlen(us.topics[j].topic_name);
			if (string_length != 0) {
				i += add_short(&buf[i], string_length);
				memcpy(&buf[i], us.topics[j].topic_name, string_length);
				i += string_length;
			}
			//buf[i] = (char) us.topics[j].qos;
		}
		break;
	}
	case UNSUBACK: {
		struct Unsuback usa = *(struct Unsuback*) message->packet;
		buf[i++] = message->message_type << 4;
		i = add_packet_length(length, buf);
		add_short(&buf[i], usa.packet_id);
		break;
	}
	case DISCONNECT:
	case PINGREQ:
	case PINGRESP:
		buf[i++] = (message->message_type << 4) | 0x2;
		buf[i] = (char)0;
		break;
	}
	return buf;
}

struct Message * decode(char * buf) {
	struct Message * m = malloc (sizeof (struct Message));
	int i = 0;
	char fixedHeader = buf[i++];

	struct LengthDetails length_details;

	enum MessageType type = (fixedHeader >> 4) & 0xf;
	switch (type) {
	case CONNECT: {
		m->message_type = type;
		struct Connect c;
		length_details = decode_remaining_length(buf);
		i += length_details.bytes_used;
		unsigned short protocol_name_size = get_short(buf, i);
		i += 2;
		char protocol_name[protocol_name_size];
		memcpy(protocol_name, &buf[i], protocol_name_size);
		if (strcmp(PROTOCOL_NAME, protocol_name) != 0) {
			//printf("CONNECT, protocol-name set to : %s", protocol_name);
			return m;
		}
		i += protocol_name_size;

		c.protocol_level = buf[i++];
		char contentFlags = buf[i++];

		int user_name_flag = (contentFlags >> 7) & 1;
		int user_pass_flag = (contentFlags >> 6) & 1;
		c.will.retain = (contentFlags >> 5) & 1;

		enum QoS will_qos = ((contentFlags & 0x1f) >> 3) & 3;

		if (will_qos < 0 || will_qos > 2) {
			printf("CONNECT, will QoS set to : %d\n", will_qos);
			return m;
		}
		c.will_flag = (contentFlags >> 2) & 1;

		if (will_qos != AT_MOST_ONCE && c.will_flag == 0) {
			printf("CONNECT, will QoS set to %d, willFlag not set\n", will_qos);
			return m;
		}
		c.will.topic.qos = will_qos;

		if (c.will.retain == 1 && c.will_flag == 0) {
			printf("CONNECT, will retain set, willFlag not set\n");
			return m;
		}

		c.clean_session = (contentFlags >> 1) & 1;

		int reservedFlag = (contentFlags & 1);
		if (reservedFlag == 1) {
			printf("CONNECT, reserved flag set to true\n");
			return m;
		}

		c.keepalive = get_short(buf, i);
		i += 2;

		int client_id_lenght = get_short(buf, i);
		i += 2;
		char client_id[client_id_lenght];
		memcpy(client_id, &buf[i], client_id_lenght);
		/*if (utf8_check(client_id) != NULL || strchr(client_id, '\0') != NULL) {
			printf("ClientID contains restricted characters: U+0000, U+D800-U+DFFF");
			return m;
		}*/
		i += client_id_lenght;
		client_id[client_id_lenght] = '\0';
		c.client_id = client_id;

		if (c.will_flag) {
			if ((length_details.length - i) < 2) {
				printf("Invalid encoding will/username/password\n");
				return m;
			}

			int will_topic_lenght = get_short(buf, i);
			i += 2;
			if ((length_details.length - i) < will_topic_lenght
					|| will_topic_lenght < 1) {
				printf("Invalid encoding will/username/password");
				return m;
			}
			char will_topic[will_topic_lenght];
			memcpy(will_topic, &buf[i], will_topic_lenght);
			i += will_topic_lenght;
			/*if (utf8_check(will_topic) != NULL || strchr(will_topic, '\0') != NULL) {
				printf(
						"Will topic contains restricted characters: U+0000, U+D800-U+DFFF");
				return m;
			}*/
			will_topic[will_topic_lenght] = '\0';
			c.will.topic.topic_name = will_topic;
			if ((length_details.length - i) < 2) {
				printf("Invalid encoding will/username/password");
				return m;
			}
			int will_message_lenght = get_short(buf, i);
			i += 2;
			if ((length_details.length - i) < will_message_lenght) {
				printf("Invalid encoding will/username/password");
				return m;
			}
			char will_message[will_message_lenght];
			memcpy(will_message, &buf[i], will_message_lenght);
			i += will_message_lenght;
			will_message[will_message_lenght] = '\0';
			c.will.content = will_message;

		}

		if (user_name_flag) {
			if ((length_details.length - i) < 2) {
				printf("Invalid encoding will/username/password");
				return m;
			}

			int username_lenght = get_short(buf, i);
			i += 2;
			if ((length_details.length - i) < username_lenght) {
				printf("Invalid encoding will/username/password");
				return m;
			}
			char username[username_lenght];
			memcpy(username, &buf[i], username_lenght);
			i += username_lenght;
			/*if (utf8_check(username) != NULL || strchr(username, '\0') != NULL) {
				printf(
						"Username contains restricted characters: U+0000, U+D800-U+DFFF");
				return m;
			}*/
			username[username_lenght] = '\0';
			c.username = username;
		}

		if (user_pass_flag) {
			if ((length_details.length - i) < 2) {
				printf("Invalid encoding will/username/password");
				return m;
			}
			int password_lenght = get_short(buf, i);
			i += 2;
			if ((length_details.length - i) < password_lenght) {
				printf("Invalid encoding will/username/password");
				return m;
			}
			char password[password_lenght];
			memcpy(password, &buf[i], password_lenght);
			i += password_lenght;
			/*if (utf8_check(password) != NULL || strchr(password, '\0') != NULL) {
				printf(
						"Password contains restricted characters: U+0000, U+D800-U+DFFF\n");
				return m;
			}*/
			password[password_lenght] = '\0';
			c.password = password;
		}

		if ((length_details.length - i) > 0) {
			printf("Invalid encoding will/username/password\n");
			return m;
		}

		m->packet = &c;
		break;
	}

	case CONNACK: {
		m->message_type = type;
		struct Connack * ca = malloc (sizeof (struct Connack));
		i++; //skip message size
		ca->session_present = buf[i++];
		if (ca->session_present != 0 && ca->session_present != 1) {
			printf("CONNACK, session-present set to %d\n",
					(ca->session_present & 0xff));
			return m;
		}

		char return_code = buf[i];
		if (return_code < 0 || return_code > 5) {
			printf("Invalid connack code: %d\n", return_code);
			return m;
		}
		ca->return_code = return_code;

		m->packet = ca;
		break;
	}

	case PUBLISH: {
		m->message_type = type;
		struct Publish * p = malloc (sizeof (struct Publish));
		length_details = decode_remaining_length(buf);
		i += length_details.bytes_used;
		fixedHeader &= 0xf;

		int dup = (fixedHeader >> 3) & 1;
		int qos = (fixedHeader & 0x07) >> 1;
		if (qos < 0 || qos > 2) {
			printf("invalid QoS value : %d", qos);
			return m;
		}
		if (dup && qos == AT_MOST_ONCE) {
			printf("PUBLISH, QoS-0 dup flag present");
			return m;
		}

		p->dup = dup;
		p->retain = fixedHeader & 1;

		int topic_lenght = get_short(buf, i);
		i += 2;
		char * topic_name = malloc (sizeof (topic_name)*(topic_lenght+1));
		memcpy(topic_name, &buf[i], topic_lenght);
		i += topic_lenght;

		/*if (utf8_check(topic_name) != NULL || strchr(topic_name, '\0') != NULL) {
			printf("PUBLISH, Topic name contains restricted characters: U+0000, U+D800-U+DFFF\n");
			return m;
		}*/
		topic_name[topic_lenght] = '\0';
		struct Topic t = {.qos = qos, .topic_name = topic_name};
		p->topic = t;

		if (qos != AT_MOST_ONCE) {
			p->packet_id = get_short(buf, i);
			i += 2;
			if (p->packet_id < 0 || p->packet_id > 65535) {
				printf("Invalid PUBLISH packetID encoding");
				return m;
			}

		}
		int data_length = length_details.length - i;
		if (data_length > 0) {
			char * data = malloc (sizeof (data)*(data_length+1));
			memcpy(data, &buf[i], data_length);
			data[data_length] = '\0';
			p->content = malloc (sizeof (p->content)*(data_length+1));
			p->content = data;
		}

		m->packet = malloc(sizeof(struct Publish));
		m->packet = p;
		break;
	}
	case PUBACK: {
		m->message_type = type;
		struct Puback * pa = malloc (sizeof (struct Puback));
		pa->packet_id = get_short(buf, ++i);
		m->packet = pa;
		break;
	}
	case PUBREC: {
		m->message_type = type;
		struct Pubrec * pr = malloc (sizeof (struct Pubrec));
		pr->packet_id = get_short(buf, ++i);
		m->packet = pr;
		break;
	}
	case PUBREL: {
		m->message_type = type;
		struct Pubrel * pl = malloc (sizeof (struct Pubrel));
		pl->packet_id = get_short(buf, ++i);
		m->packet = pl;
		break;
	}
	case PUBCOMP: {
		m->message_type = type;
		struct Pubcomp * pc = malloc (sizeof (struct Pubrel));
		pc->packet_id = get_short(buf, ++i);
		m->packet = pc;
		break;
	}
	case SUBSCRIBE: {
		m->message_type = type;
		struct Subscribe s;
		length_details = decode_remaining_length(buf);
		i += length_details.bytes_used;
		s.packet_id = get_short(buf, i);
		i += 2;
		int topics_size = length_details.length - i;
		int topics_index = 0;
		int topic_filter_length = 0;
		while (i < topics_size) {
			topic_filter_length = get_short(buf, i);
			i += 2;
			char topic_filter[topic_filter_length];
			memcpy(topic_filter, &buf[i], topic_filter_length);
			i += topic_filter_length;

			enum QoS qos = buf[i++];
			if (qos < 0 || qos > 2) {
				printf("Subscribe qos must be in range from 0 to 2: %d", qos);
				return m;
			}
			/*if (utf8_check(topic_filter) != NULL || strchr(topic_filter, '\0') != NULL) {
				printf(
						"Subscribe topic contains one or more restricted characters: U+0000, U+D000-U+DFFF");
				return m;
			}*/
			topic_filter[topic_filter_length] = '\0';
			struct Topic t = { .topic_name = topic_filter, .qos = qos };
			s.topics[topics_index] = t;
			topics_index++;
		}
		if (topics_index) {
			printf("Subscribe with 0 topics");
			return m;
		}
		s.topics_number = topics_index;
		m->packet = &s;
		break;
	}
	case SUBACK: {
		m->message_type = type;
		struct Suback * sa = malloc (sizeof (struct Suback));

		length_details = decode_remaining_length(buf);
		i += length_details.bytes_used;
		sa->packet_id = get_short(buf, i);
		i += 2;
		int codes_size = length_details.length - i;
		sa->codes = malloc (sizeof(enum SubackCode)*codes_size);
		int code_index = 0;

		while (code_index < codes_size) {
			enum SubackCode code = buf[i++];
			if (code != FAILURE
					&& (code < ACCEPTED_QOS0 || code > ACCEPTED_QOS2)) {
				printf("Invalid suback code: %d \n", code);
				return m;
			}
			sa->codes[code_index++] = code;
		}

		if (code_index == 0) {
			printf("Suback with 0 return-codes");
			return m;
		}
		sa->codes_number = code_index;
		m->packet = sa;
		break;
	}
	case UNSUBSCRIBE: {
		m->message_type = type;
		struct Unsubscribe us;
		length_details = decode_remaining_length(buf);
		i += length_details.bytes_used;
		us.packet_id = get_short(buf, i);
		i += 2;
		int topics_size = length_details.length - i;
		int topics_index = 0;
		int topic_length = 0;
		while (i < topics_size) {
			topic_length = get_short(buf, i);
			i += 2;
			char topic[topic_length];
			memcpy(topic, &buf[i], topic_length);
			i += topic_length;

			if (utf8_check(topic) != NULL /*|| strchr(topic, '\0') != NULL*/) {
				printf("Unsubscribe topic contains one or more restricted characters: U+0000, U+D000-U+DFFF");
				return m;
			}
			topic[topic_length] = '\0';
			us.topics[topics_index].topic_name = topic;
			topics_index++;
		}
		if (topics_index == 0) {
			printf("Unsubscribe with 0 topics");
			return m;
		}
		us.topics_number = topics_index;
		m->packet = &us;
		break;
	}
	case UNSUBACK: {
		m->message_type = type;
		struct Unsuback * ua = malloc (sizeof (struct Unsuback));
		ua->packet_id = get_short(buf, ++i);
		m->packet = ua;
		break;
	}
	case PINGREQ: {
		m->message_type = type;
		break;
	}
	case PINGRESP: {
		m->message_type = type;
		break;
	}
	case DISCONNECT: {
		m->message_type = type;
		break;
	}
	}

/*	if (length_details.length > i) {
		printf("Unexpected bytes in content\n");

	}
*/
	//TODO??? check is correct  headers length  length_details

	return m;
}

struct LengthDetails decode_remaining_length(char buf[]) {
	struct LengthDetails length_details;
	int i = 1;
	int length = 0;
	int multiplier = 1;
	int bytes_used = 0;
	char enc = 0;
	do {
		if (multiplier > 128 * 128 * 128) {
			printf("Encoded length exceeds maximum of 268435455 bytes");
			length_details.bytes_used = -1;
			length_details.length = -1;
			return length_details;
		}

		enc = buf[i];
		length += (enc & 0x7f) * multiplier;
		multiplier *= 128;
		bytes_used++;
	} while ((enc & 0x80) != 0);
	length_details.bytes_used = bytes_used;
	length_details.length = length+2;

	return length_details;

}
