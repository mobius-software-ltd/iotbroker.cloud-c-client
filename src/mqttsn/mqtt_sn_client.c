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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../gui/main_w.h"
#include "../dal/cdal.h"
#include "../account.h"
#include "../mqtt_listener.h"
#include "../tcp_listener.h"
#include "../net/tcp_client.h"
#include "../net/dtls_client.h"
#include "../mqttsn/mqtt_sn_timers.h"
#include "packets/sn_message.h"
#include "packets/sn_connect.h"
#include "packets/sn_connack.h"
#include "packets/sn_subscribe.h"
#include "packets/sn_suback.h"
#include "packets/sn_unsubscribe.h"
#include "packets/sn_unsuback.h"
#include "packets/sn_publish.h"
#include "packets/sn_puback.h"
#include "packets/sn_pubrel.h"
#include "packets/sn_pubrec.h"
#include "packets/sn_pubcomp.h"
#include "packets/sn_register.h"
#include "packets/sn_reg_ack.h"
#include "packets/sn_disconnect.h"
#include "packets/sn_pingreq.h"
#include "packets/sn_topic.h"
#include "packets/sn_will_topic.h"
#include "packets/sn_will_message.h"
#include "packets/sn_return_code.h"
#include "sn_parser.h"

#define SN_PROTOCOL_ID 0x01
#define UDP_PROTOCOL 2

struct TcpListener * tcp_listener = NULL;
static struct MqttListener * mqtt_listener = NULL;
struct Account * account = NULL;

static int current_packet_number = 0;
static int delay_in_seconds = 0;

void process_sn_rx(char * data, int length);
void send_sn_connect(struct Account * acc);
void send_sn_subscribe(const char * topic_name, int qos);
void send_sn_unsubscribe(const char * topic_name);
void send_sn_disconnect(void);
void send_sn_publish(struct SnPublish * sn_publish);
void send_sn_message(const char * content, const char * topic_name, int qos, int retain, int dup);
static void send_sn_register(const char * topic_name);

void sn_encode_and_fire(struct SnMessage * sn_message) {
	//encode
	int length = sn_get_length(sn_message);
	int total_length = length;
	char * buf = sn_encode(sn_message, length);
	//send
	if(account->is_secure)
		dtls_fire(buf,total_length);
	else
		write_to_net(buf,total_length);
}


int init_mqtt_sn_client(struct Account * acc, struct MqttListener * listener) {

	account = acc;
	mqtt_listener = listener;
	mqtt_listener->send_connect = send_sn_connect;
	mqtt_listener->send_sub = send_sn_subscribe;
	mqtt_listener->send_message = send_sn_message;
	mqtt_listener->send_disconnect = send_sn_disconnect;
	mqtt_listener->send_unsubscribe = send_sn_unsubscribe;

	const char * host = acc->server_host;
	int port = acc->server_port;
	tcp_listener = malloc (sizeof (struct TcpListener));
	tcp_listener->prd_pt = process_sn_rx;
	int is_successful = 0;
	if(!acc->is_secure)
		init_net_service(host, port, UDP_PROTOCOL, tcp_listener);
	else
		init_dtls(host, port, tcp_listener, acc->certificate, acc->certificate_password);

	if (is_successful >= 0) {
		printf("MQTT-SN client successfully connected \n");
	}
	else
	{
		printf("MQTT-SN client NOT connected \n");
	}
	return is_successful;

}

void send_sn_connect(struct Account * acc) {
	//prepare connect packet
	delay_in_seconds = acc->keep_alive;
	account = acc;

	struct SnConnect * sn_connect_packet = malloc (sizeof (struct SnConnect));
	sn_connect_packet->client_id = account->client_id;
	sn_connect_packet->clean_session = account->clean_session;
	sn_connect_packet->protocol_id = SN_PROTOCOL_ID;
	if(account->will != NULL && account->will_topic != NULL)
		sn_connect_packet->will_present = 1;
	else
		sn_connect_packet->will_present = 0;
	sn_connect_packet->duration = account->keep_alive;

	struct SnMessage * sn_message = malloc (sizeof (struct SnMessage));
	sn_message->message_type = SN_CONNECT;
	sn_message->packet = sn_connect_packet;
	sn_encode_and_fire(sn_message);
	sn_start_connect_timer();

	sn_add_message_in_map(sn_message);

}

void send_sn_subscribe(const char * topic_name, int qos) {

	struct SnSubscribe * sn_subscribe = malloc (sizeof (struct SnSubscribe));
	struct SnTopic * sn_topic = malloc (sizeof (struct SnTopic));
	sn_topic->qos = qos;
	unsigned short topic_id = sn_get_topic_id_from_map(topic_name);
	if(topic_id == 0) {
		sn_topic->value = malloc (sizeof (char)*strlen(topic_name));
		sn_topic->value = topic_name;
		sn_topic->topic_type = NAMED;
	} else {
		sn_topic->topic_type = ID;
		sn_topic->id = topic_id;
	}

	sn_subscribe->message_id = ++current_packet_number;
	sn_subscribe->dup = 0;
	sn_subscribe->topic = sn_topic;

	struct SnMessage * sn_message = malloc (sizeof (struct SnMessage));
	sn_message->message_type = SN_SUBSCRIBE;
	sn_message->packet = sn_subscribe;

	sn_add_message_in_map(sn_message);

	sn_encode_and_fire(sn_message);
}

void send_sn_message(const char * content, const char * topic_name, int qos, int retain, int dup) {

	unsigned short topic_id = sn_get_topic_id_from_map(topic_name);

	struct SnPublish * sn_publish = malloc (sizeof (struct SnPublish));
	sn_publish->topic.topic_type = ID;
	sn_publish->is_incoming = 0;
	sn_publish->topic.id = topic_id;
	sn_publish->data = content;
	sn_publish->topic.qos = qos;
	sn_publish->topic.value = topic_name;
	sn_publish->dup = dup;
	sn_publish->retain = retain;
	sn_publish->message_id = ++current_packet_number;

	if(topic_id == 0) {
		struct SnMessage * sn_message = malloc (sizeof (struct SnMessage));
		sn_message->message_type = SN_PUBLISH;
		sn_message->packet = sn_publish;
		sn_add_message_in_map(sn_message);
		send_sn_register(topic_name);
	} else {
		//send publish
		send_sn_publish(sn_publish);

	}

}

static void send_sn_register(const char * topic_name) {
	struct SnRegister * sn_register = malloc (sizeof (struct SnRegister));
	sn_register->topic_id = 0;
	sn_register->msg_id = ++current_packet_number;
	sn_register->topic_name = topic_name;
	struct SnMessage * sn_message = malloc (sizeof (struct SnMessage));
	sn_message->message_type = SN_REGISTER;
	sn_message->packet = sn_register;

	sn_add_message_in_map(sn_message);

	sn_encode_and_fire(sn_message);
}

void send_sn_publish(struct SnPublish * sn_publish) {

	sn_publish->is_incoming = 0;
	sn_publish->message_id = ++current_packet_number;
	struct SnMessage * sn_message = malloc (sizeof (struct SnMessage));
	sn_message->message_type = SN_PUBLISH;
	sn_message->packet = sn_publish;

	sn_encode_and_fire(sn_message);

	if(sn_publish->topic.qos == SN_AT_MOST_ONCE) {
		//store in db
		save_message(sn_publish->data, sn_publish->topic.value, sn_publish->topic.qos, sn_publish->retain, sn_publish->dup, 0);
		//update gui
		update_messages_window(sn_publish->data, sn_publish->topic.value, sn_publish->topic.qos, sn_publish->retain, sn_publish->dup, 0);
	} else {
		sn_add_message_in_map(sn_message);
	}
}

void send_sn_unsubscribe(const char * topic_name) {

	struct SnUnsubscribe * sn_unsubscribe = malloc (sizeof (struct SnUnsubscribe));
	sn_unsubscribe->topic = malloc (sizeof (struct SnTopic));

	unsigned short topic_id = sn_get_topic_id_from_map(topic_name);
	if(topic_id == 0) {
		sn_unsubscribe->topic->value = malloc (sizeof (char)*strlen(topic_name));
		sn_unsubscribe->topic->value = topic_name;
		sn_unsubscribe->topic->topic_type = NAMED;
	} else {
		sn_unsubscribe->topic->topic_type = ID;
		sn_unsubscribe->topic->id = topic_id;
	}

	sn_unsubscribe->message_id = ++current_packet_number;
	struct SnMessage * sn_message = malloc (sizeof (struct SnMessage));
	sn_message->message_type = SN_UNSUBSCRIBE;
	sn_message->packet = sn_unsubscribe;
	sn_add_message_in_map(sn_message);
	sn_encode_and_fire(sn_message);

}

static void send_will_topic() {

	struct SnWillTopic * sn_will_topic = malloc (sizeof (struct SnWillTopic));
	struct SnTopic * sn_topic = malloc (sizeof (struct SnTopic));
	sn_topic->value = account->will_topic;
	sn_topic->qos = account->qos;
	sn_will_topic->retain = account->is_retain;
	sn_will_topic->topic = sn_topic;
	struct SnMessage * sn_message = malloc (sizeof (struct SnMessage));
	sn_message->packet = sn_will_topic;
	sn_message->message_type = SN_WILL_TOPIC;
	sn_encode_and_fire(sn_message);
}

static void send_will_message() {

	struct SnWillMessage * sn_will_message = malloc (sizeof (struct SnWillMessage));
	sn_will_message->message = account->will;
	struct SnMessage * sn_message = malloc (sizeof (struct SnMessage));
	sn_message->packet = sn_will_message;
	sn_message->message_type = SN_WILL_MSG;
	sn_encode_and_fire(sn_message);
}

static void send_sn_pubrel (unsigned short message_id) {

	struct SnPubrel * sn_pubrel = malloc (sizeof (struct SnPubrel));
	sn_pubrel->message_id = message_id;
	struct SnMessage * sn_message = malloc (sizeof (struct SnMessage));
	sn_message->message_type = SN_PUBREL;
	sn_message->packet = sn_pubrel;
	sn_encode_and_fire(sn_message);
}

static void send_sn_puback(int message_id, unsigned short topic_id) {

	struct SnPuback * sn_paback = malloc (sizeof (struct SnPuback));
	sn_paback->message_id = message_id;
	sn_paback->code = SN_ACCEPTED;
	sn_paback->topic_id = topic_id;
	struct SnMessage * sn_message = malloc (sizeof (struct SnMessage));
	sn_message->message_type = SN_PUBACK;
	sn_message->packet = sn_paback;
	sn_encode_and_fire(sn_message);
}

static void send_sn_pubrec(int message_id) {

	struct SnPubrec * sn_pubrec = malloc (sizeof (struct SnPubrec));
	sn_pubrec->message_id = message_id;
	struct SnMessage * sn_message = malloc (sizeof (struct SnMessage));
	sn_message->message_type = SN_PUBREC;
	sn_message->packet = sn_pubrec;
	sn_encode_and_fire(sn_message);
}

static void send_sn_pubcomp(int message_id) {

	struct SnPubcomp * sn_pubcomp = malloc (sizeof (struct SnPubcomp));
	sn_pubcomp->message_id = message_id;
	struct SnMessage * sn_message = malloc (sizeof (struct SnMessage));
	sn_message->message_type = SN_PUBCOMP;
	sn_message->packet = sn_pubcomp;
	sn_encode_and_fire(sn_message);

}

static void send_sn_regack(int message_id, unsigned short topic_id) {

	struct SnRegAck * sn_regack = malloc (sizeof (struct SnRegAck));
	sn_regack->code = SN_ACCEPTED;
	sn_regack->message_id = message_id;
	sn_regack->topic_id = topic_id;

	struct SnMessage * sn_message = malloc (sizeof (struct SnMessage));
	sn_message->message_type = SN_REGACK;
	sn_message->packet = sn_regack;
	sn_encode_and_fire(sn_message);
}

void sn_send_ping() {

	struct SnPingreq * sn_ping_req = malloc (sizeof (struct SnPingreq));
	sn_ping_req->client_id = account->client_id;
	struct SnMessage * sn_message = malloc (sizeof (struct SnMessage));
	sn_message->message_type = SN_PINGREQ;
	sn_message->packet = sn_ping_req;
	sn_encode_and_fire(sn_message);
}

void send_sn_disconnect() {

	sn_stop_all_timers();

	struct SnDisconnect * sn_disconnect = malloc (sizeof (struct SnDisconnect));
	sn_disconnect->duration = account->keep_alive;
	struct SnMessage * sn_message = malloc (sizeof (struct SnMessage));
	sn_message->message_type = SN_DISCONNECT;
	sn_message->packet = sn_disconnect;
	sn_encode_and_fire(sn_message);

	if(account->is_secure)
		stop_dtls_net_service();
	else
		stop_net_service();

}

void process_sn_rx(char * data, int length) {

	struct SnMessage * sn_message = sn_decode(data);

	switch(sn_message->message_type) {
	        case SN_ADVERTISE:
	        case SN_SEARCHGW:
	        case SN_GWINFO:
	        case SN_CONNECT:
	        case SN_WILL_TOPIC:
	        case SN_WILL_MSG:
	        case SN_SUBSCRIBE:
	        case SN_UNSUBSCRIBE:
	        case SN_PINGREQ:
	        case SN_WILL_TOPIC_UPD:
	        case SN_WILL_TOPIC_RESP:
	        case SN_WILL_MSG_UPD:
	        case SN_WILL_MSG_RESP:
	        case SN_ENCAPSULATED:
	        {
	            printf("Received unacceptable packet %i \n", sn_message->message_type);
	            break;
	        }
	        case SN_CONNACK:
	        {
	        	sn_stop_connect_timer();
	        	sn_remove_message_from_map(0);
	        	struct SnConnack * sn_ca = (struct SnConnack*) sn_message->packet;
	        	if(sn_ca->return_code == SN_ACCEPTED) {
	        		//open new window
	        		mqtt_listener->cs_pt();
	        		//start ping timer
	        		start_mqtt_sn_ping_timer((unsigned int)delay_in_seconds);
	        		//sn_start_message_timer();

	        	} else {
	        		//connection unsuccessful
	        		printf("MQTT-SN client have got CONNACK with error : %i \n", sn_ca->return_code);
	        	}
	        	break;
	        }
	        case SN_WILL_TOPIC_REQ:
	        {
	        	send_will_topic();
	        	break;
	        }
	        case SN_WILL_MSG_REQ:
	        {
	        	send_will_message();
	        	break;
	        }
	        case SN_REGISTER:
	        {
	        	struct SnRegister * sn_register = (struct SnRegister*) sn_message->packet;
	        	sn_add_topic_id_in_map(sn_register->topic_name,sn_register->topic_id);
	        	sn_add_topic_name_in_map(sn_register->topic_id,sn_register->topic_name);
	        	send_sn_regack(sn_register->msg_id, sn_register->topic_id);
	        	break;
	        }
	        case SN_REGACK:
	        {
	        	struct SnRegAck * sn_reg_ack = (struct SnRegAck*) sn_message->packet;
	        	struct SnMessage * sn_message = sn_get_message_from_map(sn_reg_ack->message_id - 1);
	        	if(sn_message == NULL) {
	        		printf("Cannot get SnPublish after SnRegAck for id : %i \n", sn_reg_ack->message_id - 1);
	        		break;
	        	}
	        	struct SnPublish * sn_publish = (struct SnPublish*) sn_message->packet;

	        	if(sn_reg_ack->code == SN_ACCEPTED) {
	        		//store in maps <topic_name, tipic_id> and viceversa
	        		sn_add_topic_id_in_map(sn_publish->topic.value,sn_reg_ack->topic_id);
	        		sn_add_topic_name_in_map(sn_reg_ack->topic_id,sn_publish->topic.value);

	        		sn_publish->topic.id = sn_reg_ack->topic_id;
	        		send_sn_publish(sn_publish);
	        	} else {
	        		printf("REGISTER UNSUCCESFUL\n");
	        	}
	        	//clear maps of messages
	        	sn_remove_message_from_map(sn_reg_ack->message_id-1);
	        	sn_remove_message_from_map(sn_reg_ack->message_id);

				break;

	        }
	        case SN_PUBLISH:
	        {
	        	struct SnPublish * sn_p = (struct SnPublish*) sn_message->packet;
	        	char * topic_name = sn_get_topic_name_from_map(sn_p->topic.id);
	        	mqtt_listener->get_pub_pt(sn_p->data, topic_name, sn_p->topic.qos, sn_p->retain, sn_p->dup);
	        	if(sn_p->topic.qos == SN_AT_LEAST_ONCE) {
	        		send_sn_puback(sn_p->message_id, sn_p->topic.id);
	        	} else if (sn_p->topic.qos == SN_EXACTLY_ONCE) {
	        		send_sn_pubrec(sn_p->message_id);
	        	}
				break;
	        }
	        case SN_PUBACK:
	        {
	        	struct SnPuback * sn_pub_ack = (struct SnPuback*) sn_message->packet;
	        	printf("will get message with id : %i \n",sn_pub_ack->message_id);
	        	struct SnMessage * sn_message = sn_get_message_from_map(sn_pub_ack->message_id);
	        	if(sn_message == NULL) {
	        		printf("Cannot get SnPublish after SnPuback for id : %i \n", sn_pub_ack->message_id);
	        		break;
	        	}
	        	struct SnPublish * sn_p = (struct SnPublish*) sn_message->packet;

	        	save_message(sn_p->data, sn_p->topic.value, sn_p->topic.qos, sn_p->retain, sn_p->dup, sn_p->is_incoming);
	        	update_messages_window (sn_p->data, sn_p->topic.value, sn_p->topic.qos, sn_p->retain, sn_p->dup, sn_p->is_incoming);
	        	sn_remove_message_from_map(sn_pub_ack->message_id);
	        	break;
	        }
	        case SN_PUBCOMP:
	        {
	        	struct SnPubcomp * sn_pub_comp = (struct SnPubcomp*) sn_message->packet;
	        	struct SnMessage * message = sn_get_message_from_map(sn_pub_comp->message_id);
	        	if(sn_message == NULL) {
	        		printf("Cannot get SnPublish after SnPubcomp for id : %i \n", sn_pub_comp->message_id);
	        		break;
	        	}
	        	struct SnPublish * sn_p = (struct SnPublish*) message->packet;
	        	save_message(sn_p->data, sn_p->topic.value, sn_p->topic.qos, sn_p->retain, sn_p->dup, sn_p->is_incoming);
	        	update_messages_window (sn_p->data, sn_p->topic.value, sn_p->topic.qos, sn_p->retain, sn_p->dup, sn_p->is_incoming);
	        	sn_remove_message_from_map(sn_p->message_id);
				break;
	        }
	        case SN_PUBREC:
	        {
	        	struct SnPubrec * sn_pubrec = (struct SnPubrec*) sn_message->packet;
	        	send_sn_pubrel(sn_pubrec->message_id);
				break;
	        }
	        case SN_PUBREL:
	        {
	        	struct SnPubrel * sn_pubrel = (struct SnPubrel*) sn_message->packet;
	        	send_sn_pubcomp(sn_pubrel->message_id);
				break;
	        }
	        case SN_SUBACK:
	        {
	        	struct SnSuback * sa = (struct SnSuback*) sn_message->packet;
	        	struct SnMessage * sn_message = sn_get_message_from_map(sa->message_id);
	        	if(sn_message == NULL) {
	        		printf("Cannot get SnSubscribe after SnSuback for id : %i \n", sa->message_id);
	        		break;
	        	}
	        	struct SnSubscribe * s = (struct SnSubscribe*) sn_message->packet;
	        	if(sa->code == SN_ACCEPTED) {
	        		//store in maps <topic_name, tipic_id> and viceversa
	        		sn_add_topic_id_in_map(s->topic->value,sa->topic_id);
	        		sn_add_topic_name_in_map(sa->topic_id,s->topic->value);

	        		add_topics_to_list_box(s->topic->value, s->topic->qos);
	        		save_topic_to_db(s->topic->value, s->topic->qos);
	        		sn_remove_message_from_map(sa->message_id);
	        	} else {
	        		printf("MQTT client have got SUBACK with error : %i \n",sa->code);
	        	}
	        	break;
	        }
	        case SN_UNSUBACK:
	        {
	        	struct SnUnsuback * sn_unsuback = (struct SnUnsuback*) sn_message->packet;
	        	struct SnMessage * sn_m = sn_get_message_from_map(sn_unsuback->message_id);
	        	if(sn_message == NULL) {
	        		printf("Cannot get SnUnsubscribe after SnUnsuback for id : %i \n", sn_unsuback->message_id);
	        		break;
	        	}
	        	struct SnUnsubscribe * sn_un = (struct SnUnsubscribe*) sn_m->packet;

	        	char * topic_name = NULL;
	        	if(sn_un->topic->topic_type == NAMED) {
	        		topic_name = malloc(sizeof (char)*(strlen(sn_un->topic->value)+1));
	        		strcpy(topic_name,sn_un->topic->value);
	        	} else {
	        		topic_name = sn_get_topic_name_from_map(sn_un->topic->id);
	        	}
	        	printf("TOPIC NAME : %s \n", topic_name);
	        	remove_topic_from_list_box(topic_name);
	        	remove_topic_from_db(topic_name);
	        	sn_remove_message_from_map(sn_un->message_id);
				break;
	        }

	        case SN_PINGRESP:
	        {
	            break;
	        }
	        case SN_DISCONNECT:
	        {
	        	sn_stop_all_timers();
	            break;
	        }

	    }
	}

