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
#include "../coap/coap_timers.h"
#include "coap_parser.h"

#define UDP_PROTOCOL 2
#define COAP_PROTOCOL_VERSION 1

static struct TcpListener * tcp_listener = NULL;
static struct MqttListener * mqtt_listener = NULL;
static struct Account * account = NULL;

static int current_packet_number = 0;
static int delay_in_seconds = 0;
static int init_connect = 0;

void process_coap_rx(char * data, int length);
void coap_connect(struct Account * acc);
void send_coap_subscribe(const char * topic_name, int qos);
void send_coap_message(const char * content, const char * topic_name, int qos, int retain, int dup);
void send_coap_unsubscribe(const char * topic_name);
void send_coap_disconnect();
void fin_coap_client();

void coap_encode_and_fire(struct CoapMessage * coap_message) {
	//encode
	int total_length = 256;
	char * buf = coap_encode(coap_message, &total_length);
	//send
	if(account->is_secure)
		dtls_fire(buf,total_length);
	else
		write_to_net(buf,total_length);
}

int init_coap_client(struct Account * acc, struct MqttListener * listener) {

	account = acc;
	mqtt_listener = listener;
	delay_in_seconds = acc->keep_alive;
	mqtt_listener->send_connect = coap_connect;
	mqtt_listener->send_sub = send_coap_subscribe;
	mqtt_listener->send_message = send_coap_message;
	mqtt_listener->send_disconnect = send_coap_disconnect;
	mqtt_listener->send_unsubscribe = send_coap_unsubscribe;

	const char * host = acc->server_host;
	int port = acc->server_port;
	tcp_listener = malloc (sizeof (struct TcpListener));
	tcp_listener->prd_pt = process_coap_rx;
	tcp_listener->stop_pt = fin_coap_client;
	if(!acc->is_secure) {
		init_net_service(host, port, UDP_PROTOCOL, tcp_listener);
	}
	else {
		init_dtls(host, port, tcp_listener, acc->certificate, acc->certificate_password);
	}

		printf("COAP client initialized \n");
	return 0;
}


void coap_connect(struct Account * acc) {

	init_connect = 1;
	coap_send_ping();
	//start_coap_connect_timer();
}

void send_coap_disconnect() {
	coap_stop_all_timers();
	if(account->is_secure)
		stop_dtls_net_service();
	else
		stop_net_service();
}

void send_coap_subscribe(const char * topic_name, int qos) {

	struct CoapMessage * message = malloc (sizeof (struct CoapMessage));

	char qos_value [2];
	qos_value[0] = 0x00;
	switch (qos)
	{
		case AT_LEAST_ONCE:
			qos_value[1] = 0x01;
		    break;
		case AT_MOST_ONCE:
			qos_value[1] = 0x00;
		    break;
		case EXACTLY_ONCE:
			qos_value[1] = 0x02;
		    break;
	}

	message->version = COAP_PROTOCOL_VERSION;
	message->message_id = ++current_packet_number;
	char str[5];
	sprintf(str, "%d", current_packet_number);
	message->token = str;
	message->type = CONFIRMABLE;
	message->code = GET;
	char payload [1] = {0x00};
	message->payload = payload;

	struct CoapOption * options = malloc (sizeof (struct CoapOption)*4);
	struct CoapOption * observe = malloc (sizeof (struct CoapOption));
	observe->length = 4;
	observe->number = OBSERVE;
	char observe_value [4] = { 0x00, 0x00, 0x00, 0x00 };
	observe->value = observe_value;
	options[0] = *observe;
	struct CoapOption * uri_path = malloc (sizeof (struct CoapOption));
	uri_path->length = strlen(topic_name);
	uri_path->number = URI_PATH;
	uri_path->value = topic_name;
	options[1] = *uri_path;
	struct CoapOption * accept = malloc (sizeof (struct CoapOption));
	accept->length = 2;
	accept->number = ACCEPT;
	accept->value = qos_value;
	options[2] = *accept;
	struct CoapOption * node_id = malloc (sizeof (struct CoapOption));
	node_id->length = strlen(account->client_id);
	node_id->number = NODE_ID;
	node_id->value = account->client_id;
	options[3] = *node_id;

	message->options = options;
	message->options_amount = 4;

	coap_add_message_in_map(message);
	coap_encode_and_fire(message);

}

void send_coap_message(const char * content, const char * topic_name, int qos, int retain, int dup) {

	struct CoapMessage * message = malloc (sizeof (struct CoapMessage));

	char * qos_value = malloc(2 * sizeof (char));
	qos_value[0] = 0x00;
	switch (qos)
	{
		case AT_LEAST_ONCE:
			qos_value[1] = 0x01;
			break;
		case AT_MOST_ONCE:
			qos_value[1] = 0x00;
			break;
		case EXACTLY_ONCE:
			qos_value[1] = 0x02;
			break;
	}

	message->version = COAP_PROTOCOL_VERSION;
	message->message_id = ++current_packet_number;
	char str[5];
	sprintf(str, "%d", current_packet_number);
	message->token = str;
	message->type = CONFIRMABLE;
	message->code = PUT;
	message->payload = content;
	struct CoapOption * options = malloc (sizeof (struct CoapOption)*3);
	struct CoapOption * uri_path = malloc (sizeof (struct CoapOption));
	uri_path->length = strlen(topic_name);
	uri_path->number = URI_PATH;
	uri_path->value = topic_name;
	options[0] = *uri_path;
	struct CoapOption * accept = malloc (sizeof (struct CoapOption));
	accept->length = 2;
	accept->number = ACCEPT;
	accept->value = qos_value;
	options[1] = *accept;
	struct CoapOption * node_id = malloc (sizeof (struct CoapOption));
	node_id->length = strlen(account->client_id);
	node_id->number = NODE_ID;
	node_id->value = account->client_id;
	options[2] = *node_id;
	message->options = options;
	message->options_amount = 3;

	coap_add_message_in_map(message);
	coap_encode_and_fire(message);

}

void send_coap_unsubscribe(const char * topic_name) {

	struct CoapMessage * message = malloc (sizeof (struct CoapMessage));

	message->version = COAP_PROTOCOL_VERSION;
	message->message_id = ++current_packet_number;
	char str[5];
	sprintf(str, "%d", current_packet_number);
	message->token = str;
	message->type = CONFIRMABLE;
	message->code = GET;
	char payload [1] = {0x00};
	message->payload = payload;
	struct CoapOption * options = malloc (sizeof (struct CoapOption)*3);
	struct CoapOption * observe = malloc (sizeof (struct CoapOption));
	observe->length = 4;
	observe->number = OBSERVE;
	char observe_value [4] = { 0x00, 0x00, 0x00, 0x01 };
	observe->value = observe_value;
	options[0] = *observe;
	struct CoapOption * uri_path = malloc (sizeof (struct CoapOption));
	uri_path->length = strlen(topic_name);
	uri_path->number = URI_PATH;
	uri_path->value = topic_name;
	options[1] = *uri_path;
	struct CoapOption * node_id = malloc (sizeof (struct CoapOption));
	node_id->length = strlen(account->client_id);
	node_id->number = NODE_ID;
	node_id->value = account->client_id;
	options[2] = *node_id;
	message->options = options;
	message->options_amount = 3;

	coap_add_message_in_map(message);
	coap_encode_and_fire(message);

}

void coap_send_ping() {

	struct CoapMessage * message = malloc (sizeof (struct CoapMessage));

	message->version = COAP_PROTOCOL_VERSION;
	message->message_id = 0;
	char str[5];
	sprintf(str, "%d", 0);
	message->token = str;
	message->type = CONFIRMABLE;
	message->code = PUT;
	char payload [1] = {0x00};
	message->payload = payload;
	struct CoapOption * options = malloc (sizeof (struct CoapOption)*1);
	struct CoapOption * node_id = malloc (sizeof (struct CoapOption));
	node_id->length = strlen(account->client_id);
	node_id->number = NODE_ID;
	node_id->value = account->client_id;
	options[0] = *node_id;
	message->options = options;
	message->options_amount = 1;

	coap_encode_and_fire(message);
}

void send_coap_ack(struct CoapMessage * in_message, int is_ok) {

	struct CoapMessage * out_message = malloc (sizeof (struct CoapMessage));

	out_message->version = in_message->version;
	out_message->message_id = in_message->message_id;
	out_message->token = in_message->token;
	out_message->type = ACKNOWLEDGEMENT;
	if(is_ok) {
		out_message->code = in_message->code;
		out_message->options_amount = 1;
	}
	else {
		out_message->code = BAD_OPTION;
		out_message->options_amount = 2;
	}
	char payload [1] = {0x00};
	out_message->payload = payload;

	struct CoapOption * options = malloc (sizeof (struct CoapOption)*out_message->options_amount);
	struct CoapOption * content_format = NULL;
	int opt_index = 0;
	if(!is_ok) {
		content_format = malloc (sizeof (struct CoapOption));
		char * text_bytes = "text/plain";
		content_format->length = strlen(text_bytes);
		content_format->number = CONTENT_FORMAT;
		content_format->value = text_bytes;
		options[opt_index++] = *content_format;
	}
	struct CoapOption * node_id = malloc (sizeof (struct CoapOption));
	node_id->length = strlen(account->client_id);
	node_id->number = NODE_ID;
	node_id->value = account->client_id;
	options[opt_index++] = * node_id;

	out_message->options = options;

	coap_encode_and_fire(out_message);
}

void process_coap_rx(char * data, int length) {

	if(init_connect) {
		mqtt_listener->cs_pt();
		start_coap_ping_timer((unsigned int)delay_in_seconds);
		coap_start_message_timer();
		init_connect = 0;
	}

	struct CoapMessage * message = coap_decode(data, length);
	if(message == NULL) {
		printf("Error : Cannot decode COAP message...\n");
		return;
	}

    if ((message->code == POST || message->code == PUT) && message->type!=ACKNOWLEDGEMENT)
    {
        char * topic_name = NULL;
        enum QoS qos = AT_MOST_ONCE;
        for (int i = 0; i < message->options_amount; i++) {
			if(message->options[i].number == URI_PATH)
			{
				topic_name = malloc((message->options[i].length+1) * sizeof (char));
				memcpy(topic_name,message->options[i].value, message->options[i].length);
				topic_name[message->options[i].length] = '\0';
				break;
			}
			else if(message->options[i].number == ACCEPT)
				qos = message->options[i].value[message->options[i].length - 1];
        }

        const char * content = message->payload;

        if (topic_name == NULL)
        {
        	send_coap_ack(message, 0);
            return;
        } else {
        	//store message in db and gui
        	mqtt_listener->get_pub_pt(content, topic_name, qos, 0, 0);
        	send_coap_ack(message, 1);
        }
    }

    switch (message->type)
    {
        case CONFIRMABLE:
        	send_coap_ack(message, 1);
        	break;
        case NON_CONFIRMABLE:
        	coap_remove_message_from_map(message->message_id);
            break;
        case ACKNOWLEDGEMENT:
        	if (message->code == GET)
            {
                int observe = -1;
                for (int i = 0; i < message->options_amount; i++)
                {
                    if (message->options[i].number == OBSERVE && message->options[i].length > 0)
                    {
                        if (message->options[i].value[message->options[i].length-1] == 0x00)
                            observe = 0;
                        else
                            observe = 1;

                        break;
                    }
                }

                if (observe!=-1)
                {
                	struct CoapMessage * original_message = coap_get_message_from_map(message->message_id);
                    if (observe == 0)
                    {
                        if (original_message != NULL)
                        {
                            enum QoS qos = AT_MOST_ONCE;
                            for (int i = 0; i < message->options_amount; i++)
                            {
                                if(message->options[i].number == ACCEPT) {
                                	int l = message->options[i].length-1;
                                	qos = message->options[i].value[l];
                                }
                            }

                            for (int i = 0; i < original_message->options_amount; i++)
							{
								if(original_message->options[i].number == URI_PATH) {
									add_topics_to_list_box(original_message->options[i].value, qos);
									save_topic_to_db(original_message->options[i].value, qos);
								}
							}
                        } else {
                        	printf("COAP client: get message with id : %i but cannot find it in map of messages \n", message->message_id);
                        }
                    }
                    else
                    {
                        if (original_message != NULL)
                        {
                            for (int i = 0; i < original_message->options_amount; i++)
                            {
                                if (original_message->options[i].number == URI_PATH) {
                                	char * topic_name = malloc(sizeof (char)*(strlen(original_message->options[i].value)+1));
                                	strcpy(topic_name,original_message->options[i].value);
                                	remove_topic_from_list_box(topic_name);
                                	remove_topic_from_db(topic_name);
                                }
                            }
                        } else {
                        	printf("COAP client: get message with id : %i but cannot find it in map of messages \n", message->message_id);
                        }
                    }
                    coap_remove_message_from_map(message->message_id);
                }
            }
            else if(message->message_id != 0)
            {
            	//puback
            	struct CoapMessage * o_m = coap_get_message_from_map(message->message_id);
            	if(o_m == NULL) {
            		printf("COAP : Cannot get message from map by message_id %i\n", message->message_id);
            		return;
            	}
            	save_message(o_m->payload, o_m->options[0].value, o_m->options[1].value[1], 0, 0, 0);
            	update_messages_window (o_m->payload, o_m->options[0].value, o_m->options[1].value[1], 0, 0, 0);
            	coap_remove_message_from_map(message->message_id);

            }
            break;
        case RESET:
        	coap_remove_message_from_map(message->message_id);
            break;
    }

}

void fin_coap_client() {
	if(init_connect == 1) {
		mqtt_listener->cu_pt(-1);
		init_connect = 0;
	}
}

