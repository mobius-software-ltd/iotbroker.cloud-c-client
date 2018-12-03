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
#include <string.h>
#include <jansson.h>
#include "base64_parser.h"
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

char * ws_encode(struct Message * message) {

	char* s = NULL;
	json_t *root = json_object();

	switch (message->message_type) {
	case CONNECT: {
		struct Connect c = *(struct Connect*) message->packet;

		json_object_set_new( root, "packet", json_integer( CONNECT ) );
		json_object_set_new( root, "protocolLevel", json_integer( 4 ) );
		json_object_set_new( root, "protocolName", json_string("MQTT") );
		json_object_set_new( root, "username", json_string(c.username) );
		json_object_set_new( root, "password", json_string(c.password) );
		json_object_set_new( root, "clientID", json_string(c.client_id) );
		if(c.clean_session)
			json_object_set_new( root, "cleanSession", json_true() );
		else
			json_object_set_new( root, "cleanSession", json_false() );
		json_object_set_new( root, "keepalive", json_integer( c.keepalive ) );
		json_object_set_new( root, "usernameFlag", json_true() );
		json_object_set_new( root, "passwordFlag", json_true() );
		if(c.will_flag == 0) {
			json_object_set_new( root, "willFlag", json_false() );
			json_object_set_new( root, "will", json_null() );
		} else {
			json_object_set_new( root, "willFlag", json_true() );
			json_t *topic = json_object();
			json_object_set_new( topic, "name", json_string(c.will.topic.topic_name) );
			json_object_set_new( topic, "qos", json_integer(c.will.topic.qos) );
			json_t *will = json_object();
			json_object_set_new( will, "topic", topic );
			size_t *output_length = malloc(sizeof(output_length));
			size_t len = strlen(c.will.content);
			char * content = malloc(len*sizeof(content));
			content = base64_encode((const unsigned char*)c.will.content, len, output_length);
			json_object_set_new( will, "content", json_stringn(content, *output_length) );
			if(c.will.retain)
				json_object_set_new( will, "retain", json_true() );
			else
				json_object_set_new( will, "retain", json_false() );
			json_object_set_new( root, "will", will );
		}

		break;
	}
	case CONNACK: {
		struct Connack ca = *(struct Connack*) message->packet;
		json_object_set_new( root, "packet", json_integer( CONNACK ) );
		json_object_set_new( root, "returnCode", json_integer( ca.return_code ) );
		if(ca.session_present)
			json_object_set_new( root, "sessionPresent", json_true() );
		else
			json_object_set_new( root, "sessionPresent", json_false() );
		break;
	}
	case PUBLISH: {
		struct Publish p = *(struct Publish*) message->packet;
		json_object_set_new( root, "packet", json_integer( PUBLISH ) );
		if(p.packet_id != 0)
			json_object_set_new( root, "packetID", json_integer( p.packet_id ) );
		json_t *topic = json_object();
		json_object_set_new( topic, "name", json_string(p.topic.topic_name) );
		json_object_set_new( topic, "qos", json_integer(p.topic.qos) );
		json_object_set_new( root, "topic", topic );
		//content
		size_t *output_length = malloc(sizeof(output_length));
		size_t len = strlen(p.content);
		char * content = malloc(len*sizeof(content));
		content = base64_encode((const unsigned char*)p.content, len, output_length);
		json_object_set_new( root, "content", json_stringn(content, *output_length) );
		if(p.retain)
			json_object_set_new( root, "retain", json_true() );
		else
			json_object_set_new( root, "retain", json_false() );
		if(p.dup)
			json_object_set_new( root, "dup", json_true() );
		else
			json_object_set_new( root, "dup", json_false() );

		break;
	}
	case PUBACK: {
		struct Puback pa = *(struct Puback*) message->packet;
		json_object_set_new( root, "packet", json_integer( PUBACK ) );
		json_object_set_new( root, "packetID", json_integer( pa.packet_id ) );
		break;
	}
	case PUBREC: {
		struct Pubrec pr = *(struct Pubrec*) message->packet;
		json_object_set_new( root, "packet", json_integer( PUBREC ) );
		json_object_set_new( root, "packetID", json_integer( pr.packet_id ) );

		break;
	}
	case PUBREL: {
		struct Pubrel pl = *(struct Pubrel*) message->packet;
		json_object_set_new( root, "packet", json_integer( PUBREL ) );
		json_object_set_new( root, "packetID", json_integer( pl.packet_id ) );
		break;
	}
	case PUBCOMP: {
		struct Pubcomp pc = *(struct Pubcomp*) message->packet;
		json_object_set_new( root, "packet", json_integer( PUBCOMP ) );
		json_object_set_new( root, "packetID", json_integer( pc.packet_id ) );
		break;
	}
	case SUBSCRIBE: {
		struct Subscribe s = *(struct Subscribe*) message->packet;
		json_object_set_new( root, "packet", json_integer( SUBSCRIBE ) );
		json_object_set_new( root, "packetID", json_integer( s.packet_id ) );
		json_t *topics = json_array();
		for(int i = 0; i < s.topics_number; i++) {
			json_t *topic = json_object();
			json_object_set_new( topic, "name", json_stringn(s.topics[i].topic_name, strlen(s.topics[i].topic_name)) );
			json_object_set_new( topic, "qos", json_integer(s.topics[i].qos) );
			json_array_append_new(topics, topic);
		}
		json_object_set_new( root, "topics", topics );

		break;
	}
	case SUBACK: {
		struct Suback sa = *(struct Suback*) message->packet;
		json_object_set_new( root, "packet", json_integer( SUBACK ) );
		json_object_set_new( root, "packetID", json_integer( sa.packet_id ) );
		json_t *codes = json_array();
		for(int i = 0; i < sa.codes_number; i++) {
			json_array_append_new(codes, json_integer( sa.codes[i] ));
		}
		json_object_set_new( root, "returnCodes", codes );
		break;
	}
	case UNSUBSCRIBE: {
		struct Unsubscribe us = *(struct Unsubscribe*) message->packet;
		json_object_set_new( root, "packet", json_integer( UNSUBSCRIBE ) );
		json_object_set_new( root, "packetID", json_integer( us.packet_id ) );
		json_t *topics = json_array();
		for(int i = 0; i < us.topics_number; i++) {
			json_array_append_new(topics, json_string(us.topics[i].topic_name));
		}
		json_object_set_new( root, "topics", topics );
		break;
	}
	case UNSUBACK: {
		struct Unsuback usa = *(struct Unsuback*) message->packet;
		json_object_set_new( root, "packet", json_integer( UNSUBACK ) );
		json_object_set_new( root, "packetID", json_integer( usa.packet_id ) );
		break;
	}
	case DISCONNECT:
		json_object_set_new( root, "packet", json_integer( DISCONNECT ) );
		break;
	case PINGREQ:
		json_object_set_new( root, "packet", json_integer( PINGREQ ) );
		break;
	case PINGRESP:
		json_object_set_new( root, "packet", json_integer( PINGRESP ) );
		break;
	}
	s = json_dumps(root, JSON_COMPACT);

	return s;
}

struct Message * ws_decode(char * data) {

	json_t *j_message;
	json_error_t error;
	j_message = json_loads(data, 0, &error);
	if(!j_message)
	{
		fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
		return NULL;
	}

	struct Message * m = malloc (sizeof (struct Message));


	enum MessageType type = json_integer_value(json_object_get(j_message, "packet"));
	switch (type) {
	case CONNECT: {
		m->message_type = type;
		struct Connect * c = malloc (sizeof (struct Connect));
		c->protocol_level = json_integer_value(json_object_get(j_message, "protocolLevel"));
		c->username = json_string_value(json_object_get(j_message, "username"));
		c->password = json_string_value(json_object_get(j_message, "password"));
		c->client_id = json_string_value(json_object_get(j_message, "clientID"));
		c->keepalive = json_integer_value(json_object_get(j_message, "keepalive"));
		int clean_session = 0;
		json_t *clean_session_j = json_object_get(j_message, "cleanSession");
		if(json_typeof(clean_session_j) == JSON_TRUE) {
			clean_session = 1;
		}
		c->clean_session = clean_session;
		int will_flag = 0;
		json_t *will_flag_j = json_object_get(j_message, "willFlag");
		if(json_typeof(will_flag_j) == JSON_TRUE) {
			will_flag = 1;
		}
		c->will_flag = will_flag;
		json_t *will_j = json_object_get(j_message, "will");

		const char * content_64 = json_string_value(json_object_get(will_j, "content"));
		size_t *output_length = malloc(sizeof(output_length));
		size_t len = strlen(content_64);
		unsigned char * content = base64_decode(content_64, len, output_length);
		content[*output_length] = '\0';
		c->will.content = malloc(*output_length*sizeof(content));
		strcpy((char*)c->will.content,(char*)content);

		int retain = 0;
		json_t *retain_j = json_object_get(will_j, "retain");
		if(json_typeof(retain_j) == JSON_TRUE) {
			retain = 1;
		}
		c->will.retain = retain;
		json_t *topic_j = json_object_get(j_message, "topic");
		c->will.topic.topic_name = json_string_value(json_object_get(topic_j, "name"));
		c->will.topic.qos = json_integer_value(json_object_get(topic_j, "qos"));

		m->packet = c;
		break;
	}
	case CONNACK: {
		m->message_type = type;
		struct Connack * ca = malloc (sizeof (struct Connack));
		int session_present = 0;
		json_t *retain_j = json_object_get(j_message, "sessionPresent");
		if(json_typeof(retain_j) == JSON_TRUE) {
			session_present = 1;
		}
		ca->session_present = session_present;

		char return_code = json_integer_value(json_object_get(j_message, "returnCode"));
		if (return_code < 0 || return_code > 5) {
			printf("Invalid connack code: %d\n", return_code);
			return NULL;
		}
		ca->return_code = return_code;
		m->packet = ca;
		break;
	}

	case PUBLISH: {
		m->message_type = type;
		struct Publish * p = malloc (sizeof (struct Publish));

		const char * content_64 = json_string_value(json_object_get(j_message, "content"));
		size_t *output_length = malloc(sizeof(output_length));
		size_t len = strlen(content_64);
		unsigned char * content = base64_decode(content_64, len, output_length);
		content[*output_length] = '\0';
		p->content = malloc(*output_length*sizeof(content));
		strcpy((char*)p->content,(char*)content);

		int retain = 0;
		int dup = 0;
		json_t *retain_j = json_object_get(j_message, "retain");
		if(json_typeof(retain_j) == JSON_TRUE) {
			retain = 1;
		}
		json_t *dup_j = json_object_get(j_message, "dup");
		if(json_typeof(dup_j) == JSON_TRUE) {
			dup = 1;
		}

		json_t *topic = json_object_get(j_message, "topic");
		const char * topic_name = json_string_value(json_object_get(topic, "name"));
		enum QoS qos = json_integer_value(json_object_get(topic, "qos"));
		int packet_id = json_integer_value(json_object_get(j_message, "packetID"));

		if (qos < 0 || qos > 2) {
			printf("invalid QoS value : %d", qos);
			return NULL;
		}
		if (dup && qos == AT_MOST_ONCE) {
			printf("PUBLISH, QoS-0 dup flag present");
			return NULL;
		}

		p->dup = dup;
		p->retain = retain;

		struct Topic t = {.qos = qos, .topic_name = topic_name};
		p->topic = t;
		if (qos != AT_MOST_ONCE) {
			p->packet_id = packet_id;

			if (p->packet_id < 0 || p->packet_id > 65535) {
				printf("Invalid PUBLISH packetID encoding");
				return NULL;
			}
		}

		m->packet = malloc(sizeof(struct Publish));
		m->packet = p;
		break;
	}
	case PUBACK: {
		m->message_type = type;
		struct Puback * pa = malloc (sizeof (struct Puback));
		pa->packet_id = json_integer_value(json_object_get(j_message, "packetID"));
		m->packet = pa;
		break;
	}
	case PUBREC: {
		m->message_type = type;
		struct Pubrec * pr = malloc (sizeof (struct Pubrec));
		int packet_id = json_integer_value(json_object_get(j_message, "packetID"));
		pr->packet_id = packet_id;
		m->packet = pr;
		break;
	}
	case PUBREL: {
		m->message_type = type;
		struct Pubrel * pl = malloc (sizeof (struct Pubrel));
		pl->packet_id = json_integer_value(json_object_get(j_message, "packetID"));
		m->packet = pl;
		break;
	}
	case PUBCOMP: {
		m->message_type = type;
		struct Pubcomp * pc = malloc (sizeof (struct Pubrel));
		pc->packet_id = json_integer_value(json_object_get(j_message, "packetID"));
		m->packet = pc;
		break;
	}
	case SUBSCRIBE: {
		m->message_type = type;
		struct Subscribe * s = malloc (sizeof (struct Subscribe));
		s->packet_id = json_integer_value(json_object_get(j_message, "packetID"));
		json_t *j_topics = json_object_get(j_message, "topics");
		size_t topics_size = json_array_size(j_topics);
		for (int i = 0; i < topics_size; i++) {
			json_t * entry = json_array_get(j_topics, i);
			s->topics[i].topic_name = json_string_value(json_object_get(entry, "name"));
			s->topics[i].qos = json_integer_value(json_object_get(entry, "qos"));
		}
		s->topics_number = topics_size;

		m->packet = s;
		break;
	}
	case SUBACK: {
		m->message_type = type;
		struct Suback * sa = malloc (sizeof (struct Suback));
		sa->packet_id = json_integer_value(json_object_get(j_message, "packetID"));
		json_t *j_codes = json_object_get(j_message, "returnCodes");
		size_t codes_size = json_array_size(j_codes);
		sa->codes = malloc(codes_size * sizeof(sa->codes));
		int code_index = 0;
		for (int i = 0; i < codes_size; i++) {
			int code = json_integer_value(json_array_get(j_codes, i));
			if (code != FAILURE && (code < ACCEPTED_QOS0 || code > ACCEPTED_QOS2)) {
				printf("Invalid suback code: %d \n", code);
				return NULL;
			}
			sa->codes[i] = code;
			code_index++;
		}

		if (code_index == 0) {
			printf("Suback with 0 return-codes\n");
			return NULL;
		}
		sa->codes_number = code_index;
		m->packet = sa;
		break;
	}
	case UNSUBSCRIBE: {
		m->message_type = type;
		struct Unsubscribe * us = malloc (sizeof (struct Unsubscribe));
		us->packet_id = json_integer_value(json_object_get(j_message, "packetID"));

		json_t *j_topics = json_object_get(j_message, "topics");
		size_t topics_size = json_array_size(j_topics);
		for (int i = 0; i < topics_size; i++) {
			us->topics[i].topic_name = json_string_value(json_array_get(j_topics, i));
		}

		m->packet = us;
		break;
	}
	case UNSUBACK: {
		m->message_type = type;
		struct Unsuback * ua = malloc (sizeof (struct Unsuback));
		int packet_id = json_integer_value(json_object_get(j_message, "packetID"));
		ua->packet_id = packet_id;
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
	free(j_message);
	return m;
}
