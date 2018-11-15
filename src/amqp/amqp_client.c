/*
 * amqp_client.c
 *
 *  Created on: Oct 31, 2018
 *      Author: konstantinnosach
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../gui/main_w.h"
#include "../dal/cdal.h"
#include "../account.h"
#include "../common_topic.h"
#include "../mqtt_listener.h"
#include "../tcp_listener.h"
#include "../net/tcp_client.h"
#include "../helpers.h"
#include "amqp_parser.h"
#include "amqp_timers.h"
#include "packets/amqp_proto_header.h"
#include "packets/sasl_mechanisms.h"
#include "packets/sasl_init.h"
#include "packets/sasl_outcome.h"
#include "packets/amqp_header.h"
#include "packets/amqp_open.h"
#include "packets/amqp_begin.h"
#include "packets/amqp_attach.h"
#include "packets/amqp_detach.h"
#include "packets/amqp_transfer.h"
#include "packets/amqp_disposition.h"
#include "packets/amqp_end.h"
#include "packets/amqp_close.h"
#include "wrappers/amqp_symbol.h"
#include "sections/section_entry.h"
#include "sections/amqp_data.h"
#include "avps/outcome_code.h"

#define TCP_PROTOCOL 1

static struct TcpListener * tcp_listener = NULL;
static struct MqttListener * mqtt_listener = NULL;
static struct Account * account = NULL;

static int channel = 0;
static int next_handle = 1;
static int packet_id_counter = 1;
//static int current_packet_number = 0;
//static int delay_in_seconds = 0;

void process_amqp_rx(char * data, int length);
void send_amqp_proto_header(int protocol_id);
void start_amqp_connect(struct Account * acc);
void send_amqp_attach(const char * topic_name, int qos);
void send_amqp_detach(const char * topic_name);
void send_amqp_end(void);
void send_amqp_close(void);
void amqp_data_received(char * buf, int readable_bytes);
void send_amqp_transfer(const char * content, const char * topic_name, int qos, int retain, int dup);
//static void send_sn_register(const char * topic_name);

void amqp_encode_and_fire(struct AmqpHeader * header) {

	int total_length = 0;
	char * buf = amqp_encode(header, &total_length);
	//send
	write_to_tcp_connection(buf,total_length);
}


int init_amqp_client(struct Account * acc, struct MqttListener * listener) {

	account = acc;
	mqtt_listener = listener;
	mqtt_listener->send_connect = start_amqp_connect;
	mqtt_listener->send_sub = send_amqp_attach;
	mqtt_listener->send_message = send_amqp_transfer;

	mqtt_listener->send_unsubscribe = send_amqp_detach;
	mqtt_listener->send_disconnect = send_amqp_end;

	tcp_listener = malloc (sizeof (struct TcpListener));
	tcp_listener->prd_pt = amqp_data_received;
	int isSuccessful = open_tcp_connection(acc->server_host, acc->server_port, TCP_PROTOCOL, tcp_listener);
	if (isSuccessful >= 0) {
		printf("result of successful connection :  %i \n", isSuccessful);
	}
	else
	{
		printf("result of unsuccessful connection : %i \n", isSuccessful);
	}
	return isSuccessful;

}

void start_amqp_connect(struct Account * acc) {
	send_amqp_proto_header(3);
	amqp_start_connect_timer();
}

void send_amqp_proto_header(int protocol_id) {
	//prepare connect packet

	struct AmqpProtoHeader * proto = malloc (sizeof (struct AmqpProtoHeader));
	proto->protocol = "AMQP";
	proto->protocol_id = protocol_id;
	proto->version_major = 1;
	proto->version_minor = 0;
	proto->version_revision = 0;

	struct AmqpHeader * header = malloc (sizeof (struct AmqpHeader));
	header->code = PROTO;
	header->packet = proto;

	amqp_encode_and_fire(header);
}

void send_amqp_sasl_init(struct AmqpHeader * in_header) {

	struct SaslMechanisms * sasl_mechanisms = (struct SaslMechanisms*) in_header->packet;
	struct SaslInit * sasl_init = malloc (sizeof (struct SaslInit));
	sasl_init->mechanism = malloc(sizeof(sasl_init->mechanism));
	sasl_init->mechanism = sasl_mechanisms->mechanisms;

	int user_bytes_length = strlen(account->username);
	int password_bytes_length = strlen(account->password);

	int total_length = user_bytes_length + 1 + user_bytes_length + 1 + password_bytes_length;
	char * challenge = malloc(total_length);
	int pos = 0;
	memcpy(&challenge[pos], account->username, user_bytes_length);
	challenge[user_bytes_length] = 0x00;
	pos+=user_bytes_length + 1;
	memcpy(&challenge[pos], account->username, user_bytes_length);
	challenge[user_bytes_length + 1 + user_bytes_length] = 0x00;
	pos += user_bytes_length + 1;
	memcpy(&challenge[pos], account->password, password_bytes_length);
	sasl_init->initial_response = challenge;
	sasl_init->initial_response_length = total_length;
	sasl_init->host_name = NULL;

	struct AmqpHeader * header = malloc (sizeof (struct AmqpHeader));
	header->code = INIT;
	header->packet = sasl_init;
	header->channel = in_header->channel;
	header->doff = in_header->doff;
	header->type = in_header->type;

	amqp_encode_and_fire(header);

}

void send_amqp_open() {

	struct AmqpOpen * open =  malloc (sizeof (struct AmqpOpen));
	open->container_id = (char *)account->client_id;
	open->idle_timeout = malloc(sizeof(long));
	*(open->idle_timeout) = account->keep_alive * 1000;

	open->channel_max = NULL;
	open->desired_capabilities = NULL;
	open->hostname = NULL;
	open->incoming_locales = NULL;
	open->max_frame_size = NULL;
	open->offered_capabilities = NULL;
	open->outgoing_locales = NULL;
	open->properties = NULL;

	struct AmqpHeader * header = malloc (sizeof (struct AmqpHeader));
	header->code = OPEN;
	header->packet = open;
	header->channel = channel;
	header->doff = 2;
	header->type = 0;

	amqp_encode_and_fire(header);

}

void send_amqp_begin() {

	struct AmqpBegin * begin =  malloc (sizeof (struct AmqpBegin));
	begin->desired_capabilities = NULL;
	begin->handle_max = NULL;
	begin->incoming_window = malloc(sizeof(long));
	*(begin->incoming_window) = 2147483647;
	begin->next_outgoing_id = malloc(sizeof(long));
	*(begin->next_outgoing_id) = 0;
	begin->offered_capabilities = NULL;
	begin->outgoing_window = malloc(sizeof(long));
	*(begin->outgoing_window) = 0;
	begin->properties = NULL;
	begin->remote_channel = NULL;

	struct AmqpHeader * header = malloc (sizeof (struct AmqpHeader));
	header->code = BEGIN;
	header->packet = begin;
	header->channel = channel;
	header->doff = 2;
	header->type = 0;

	amqp_encode_and_fire(header);
}

void send_amqp_end () {

	struct AmqpHeader * header = malloc (sizeof (struct AmqpHeader));
	header->code = END;
	header->packet = NULL;
	header->channel = channel;
	header->doff = 2;
	header->type = 0;

	amqp_encode_and_fire(header);

}

void send_amqp_close () {

	struct AmqpHeader * header = malloc (sizeof (struct AmqpHeader));
	header->code = CLOSE;
	header->packet = NULL;
	header->channel = channel;
	header->doff = 2;
	header->type = 0;

	amqp_encode_and_fire(header);

}

void send_amqp_ping () {

	struct AmqpHeader * header = malloc (sizeof (struct AmqpHeader));
	header->code = PING;
	header->packet = NULL;
	header->channel = channel;
	header->doff = 2;
	header->type = 0;

	amqp_encode_and_fire(header);
}

void send_amqp_attach_with_handler(const char * topic_name, int qos, long handler) {

	struct AmqpAttach * attach =  malloc (sizeof (struct AmqpAttach));

	attach->name = topic_name;

	attach->handle = malloc(sizeof(long));
	*(attach->handle) = handler;
	attach->role = malloc(sizeof(enum RoleCode));
	*(attach->role) = RECEIVER;
	attach->snd_settle_mode = malloc(sizeof(enum SendCode));
	*(attach->snd_settle_mode) = MIXED;

	attach->desired_capabilities = NULL;
	attach->incomplete_unsettled = NULL;
	attach->initial_delivery_count = NULL;
	attach->max_message_size = NULL;
	attach->offered_capabilities = NULL;
	attach->properties = NULL;
	attach->rcv_settle_mode = NULL;
	attach->source = NULL;
	attach->unsettled = NULL;

	struct AmqpTarget * target =  malloc (sizeof (struct AmqpTarget));
	target->address = topic_name;
	target->durable = malloc(sizeof(enum TerminusDurability));
	*(target->durable) = DURABILITY_NONE;
	target->timeout = malloc(sizeof(long));
	*(target->timeout) = 0;
	target->dynamic = malloc(sizeof(int));
	*(target->dynamic) = 0;

	target->capabilities = NULL;
	target->dynamic_node_properties = NULL;
	target->expiry_period = NULL;

	attach->target = target;

	struct AmqpHeader * header = malloc (sizeof (struct AmqpHeader));
	header->code = ATTACH;
	header->packet = attach;
	header->channel = channel;
	header->doff = 2;
	header->type = 0;

	amqp_encode_and_fire(header);

}

void send_amqp_detach(const char * topic_name) {

	struct AmqpDetach * detach =  malloc (sizeof (struct AmqpDetach));
	long handle = amqp_get_handler_from_map(topic_name);
	if(handle == 0) {
		printf("AMQP: can not find hadle for topic name: %s, send unsubscribe (detach) aborted\n",topic_name);
		return;
	}
	detach->handle = malloc(sizeof(long));
	*(detach->handle) = handle;
	detach->closed = malloc(sizeof(int));
	*(detach->closed) = 1;
	detach->error = NULL;

	struct AmqpHeader * header = malloc (sizeof (struct AmqpHeader));
	header->code = DETACH;
	header->packet = detach;
	header->channel = channel;
	header->doff = 2;
	header->type = 0;

	amqp_encode_and_fire(header);

}

void send_amqp_disposition (int delivery_id) {

	struct AmqpDisposition * disposition =  malloc (sizeof (struct AmqpDisposition));

	disposition->role = malloc(sizeof(enum RoleCode));
	*(disposition->role) = RECEIVER;
	disposition->first = malloc(sizeof(long));
	*(disposition->first) = delivery_id;
	disposition->last = malloc(sizeof(long));
	*(disposition->last) = delivery_id;
	disposition->settled = malloc(sizeof(int));
	*(disposition->settled) = 1;
	disposition->batchable = NULL;
	struct AmqpState * state = malloc(sizeof(struct AmqpState));
	state->code = AMQP_ACCEPTED;
	disposition->state = state;

	struct AmqpHeader * header = malloc (sizeof (struct AmqpHeader));
	header->code = DISPOSITION;
	header->packet = disposition;
	header->channel = channel;
	header->doff = 2;
	header->type = 0;

	amqp_encode_and_fire(header);

}

void send_amqp_transfer(const char * content, const char * topic_name, int qos, int retain, int dup) {

	struct AmqpTransfer * transfer =  malloc (sizeof (struct AmqpTransfer));

	transfer->settled = malloc(sizeof(int));
	if(qos==AT_MOST_ONCE)
		*(transfer->settled) = 1;
	else
		*(transfer->settled) = 0;

	transfer->more = malloc(sizeof(int));
	*(transfer->more) = 0;
	transfer->delivery_id = malloc(sizeof(long));
	*(transfer->delivery_id) = packet_id_counter++;

	transfer->message_format = malloc(sizeof(struct AmqpMessageFormat));
	struct AmqpMessageFormat * format = malloc(sizeof(struct AmqpMessageFormat));
	format->message_format = 0;
	transfer->message_format = format;

	transfer->aborted = NULL;
	transfer->batchable = NULL;

	transfer->delivery_tag = NULL;
	transfer->rcv_settle_mode = NULL;
	transfer->resume = NULL;
	transfer->state = NULL;

	transfer->sections = malloc(1 * sizeof(struct SectionEntry));
	transfer->section_number = malloc(sizeof(int));
	*(transfer->section_number) = 1;
	struct SectionEntry * entry = malloc(sizeof(struct SectionEntry));
	struct AmqpData * amqp_data = malloc(sizeof(struct AmqpData));
	amqp_data->data = (char*)content;
	amqp_data->data_length = strlen(amqp_data->data);
	entry->section = amqp_data;
	entry->code = DATA;
	transfer->sections[0] = *entry;

	struct AmqpHeader * header = malloc (sizeof (struct AmqpHeader));
	header->code = TRANSFER;
	header->packet = transfer;
	header->channel = channel;
	header->doff = 2;
	header->type = 0;

	long handle = amqp_get_handler_from_map_outgoing(topic_name);


	if (handle != 0)
	{
		transfer->handle = malloc(sizeof(long));
		*(transfer->handle) = handle;

		amqp_encode_and_fire(header);

	} else {
		//send attach as sender

		long current_handler = next_handle++;

		amqp_add_handler_in_map_outgoing(topic_name, current_handler);
		amqp_add_topic_name_in_map(current_handler, topic_name);

		transfer->handle = malloc(sizeof(long));
		*(transfer->handle) = current_handler;

		//store message, will send it after attach
		amqp_add_message_in_map_by_handler(current_handler, header);

		struct AmqpAttach * attach =  malloc (sizeof (struct AmqpAttach));
		attach->name = topic_name;
		attach->handle = malloc(sizeof(long));
		*(attach->handle) = current_handler;
		attach->role = malloc(sizeof(enum RoleCode));
		*(attach->role) = SENDER;
		attach->rcv_settle_mode = malloc(sizeof(enum ReceiveCode));
		*(attach->rcv_settle_mode) = FIRST;
		attach->initial_delivery_count = malloc(sizeof(long));
		*(attach->initial_delivery_count) = 0;

		attach->desired_capabilities = NULL;
		attach->incomplete_unsettled = NULL;
		attach->max_message_size = NULL;
		attach->offered_capabilities = NULL;
		attach->properties = NULL;
		attach->target = NULL;
		attach->unsettled = NULL;
		attach->snd_settle_mode = NULL;

		struct AmqpSource * source = malloc (sizeof (struct AmqpSource));
		//memset(source, 0, sizeof(source));
		source->address = (char*)topic_name;
		source->durable = malloc(sizeof(enum TerminusDurability));
		*(source->durable) = DURABILITY_NONE;
		source->timeout = malloc(sizeof(long));
		*(source->timeout) = 0;
		source->dynamic = malloc(sizeof(int));
		*(source->dynamic) = 0;

		source->capabilities = NULL;
		source->default_outcome = NULL;
		source->distribution_mode = NULL;
		source->dynamic_node_properties = NULL;
		source->expiry_period = NULL;
		source->filter = NULL;
		source->outcomes = NULL;

		attach->source = source;

		struct AmqpHeader * header = malloc (sizeof (struct AmqpHeader));
		header->code = ATTACH;
		header->packet = attach;
		header->channel = channel;
		header->doff = 2;
		header->type = 0;

		amqp_encode_and_fire(header);
	}

}

void send_amqp_attach(const char * topic_name, int qos) {

	send_amqp_attach_with_handler(topic_name, qos, next_handle++);
}

void amqp_data_received(char * buf, int readable_bytes) {

	int i = 0;
	int length = get_int(buf,0);
	if (length == 1095586128)
	{
		i += 4;
		int protocolId = buf[i++];
		int versionMajor = buf[i++];
		int versionMinor = buf[i++];
		int versionRevision = buf[i++];
		if ((protocolId == 0 || protocolId == 3) && versionMajor == 1 && versionMinor == 0 && versionRevision == 0 && readable_bytes-8 > 0)
		{
			//proto+mech
			char * proto_bytes = malloc(8*sizeof(char));
			memcpy(proto_bytes,buf,8);
			process_amqp_rx(proto_bytes, 8);
			char * mech_bytes = malloc((readable_bytes - 8) * sizeof(char));
			memcpy(mech_bytes,&(buf[8]),(readable_bytes - 8));
			process_amqp_rx(mech_bytes, (readable_bytes - 8));
			//tcp_listener->prd_pt();

		} else {
			//proto
			process_amqp_rx(buf, readable_bytes);
		}
	} else {

		do {
			char * next_bytes = malloc(length * sizeof(char));
			memcpy(next_bytes,&(buf[i]),length);
			process_amqp_rx(next_bytes, length);
			i += length;
			length = get_int(buf,i);
		} while (i < readable_bytes);
	}
}
void process_amqp_rx(char * data, int readable_bytes) {

	struct AmqpHeader * header = amqp_decode(data, readable_bytes);

	switch(header->code) {

		case PROTO: {
			struct AmqpProtoHeader * proto = (struct AmqpProtoHeader*) header->packet;

			if (proto->protocol_id == 0)
			{
				channel = header->channel;
				send_amqp_open();
			}
			break;
		}
		case OPEN: {
			amqp_stop_connect_timer();
			start_amqp_ping_timer(account->keep_alive);
			send_amqp_begin();
			break;
		}
		case BEGIN: {
			mqtt_listener->cs_pt();

			struct MqttModel * mqtt_model = get_topics_from_db();
			if(mqtt_model != NULL) {
				for(int i = 0; i < mqtt_model->topic_size; i++) {
					struct CommonTopic * common_topic = &(mqtt_model->topic[i]);
					//save in map
					long current_handler = next_handle++;
					amqp_add_handler_in_map(common_topic->topic_name, current_handler);
					amqp_add_topic_name_in_map(current_handler,common_topic->topic_name);
					//send attach for each topic in db
					send_amqp_attach_with_handler(common_topic->topic_name, common_topic->qos, current_handler);
				}
			}

			//start resend timer
			amqp_start_message_timer();

			break;
		}
		case ATTACH: {

			struct AmqpAttach * attach = (struct AmqpAttach*) header->packet;

	        if (attach->role != NULL) {
	        	//its opposite here
	            if (*(attach->role) == RECEIVER)
	            {
	            	long real_handle = amqp_get_handler_from_map_outgoing(attach->name);

	            	//publish
	                if (real_handle != 0)
	                {
	                	struct AmqpHeader * current_message = amqp_get_message_from_map_by_handler(real_handle);
	                	if(current_message != NULL) {
	                		amqp_encode_and_fire(current_message);
	                		amqp_remove_message_from_map_handler(real_handle);

	                		//if qos = 1 store in map for resend
	                		struct AmqpTransfer * transfer = (struct AmqpTransfer*)current_message->packet;
	                		if(transfer->settled != NULL && *(transfer->settled)) {
	                			//qos 0
	                			//add mesage in db and gui
	                			struct AmqpData * amqp_data = (struct AmqpData*)transfer->sections[0].section;
								save_message (amqp_data->data, attach->name, AT_MOST_ONCE, 0, 0, 0);
								//update gui
								update_messages_window(amqp_data->data, attach->name, AT_MOST_ONCE, 0, 0, 0);
	                		} else {
	                			//qos 1
	                			//add mesage in timer
	                			amqp_add_message_in_map_by_delivery_id(*(transfer->delivery_id), current_message);
	                		}
	                	} else {
	                		printf("AMQP : can not find message for transferring for handle : %ld\n", real_handle);
	                	}
	                } else {
	                	printf("AMQP : can not find handle transferring for topic : %s\n", attach->name);
	                }
	            }
	            else if(*(attach->role) == SENDER)
	            {

	            	long current_handler = amqp_get_handler_from_map(attach->name);
					if ( current_handler == 0) {
						amqp_add_handler_in_map(attach->name, *(attach->handle));
						amqp_add_topic_name_in_map(*(attach->handle), attach->name);
						add_topics_to_list_box(attach->name, AT_LEAST_ONCE);
						save_topic_to_db(attach->name, AT_LEAST_ONCE);
					}

	                if(*(attach->handle) >= next_handle)
	                	next_handle = *(attach->handle) + 1;

	                //subscribe

	            } else {
	            	printf("Error: incorrect role : %i\n",*(attach->role));
	            }
	        } else {
	        	printf("Attach does not have role, client will skip this packet!\n");
	        }

			break;
		}
		case FLOW: {
			break;
		}
		case TRANSFER: {
			struct AmqpTransfer * transfer = (struct AmqpTransfer*) header->packet;
			enum QoS qos = AT_LEAST_ONCE;
			if (transfer->settled != NULL && *(transfer->settled)){
				qos = AT_MOST_ONCE;
				printf("setteled true at most once enbled\n");
			} else {
				printf("will send disposition\n");
				send_amqp_disposition(*(transfer->delivery_id));
			}

			struct AmqpData * amqp_data = (struct AmqpData*)transfer->sections[0].section;
			char * topic_name = amqp_get_topic_name_from_map(*(transfer->handle));

			char * content = malloc((amqp_data->data_length+1)*sizeof(char));
			memcpy(content, amqp_data->data, amqp_data->data_length);
			content[amqp_data->data_length] ='\0';
			save_message (content, topic_name, qos, 0, 0, 1);
			update_messages_window(content, topic_name, qos, 0, 0, 1);

			break;
		}
		case DISPOSITION: {
			struct AmqpDisposition * disposition = (struct AmqpDisposition*) header->packet;

			struct AmqpHeader * curr_header = amqp_get_message_from_map_by_delivery_id(*(disposition->first));
			if(curr_header == NULL) {
				printf("AMQP : Can not get message from map by delivery id : %ld\n",*(disposition->first));
				return;
			} else {

				struct AmqpTransfer * transfer = (struct AmqpTransfer*)curr_header->packet;
				struct AmqpData * amqp_data = (struct AmqpData*)transfer->sections[0].section;

				char * topic_name = amqp_get_topic_name_from_map(*(transfer->handle));
				save_message (amqp_data->data, topic_name, AT_LEAST_ONCE, 0, 0, 0);
				update_messages_window(amqp_data->data, topic_name, AT_LEAST_ONCE, 0, 0, 0);

			}
			//remove message from map
			amqp_remove_message_from_map_delivery_id(*(disposition->first));

			break;
		}
		case DETACH: {
			struct AmqpDetach * detach = (struct AmqpDetach*) header->packet;
			//get from map
			char * topic_name = amqp_get_topic_name_from_map(*(detach->handle));
			if (topic_name != NULL) {
				remove_topic_from_list_box(topic_name);
				remove_topic_from_db(topic_name);
				//remove from maps
				amqp_remove_topic_name_from_map(topic_name);
				amqp_remove_handle_from_map(*(detach->handle));

			} else {
				printf("AMQP: Detach received with unrecognized handle \n");
			}
			free(detach);
			break;
		}
		case END: {
			send_amqp_close();
			break;
		}
		case CLOSE: {
			break;
		}
		case MECHANISMS: {
			send_amqp_sasl_init(header);
			break;
		}
		case INIT: {
			break;
		}
		case CHALLENGE: {
			break;
		}
		case RESPONSE: {
			break;
		}
		case OUTCOME: {
			struct SaslOutcome * outcome = (struct SaslOutcome*) header->packet;
			if(outcome->outcome_code == OK) {
				send_amqp_proto_header(0);
			} else {
				printf("Errore of Outcome : %x\n", outcome->outcome_code);
			}

			break;
		}
		case PING: {
			break;
		}


	}
}
