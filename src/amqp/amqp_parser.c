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
#include "../helpers.h"
#include "packets/amqp_header.h"
#include "packets/amqp_proto_header.h"
#include "packets/amqp_open.h"
#include "packets/amqp_begin.h"
#include "packets/amqp_attach.h"
#include "packets/amqp_flow.h"
#include "packets/amqp_transfer.h"
#include "packets/amqp_disposition.h"
#include "packets/amqp_detach.h"
#include "packets/amqp_end.h"
#include "packets/amqp_close.h"
#include "packets/sasl_init.h"
#include "wrappers/wrapper.h"
#include "sections/section_entry.h"
#include "sections/amqp_data.h"

#include "avps/amqp_type.h"
#include "avps/section_code.h"
#include "tlv/tlv_amqp.h"
#include "tlv/tlv_list.h"
#include "tlv/tlv_constructor.h"
#include "header_factory.h"
#include "amqp_calc.h"

struct TlvList * add_arguments (struct AmqpHeader * header);
struct TlvList * add_section(struct SectionEntry section);
//char * get_bytes (struct TlvList * list);
//static void calculate_bytes(struct TlvList * tlv_list, enum HeaderCode header_code);


struct AmqpHeader * amqp_decode(char * data, int readable_bytes) {

	struct AmqpHeader * header = malloc (sizeof (struct AmqpHeader));

	int i = 0;
	int length = data[i] << 24 | (data[i+1] << 16) | (data[i+2] << 8) | (data[i+3]);
	i += 4;
	char doff = data[i++];
	char type = data[i++];
	short channel = get_short(data,i);
	i+=2;

	readable_bytes -= i;
	if (length == 8 && doff == 2 && (type == 0 || type == 1) && channel == 0) {
		if (readable_bytes == 0) {
			//TODO return PING
			return header;
		} else {
			printf("Received malformed ping-header with invalid length\n");
			return NULL;
		}
	}

	// PTOROCOL-HEADER
	if (length == 1095586128 && (doff == 3 || doff == 0) && type == 1 && channel == 0) {
		if (readable_bytes == 0) {
			struct AmqpProtoHeader * proto = malloc (sizeof (struct AmqpProtoHeader));
			proto->protocol_id = doff;

			header->packet = proto;
			header->code = PROTO;
			header->doff = 0;
			header->type = 0;
			header->channel = 0;
			return header;
		} else {
			printf("Received malformed protocol-header with invalid length\n");
			return NULL;
		}
	}
	if (length != readable_bytes + 8) {
		printf("Received malformed header with invalid length\n");
		return NULL;
	}


	if (type == 0) {
		header = get_amqp(data, &i);
	} else if (type == 1) {
		header = get_sasl(data, &i);
	} else {
		printf("Received malformed header with invalid type: %i\n",type);
		return NULL;
	}
	header->doff = doff;
	header->type = type;
	header->channel = channel;

	if (header->code == TRANSFER)
	{
		struct AmqpTransfer * transfer = (struct AmqpTransfer*)header->packet;
		transfer->section_number = malloc(sizeof(int));
		transfer->sections = malloc(sizeof(struct SectionEntry));

		int section_counter = 0;
		while (i < (readable_bytes + 8))
		{
			struct SectionEntry * section = get_section(data, &i);
			*(transfer->section_number) = section_counter;
			transfer->sections[section_counter++] = *section;
		}
	}
	return header;
}

char * amqp_encode(struct AmqpHeader * header, int * length) {

	char * buf = NULL;
	int i = 0;

	switch (header->code) {
		case PROTO: {
			struct AmqpProtoHeader proto = *(struct AmqpProtoHeader*) header->packet;

			buf = malloc(8 * sizeof(char));
			while(*proto.protocol!='\0') {
				buf[i] = *proto.protocol;
				proto.protocol++; i++;
			}
			buf[i++] =  proto.protocol_id;
			buf[i++] =  proto.version_major;
			buf[i++] =  proto.version_minor;
			buf[i++] =  proto.version_revision;
			*length = 8;
			break;
		}
		case PING: {
			int ping_length = 8;
			buf = malloc(ping_length * sizeof(char));
			memcpy(&buf[i], &ping_length, sizeof(int));
			reverse((unsigned char *)buf, 4);
			i+= sizeof(int);
			buf[i++] = header->doff;
			buf[i++] = header->type;
			i+= add_short(&buf[i], header->channel);
			*length = ping_length;

			break;
		}
		default : {

			int frame_length = 8;

			struct TlvList * arguments = add_arguments(header);

			*length = frame_length + arguments->length;

			buf = malloc(*length * sizeof(char));
			int length_index = i;
			i+= sizeof(int);
			buf[i++] = header->doff;
			buf[i++] = header->type;
			i+= add_short(&buf[i], header->channel);
			memcpy(&buf[i], arguments->bytes, arguments->length);
			i += arguments->length;
			if(header->code == TRANSFER) {
				struct TlvList * section = NULL;
				struct AmqpTransfer * transfer = (struct AmqpTransfer*) header->packet;
				if(transfer->sections != NULL) {
					for(int k = 0; k < *(transfer->section_number); k++ )
					{
						section = add_section(transfer->sections[k]);
						buf = (char *) realloc (buf,(*length += section->length));
						memcpy(&buf[i], section->bytes, section->length);
						i += section->length;
					}
				}

			}
			unsigned char * frame_len_array = malloc(sizeof(int));
			memcpy(frame_len_array, length, sizeof(int));
			reverse(frame_len_array, sizeof(int));
			memcpy(&buf[length_index], frame_len_array, sizeof(int));
		}
	}

	return buf;
}

struct TlvList * add_section(struct SectionEntry section_entry) {

	struct TlvList * tlv_list = malloc(sizeof(struct TlvList));

	switch (section_entry.code) {

		case HEADER: {
			break;
		}
		case DELIVERY_ANNOTATIONS : {
			break;
		}
		case MESSAGE_ANNOTATIONS : {
			break;
		}
		case PROPERTIES : {
			break;
		}
		case APPLICATION_PROPERTIES : {
			break;
		}
		case DATA : {

			struct AmqpData data = *(struct AmqpData*) section_entry.section;
			struct TlvAmqp * tlv_data = wrap_binary(data.data, data.data_length);
			int total_size = 3 + tlv_data->length;
			tlv_list->length = total_size;
			tlv_list->bytes = malloc (total_size * sizeof(char));
			int i = 0;
			tlv_list->bytes[i++] = 0X00;
			tlv_list->bytes[i++] = SMALL_ULONG;
			tlv_list->bytes[i++] = section_entry.code;
			tlv_list->bytes[i++] = tlv_data->code;
			tlv_list->bytes[i++] = tlv_data->length-2;
			memcpy(&(tlv_list->bytes[i]), tlv_data->value_array, tlv_data->length-2);
			break;

		}
		case SEQUENCE : {
			break;
		}
		case VALUE : {
			break;
		}
		case FOOTER : {
			break;
		}
	}

	return tlv_list;
}

struct TlvList * add_arguments (struct AmqpHeader * header) {

	struct TlvList * tlv_list = malloc(sizeof(struct TlvList));
	switch (header->code) {

		case INIT: {
			struct SaslInit init = *(struct SaslInit*) header->packet;

			if (init.mechanism == NULL) {
				printf("SASL-Init header's mechanism can't be null\n");
				return NULL;
			}
			struct TlvAmqp * mechanism = wrap_symbol(init.mechanism);
			tlv_list->list = malloc(3 * sizeof(struct TlvAmqp));

			tlv_list->count = 1;
			tlv_list->size = mechanism->length;
			tlv_list->width = 1;
			tlv_list->list[0] = *mechanism;

			struct TlvAmqp * initial_response = NULL;
			if (init.initial_response != NULL) {
				initial_response = wrap_binary(init.initial_response, init.initial_response_length);
				tlv_list->count = 2;
				tlv_list->size += initial_response->length;
				tlv_list->list[1] = *initial_response;
			}

			struct TlvAmqp * host_name = NULL;
			if (init.host_name != NULL) {
				host_name = wrap_string(init.host_name);
				tlv_list->count = 3;
				tlv_list->size += host_name->length;
				tlv_list->list[2] = *host_name;
			}
			calculate_bytes(tlv_list, header->code);
			break;
		}
		case OPEN: {
			struct AmqpOpen open = *(struct AmqpOpen*) header->packet;
			tlv_list->list = malloc(5 * sizeof(struct TlvAmqp));

			if (open.container_id == NULL) {
				printf("Open header's container id can't be NULL\n");
				return NULL;
			}

			int count_not_null = 0;
			struct TlvAmqp * container_id = wrap_string(open.container_id);
			tlv_list->count = 1;
			count_not_null++;
			tlv_list->size = container_id->length;
			tlv_list->width = 1;
			tlv_list->list[0] = *container_id;

			struct TlvAmqp * hostname = NULL;
			if (open.hostname != NULL) {
				hostname = wrap_string(open.hostname);
				count_not_null++;
				tlv_list->count = 2;
				tlv_list->size += hostname->length;
				tlv_list->list[1] = *hostname;
			} else {
				tlv_list->list[1] = *wrap_null();
			}

			struct TlvAmqp * max_frame_size = NULL;
			if (open.max_frame_size != NULL) {
				max_frame_size = wrap_long(open.max_frame_size);
				count_not_null++;
				tlv_list->count = 3;
				tlv_list->size += max_frame_size->length;
				tlv_list->list[2] = *max_frame_size;
			} else {
				tlv_list->list[2] = *wrap_null();
			}

			struct TlvAmqp * channel_max = NULL;
			if (open.channel_max != NULL) {
				channel_max = wrap_int(open.channel_max);
				count_not_null++;
				tlv_list->count = 4;
				tlv_list->size += channel_max->length;
				tlv_list->list[3] = *channel_max;
			} else {
				tlv_list->list[3] = *wrap_null();
			}

			struct TlvAmqp * idle_timeout = NULL;
			if (open.idle_timeout != NULL) {
				idle_timeout = wrap_long(open.idle_timeout);
				count_not_null++;
				tlv_list->count = 5;
				tlv_list->size += idle_timeout->length;
				tlv_list->list[4] = *idle_timeout;
			} else {
				tlv_list->list[4] = *wrap_null();
			}

			tlv_list->size += tlv_list->count - count_not_null;

			calculate_bytes(tlv_list, header->code);

			break;
		}
		case BEGIN: {
			struct AmqpBegin begin = *(struct AmqpBegin*) header->packet;
			tlv_list->list = malloc(8 * sizeof(struct TlvAmqp));
			int count_not_null = 0;
			tlv_list->size = 0;
			int list_index = 0;
			tlv_list->width = 1;

			struct TlvAmqp * remote_channel = NULL;
			if (begin.remote_channel != NULL){
				remote_channel = wrap_int(begin.remote_channel);
				count_not_null++;
				tlv_list->count = 1;
				tlv_list->size += remote_channel->length;
				tlv_list->list[list_index++] = *remote_channel;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			if (begin.next_outgoing_id == NULL) {
				printf("Begin header's next-outgoing-id can't be null\n");
				return NULL;
			}

			struct TlvAmqp * next_outgoing_id = NULL;
			if (begin.next_outgoing_id != NULL){
				next_outgoing_id = wrap_long(begin.next_outgoing_id);
				count_not_null++;
				tlv_list->count = 2;
				tlv_list->size += next_outgoing_id->length;
				tlv_list->list[list_index++] = *next_outgoing_id;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			if (begin.incoming_window == NULL) {
				printf("Begin header's incoming_window can't be null\n");
				return NULL;
			}

			struct TlvAmqp * incoming_window = NULL;
			if (begin.incoming_window != NULL){
				incoming_window = wrap_long(begin.incoming_window);
				count_not_null++;
				tlv_list->count = 3;
				tlv_list->size += incoming_window->length;
				tlv_list->list[list_index++] = *incoming_window;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			if (begin.incoming_window == NULL) {
				printf("Begin header's outgoing_window can't be null\n");
				return NULL;
			}

			struct TlvAmqp * outgoing_window = NULL;
			if (begin.outgoing_window != NULL){
				outgoing_window = wrap_long(begin.outgoing_window);
				count_not_null++;
				tlv_list->count = 4;
				tlv_list->size += outgoing_window->length;
				tlv_list->list[list_index++] = *outgoing_window;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			struct TlvAmqp * handle_max = NULL;
			if (begin.handle_max != NULL){
				handle_max = wrap_long(begin.handle_max);
				count_not_null++;
				tlv_list->count = 5;
				tlv_list->size += handle_max->length;
				tlv_list->list[list_index++] = *handle_max;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			struct TlvAmqp * offered_capabilities = NULL;
			if (begin.offered_capabilities != NULL){
				offered_capabilities = wrap_array(begin.offered_capabilities);
				count_not_null++;
				tlv_list->count = 6;
				tlv_list->size += offered_capabilities->length;
				tlv_list->list[list_index++] = *offered_capabilities;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			struct TlvAmqp * desired_capabilities = NULL;
			if (begin.desired_capabilities != NULL){
				desired_capabilities = wrap_array(begin.desired_capabilities);
				count_not_null++;
				tlv_list->count = 7;
				tlv_list->size += desired_capabilities->length;
				tlv_list->list[list_index++] = *desired_capabilities;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			struct TlvAmqp * properties = NULL;
			if (begin.properties != NULL){
				properties = wrap_map(begin.properties);
				count_not_null++;
				tlv_list->count = 8;
				tlv_list->size += properties->length;
				tlv_list->list[list_index++] = *properties;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			tlv_list->size += tlv_list->count - count_not_null;

			calculate_bytes(tlv_list, header->code);

			break;
		}
		case ATTACH: {
			struct AmqpAttach attach = *(struct AmqpAttach*) header->packet;
			tlv_list->list = malloc(14 * sizeof(struct TlvAmqp));
			int count_not_null = 0;
			tlv_list->size = 0;
			int list_index = 0;
			tlv_list->width = 1;

			if (attach.name == NULL) {
				printf("Attach header's name can't be null\n");
				return NULL;
			}
			struct TlvAmqp * name = NULL;
			if (attach.name != NULL){
				name = wrap_string((char*)attach.name);
				count_not_null++;
				tlv_list->count = 1;
				tlv_list->size += name->length;
				tlv_list->list[list_index++] = *name;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			if (attach.handle == NULL) {
				printf("Attach header's handle can't be null\n");
				return NULL;
			}
			struct TlvAmqp * handle = NULL;
			if (attach.handle != NULL){
				handle = wrap_long(attach.handle);
				count_not_null++;
				tlv_list->count = 2;
				tlv_list->size += handle->length;
				tlv_list->list[list_index++] = *handle;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			if (attach.role == NULL) {
				printf("Attach header's role can't be null\n");
				return NULL;
			}

			struct TlvAmqp * role = NULL;
			if (attach.role != NULL){
				role = wrap_bool((int *)attach.role);
				count_not_null++;
				tlv_list->count = 3;
				tlv_list->size += role->length;
				tlv_list->list[list_index++] = *role;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			struct TlvAmqp * snd_settle_mode = NULL;
			if (attach.snd_settle_mode != NULL){
				short * mode = malloc(sizeof(short));
				*mode = *(attach.snd_settle_mode);
				snd_settle_mode = wrap_short(mode);
				count_not_null++;
				tlv_list->count = 4;
				tlv_list->size += snd_settle_mode->length;
				tlv_list->list[list_index++] = *snd_settle_mode;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			struct TlvAmqp * rcv_settle_mode = NULL;
			if (attach.rcv_settle_mode != NULL){
				rcv_settle_mode = wrap_short((short *)attach.rcv_settle_mode);
				count_not_null++;
				tlv_list->count = 5;
				tlv_list->size += rcv_settle_mode->length;
				tlv_list->list[list_index++] = *rcv_settle_mode;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			struct TlvAmqp * source = NULL;
			if (attach.source != NULL){
				source = wrap_amqp_source(attach.source);
				count_not_null++;
				tlv_list->count = 6;
				tlv_list->size += source->length;
				tlv_list->list[list_index++] = *source;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			struct TlvAmqp * target = NULL;
			if (attach.target != NULL){
				target = wrap_amqp_target(attach.target);
				count_not_null++;
				tlv_list->count = 7;
				tlv_list->size += target->length;
				tlv_list->list[list_index++] = *target;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			struct TlvAmqp * unsettled = NULL;
			if (attach.unsettled != NULL){
				unsettled = wrap_map(attach.unsettled);
				count_not_null++;
				tlv_list->count = 8;
				tlv_list->size += unsettled->length;
				tlv_list->list[list_index++] = *unsettled;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			struct TlvAmqp * incomplete_unsettled = NULL;
			if (attach.incomplete_unsettled != NULL){
				incomplete_unsettled = wrap_int(attach.incomplete_unsettled);
				count_not_null++;
				tlv_list->count = 9;
				tlv_list->size += incomplete_unsettled->length;
				tlv_list->list[list_index++] = *incomplete_unsettled;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			struct TlvAmqp * initial_delivery_count = NULL;
			if (attach.initial_delivery_count != NULL){
				initial_delivery_count = wrap_long(attach.initial_delivery_count);
				count_not_null++;
				tlv_list->count = 10;
				tlv_list->size += initial_delivery_count->length;
				tlv_list->list[list_index++] = *initial_delivery_count;
			} else {
				if (attach.role == SENDER){
					printf("Sender's attach header must contain a non-null initial-delivery-count value\n");
					return NULL;
				}
				tlv_list->list[list_index++] = *wrap_null();
			}

			struct TlvAmqp * max_message_size = NULL;
			if (attach.max_message_size != NULL){
				max_message_size = wrap_long(attach.max_message_size);
				count_not_null++;
				tlv_list->count = 11;
				tlv_list->size += max_message_size->length;
				tlv_list->list[list_index++] = *max_message_size;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			struct TlvAmqp * offered_capabilities = NULL;
			if (attach.offered_capabilities != NULL){
				offered_capabilities = wrap_array(attach.offered_capabilities);
				count_not_null++;
				tlv_list->count = 12;
				tlv_list->size += offered_capabilities->length;
				tlv_list->list[list_index++] = *offered_capabilities;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			struct TlvAmqp * desired_capabilities = NULL;
			if (attach.desired_capabilities != NULL){
				desired_capabilities = wrap_array(attach.desired_capabilities);
				count_not_null++;
				tlv_list->count = 13;
				tlv_list->size += desired_capabilities->length;
				tlv_list->list[list_index++] = *desired_capabilities;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			struct TlvAmqp * properties = NULL;
			if (attach.properties != NULL){
				properties = wrap_map(attach.properties);
				count_not_null++;
				tlv_list->count = 14;
				tlv_list->size += properties->length;
				tlv_list->list[list_index++] = *properties;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			tlv_list->size += tlv_list->count-count_not_null;

			calculate_bytes(tlv_list, header->code);

			break;
		}
		case FLOW: {
			//struct AmqpFlow flow = *(struct AmqpFlow*) header->packet;
			break;
		}
		case TRANSFER: {
			struct AmqpTransfer transfer = *(struct AmqpTransfer*) header->packet;
			tlv_list->list = malloc(11 * sizeof(struct TlvAmqp));
			int count_not_null = 0;
			tlv_list->size = 0;
			int list_index = 0;
			tlv_list->width = 1;

			if (transfer.handle == NULL) {
				printf("Transfer header's handle can't be null\n");
				return NULL;
			}

			struct TlvAmqp * handle = NULL;
			if (transfer.handle != NULL){
				handle = wrap_long(transfer.handle);
				count_not_null++;
				tlv_list->count = 1;
				tlv_list->size += handle->length;
				tlv_list->list[list_index++] = *handle;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			struct TlvAmqp * delivery_id = NULL;
			if (transfer.delivery_id != NULL){
				delivery_id = wrap_long(transfer.delivery_id);
				count_not_null++;
				tlv_list->count = 2;
				tlv_list->size += delivery_id->length;
				tlv_list->list[list_index++] = *delivery_id;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			struct TlvAmqp * delivery_tag = NULL;
			if (transfer.delivery_tag != NULL){
				delivery_tag = wrap_binary(transfer.delivery_tag, *(transfer.delivery_tag_length));
				count_not_null++;
				tlv_list->count = 3;
				tlv_list->size += delivery_tag->length;
				tlv_list->list[list_index++] = *delivery_tag;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			struct TlvAmqp * message_format = NULL;
			if (transfer.message_format != NULL){
				message_format = wrap_long(&(transfer.message_format->message_format));
				count_not_null++;
				tlv_list->count = 4;
				tlv_list->size += message_format->length;
				tlv_list->list[list_index++] = *message_format;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			struct TlvAmqp * settled = NULL;
			if (transfer.settled != NULL){
				settled = wrap_bool(transfer.settled);
				count_not_null++;
				tlv_list->count = 5;
				tlv_list->size += settled->length;
				tlv_list->list[list_index++] = *settled;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			struct TlvAmqp * more = NULL;
			if (transfer.more != NULL){
				more = wrap_bool(transfer.more);
				count_not_null++;
				tlv_list->count = 6;
				tlv_list->size += more->length;
				tlv_list->list[list_index++] = *more;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			struct TlvAmqp * rcv_settle_mode = NULL;
			if (transfer.rcv_settle_mode != NULL){
				short * mode = malloc(sizeof(short));
				*mode = *(transfer.rcv_settle_mode);
				rcv_settle_mode = wrap_short(mode);
				count_not_null++;
				tlv_list->count = 7;
				tlv_list->size += rcv_settle_mode->length;
				tlv_list->list[list_index++] = *rcv_settle_mode;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			struct TlvAmqp * state = NULL;
			if (transfer.state != NULL){
				state = wrap_state(transfer.state);
				count_not_null++;
				tlv_list->count = 8;
				tlv_list->size += state->length;
				tlv_list->list[list_index++] = *state;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			struct TlvAmqp * resume = NULL;
			if (transfer.resume != NULL){
				resume = wrap_bool(transfer.resume);
				count_not_null++;
				tlv_list->count = 9;
				tlv_list->size += resume->length;
				tlv_list->list[list_index++] = *resume;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			struct TlvAmqp * aborted = NULL;
			if (transfer.aborted != NULL){
				resume = wrap_bool(transfer.aborted);
				count_not_null++;
				tlv_list->count = 10;
				tlv_list->size += aborted->length;
				tlv_list->list[list_index++] = *aborted;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			struct TlvAmqp * batchable = NULL;
			if (transfer.batchable != NULL){
				batchable = wrap_bool(transfer.batchable);
				count_not_null++;
				tlv_list->count = 11;
				tlv_list->size += batchable->length;
				tlv_list->list[list_index++] = *batchable;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			tlv_list->size += tlv_list->count-count_not_null;

			calculate_bytes(tlv_list, header->code);

			break;
		}
		case DISPOSITION: {
			struct AmqpDisposition disposition = *(struct AmqpDisposition*) header->packet;
			tlv_list->list = malloc(6 * sizeof(struct TlvAmqp));
			int count_not_null = 0;
			tlv_list->size = 0;
			int list_index = 0;
			tlv_list->width = 1;

			if (disposition.role == NULL) {
				printf("Disposition header's role can't be null\n");
				return NULL;
			}
			if (disposition.first == NULL) {
				printf("Transfer header's first can't be null\n");
				return NULL;
			}

			struct TlvAmqp * role = NULL;
			if (disposition.role != NULL){
				int * _role = malloc(sizeof(int));
				*_role = *(disposition.role);
				role = wrap_bool(_role);
				count_not_null++;
				tlv_list->count = 1;
				tlv_list->size += role->length;
				tlv_list->list[list_index++] = *role;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			struct TlvAmqp * first = NULL;
			if (disposition.first != NULL){
				first = wrap_long(disposition.first);
				count_not_null++;
				tlv_list->count = 2;
				tlv_list->size += first->length;
				tlv_list->list[list_index++] = *first;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			struct TlvAmqp * last = NULL;
			if (disposition.last != NULL){
				last = wrap_long(disposition.last);
				count_not_null++;
				tlv_list->count = 3;
				tlv_list->size += last->length;
				tlv_list->list[list_index++] = *last;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			struct TlvAmqp * settled = NULL;
			if (disposition.settled != NULL){
				settled = wrap_bool(disposition.settled);
				count_not_null++;
				tlv_list->count = 4;
				tlv_list->size += settled->length;
				tlv_list->list[list_index++] = *settled;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			struct TlvAmqp * state = NULL;
			if (disposition.state != NULL) {
				state = wrap_state(disposition.state);
				count_not_null++;
				tlv_list->count = 5;
				tlv_list->size += state->length;
				tlv_list->list[list_index++] = *state;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			struct TlvAmqp * batchable = NULL;
			if (disposition.batchable != NULL){
				batchable = wrap_bool(disposition.batchable);
				count_not_null++;
				tlv_list->count = 6;
				tlv_list->size += batchable->length;
				tlv_list->list[list_index++] = *batchable;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			tlv_list->size += tlv_list->count-count_not_null;

			calculate_bytes(tlv_list, header->code);

			break;
		}
		case DETACH: {
			struct AmqpDetach detach = *(struct AmqpDetach*) header->packet;
			tlv_list->list = malloc(3 * sizeof(struct TlvAmqp));
			int count_not_null = 0;
			tlv_list->size = 0;
			int list_index = 0;
			tlv_list->width = 1;

			if (detach.handle == NULL) {
				printf("Detach header's handle can't be null\n");
				return NULL;
			}

			struct TlvAmqp * handle = NULL;
			if (detach.handle != NULL){
				handle = wrap_long(detach.handle);
				count_not_null++;
				tlv_list->count = 1;
				tlv_list->size += handle->length;
				tlv_list->list[list_index++] = *handle;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			struct TlvAmqp * closed = NULL;
			if (detach.closed != NULL){
				closed = wrap_bool(detach.closed);
				count_not_null++;
				tlv_list->count = 2;
				tlv_list->size += closed->length;
				tlv_list->list[list_index++] = *closed;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			struct TlvAmqp * error = NULL;
			if (detach.error != NULL){
				error = wrap_amqp_error(detach.error);
				count_not_null++;
				tlv_list->count = 3;
				tlv_list->size += error->length;
				tlv_list->list[list_index++] = *error;
			} else {
				tlv_list->list[list_index++] = *wrap_null();
			}

			//printf("size of NULL arguments %i\n",(tlv_list->count-count_not_null));

			tlv_list->size += tlv_list->count-count_not_null;

			calculate_bytes(tlv_list, header->code);

			break;
		}
		case CLOSE:
		case END: {
			tlv_list->list = malloc(1 * sizeof(struct TlvAmqp));
			tlv_list->size = 2;
			int list_index = 0;
			tlv_list->width = 1;
			tlv_list->count = 1;
			tlv_list->list[list_index++] = *wrap_null();
			calculate_bytes(tlv_list, header->code);
			break;
		}

		case PROTO:
		case PING:
		case OUTCOME:
		case MECHANISMS:
		case CHALLENGE:
		case RESPONSE:
		{
			break;
		}
	}
	return tlv_list;
}
