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
#include "packets/amqp_header.h"
#include "sections/section_entry.h"
#include "sections/amqp_data.h"
#include "tlv/tlv_amqp.h"
#include "tlv/tlv_list.h"
#include "tlv/amqp_outcome.h"
#include "tlv_factory.h"
#include "packets/sasl_challenge.h"
#include "packets/sasl_init.h"
#include "packets/sasl_mechanisms.h"
#include "packets/sasl_outcome.h"
#include "packets/sasl_response.h"
#include "packets/amqp_open.h"
#include "packets/amqp_begin.h"
#include "packets/amqp_flow.h"
#include "packets/amqp_attach.h"
#include "packets/amqp_detach.h"
#include "packets/amqp_end.h"
#include "packets/amqp_close.h"
#include "packets/amqp_transfer.h"
#include "packets/amqp_disposition.h"
#include "wrappers/unwrapper.h"
#include "wrappers/amqp_symbol.h"

struct AmqpHeader * get_header_empty (enum HeaderCode code);
void init_header(struct AmqpHeader * header, struct TlvAmqp * list);
static struct AmqpOutcome * get_outcome(struct TlvAmqp tlv);

struct AmqpHeader * get_sasl(char * data, int * i) {

	struct TlvAmqp * list = get_tlv(data, i);

	if (list->constructor->code != LIST_0 && list->constructor->code != LIST_8 && list->constructor->code != LIST_32)
		printf("Received sasl-header with malformed arguments, with incorrect code : %i\n", list->constructor->code);

	enum HeaderCode heade_code = list->constructor->tlv->value_array[0];
	struct AmqpHeader * header = get_header_empty (heade_code);
	init_header(header, list);
	return header;
}

struct AmqpHeader * get_amqp(char * data, int * i) {

	struct TlvAmqp * list = get_tlv(data, i);

		if (list->constructor->code != LIST_0 && list->constructor->code != LIST_8 && list->constructor->code != LIST_32)
			printf("Received amqp-header with malformed arguments, with incorrect code : %i\n", list->constructor->code);

		enum HeaderCode heade_code = list->constructor->tlv->value_array[0];
		struct AmqpHeader * header = get_header_empty (heade_code);
		init_header(header, list);
		return header;
}

struct AmqpHeader * get_header_empty (enum HeaderCode code) {

	struct AmqpHeader * header = malloc(sizeof(struct AmqpHeader));
	header->code = code;
	return header;
}

struct SectionEntry * get_section(char * data, int * i) {

	struct TlvAmqp * list = get_tlv(data, i);
	struct SectionEntry * section = NULL;
	enum SectionCode code = list->constructor->tlv->value_array[0];
	switch (code)
	{
	case APPLICATION_PROPERTIES:{
		//not imlemented
		break;
	}
	case DATA: {
		section = malloc(sizeof (struct SectionEntry));
		section->code = code;
		struct AmqpData * amqp_data = malloc(sizeof(struct AmqpData));
		amqp_data->data = unwrap_binary(list);
		amqp_data->data_length = list->length;
		section->section = amqp_data;
		break;
	}
	case DELIVERY_ANNOTATIONS: {
		//not imlemented
		break;
	}
	case FOOTER: {
		//not imlemented
		break;
	}
	case HEADER: {
		//not imlemented
		break;
	}
	case MESSAGE_ANNOTATIONS: {
		//not imlemented
		break;
	}
	case PROPERTIES: {
		//not imlemented
		break;
	}
	case SEQUENCE: {
		//not imlemented
		break;
	}
	case VALUE: {
		//not imlemented
		break;
	}
	default:
		printf("Received header with unrecognized message section code\n");
	}

	return section;
}

void init_header(struct AmqpHeader * header, struct TlvAmqp * list) {

	switch (header->code)
	{
		case CHALLENGE: {
			struct SaslChallenge * challenge = malloc(sizeof(struct SaslChallenge));

			int count = *(list->count);
			if (count == 0) {
				printf("Received malformed SASL-Challenge header: challenge can't be null\n");
				return;
			}
			if (count > 1) {
				printf("Received malformed SASL-Challenge header. Invalid number of arguments: %i\n",count);
				return;
			}
			if (count > 0)
			{
				if (&(list->list[0]) == NULL) {
					printf("Received malformed SASL-Challenge header: challenge can't be null");
					return;
				}
				struct TlvAmqp * element = &(list->list[0]);
				challenge->challenge = unwrap_binary(element);
			}

			header->packet = challenge;
			break;
		}
		case INIT: {
			struct SaslInit * init = malloc(sizeof(struct SaslInit));

			int count = *(list->count);
			if (count == 0) {
				printf("Received malformed SASL-Init header: mechanism can't be null\n");
				return;
			}
			if (count > 3) {
				printf("Received malformed SASL-Init header. Invalid number of arguments: %i\n", count);
				return;
			}
			if (count > 0)
			{
				struct TlvAmqp * element = &(list->list[0]);
				if (element == NULL) {
					printf("Received malformed SASL-Init header: mechanism can't be null\n");
					return;
				}
				init->mechanism = unwrap_symbol(element);
			}
			if (count > 1)
			{
				struct TlvAmqp * element = &(list->list[1]);
				if (element->code != 64)
					init->initial_response = unwrap_binary(element);
			}
			if (count > 2)
			{
				struct TlvAmqp * element = &(list->list[2]);
				if (element->code != 64)
					init->host_name = unwrap_string(element);
			}

			header->packet = init;
			break;
		}
		case MECHANISMS: {
			struct SaslMechanisms * mechanisms = malloc(sizeof(struct SaslMechanisms));

			if (*(list->count) > 0)
			{
				struct TlvAmqp * element = &(list->list[0]);
				//printf("here2 :  %x\n", list->list[0].list[0].value_array[0]);
				if (element->code != 64) {
					mechanisms->mechanisms = (struct AmqpSymbol * )(unwrap_array(element)[0]);
					//printf("here4 : %x\n",mechanisms->mechanisms->value[0]);
				}
			}


			header->packet = mechanisms;
			break;
		}
		case OUTCOME: {

			struct SaslOutcome * outcome = malloc(sizeof(struct SaslOutcome));

			if (*(list->count) == 0) {
				printf("Received malformed SASL-Outcome header: code can't be null\n");
				return;
			}

			if (*(list->count) > 2) {
				printf("Received malformed SASL-Outcome header. Invalid number of arguments: %i\n", *(list->count));
				return;
			}

			if (*(list->count) > 0)
			{
				struct TlvAmqp * element = &(list->list[0]);
				if (element == NULL) {
					printf("Received malformed SASL-Outcome header: code can't be null\n");
					return;
				}
				outcome->outcome_code = *unwrap_ubyte(element);
			}

			if (*(list->count) > 1)
			{
				struct TlvAmqp * element = &(list->list[0]);
				if (element->code != 64)
					outcome->additional_data = unwrap_binary(element);
			}

			header->packet = outcome;
			break;
		}
		case RESPONSE: {
			struct SaslResponse * response = malloc(sizeof(struct SaslResponse));
			header->packet = response;
			break;
		}
		case OPEN : {
			struct AmqpOpen * open = malloc(sizeof(struct AmqpOpen));

			if (*(list->count) == 0) {
				printf("Received malformed Open header: container id can't be null\n");
				return;
			}

			if (*(list->count) > 10) {
				printf("Received malformed Open header. Invalid number of arguments: %i\n", *(list->count));
			}

			struct TlvAmqp * element = &(list->list[0]);
			if (element == NULL) {
				printf("Received malformed Open header: container id can't be null\n");
				return;
			}
			open->container_id = unwrap_string(element);

			if (*(list->count) > 1)
			{
				element = &(list->list[1]);
				if (element->code != 64)
					open->hostname = unwrap_string(element);
			}
			if (*(list->count) > 2)
			{
				element = &(list->list[2]);
				if (element->code != 64)
					open->max_frame_size = unwrap_uint(element);
			}
			if (*(list->count) > 3)
			{
				element = &(list->list[3]);
				if (element->code != 64)
					open->channel_max = unwrap_ushort(element);
			}
			if (*(list->count) > 4)
			{
				element = &(list->list[4]);
				if (element->code != 64)
					open->idle_timeout = unwrap_uint(element);
			}
			if (*(list->count) > 5)
			{
				element = &(list->list[5]);
				if (element->code != 64)
					open->outgoing_locales = (struct AmqpSymbol * )(unwrap_array(element)[0]);
			}
			if (*(list->count) > 6)
			{
				element = &(list->list[6]);
				if (element->code != 64)
					open->incoming_locales = (struct AmqpSymbol * )(unwrap_array(element)[0]);
			}
			if (*(list->count) > 7)
			{
				element = &(list->list[7]);
				if (element->code != 64)
					open->offered_capabilities = (struct AmqpSymbol * )(unwrap_array(element)[0]);
			}
			if (*(list->count) > 8)
			{
				element = &(list->list[8]);
				if (element->code != 64)
					open->desired_capabilities = (struct AmqpSymbol * )(unwrap_array(element)[0]);
			}
			if (*(list->count) > 9)
			{
				element = &(list->list[9]);
				if (element->code != 64)
					open->properties = (struct AmqpSymbolVoidEntry * )unwrap_map(element);
			}

			header->packet = open;
			break;

		}
		case BEGIN : {
			struct AmqpBegin * begin = malloc(sizeof(struct AmqpBegin));
			if (*(list->count) < 4)
				printf("Received malformed Begin header: mandatory fields next-outgoing-id, incoming-window and outgoing-window must not be null\n");

			if (*(list->count) > 8)
				printf("Received malformed Begin header. Invalid number of arguments: %i\n",*(list->count));

			struct TlvAmqp * element = NULL;

			if (*(list->count) > 0)
			{
				element = &(list->list[0]);
				if (element->code != 64)
					begin->remote_channel = unwrap_ushort(element);
			}
			if (*(list->count) > 1)
			{
				element = &(list->list[1]);
				if (element == NULL) {
					printf("Received malformed Begin header: next-outgoing-id can't be null\n");
					return;
				}
				begin->next_outgoing_id = unwrap_uint(element);
			}
			if (*(list->count) > 2)
			{
				element = &(list->list[2]);
				if (element == NULL) {
					printf("Received malformed Begin header: incoming-window can't be null\n");
					return;
				}
				begin->incoming_window = unwrap_uint(element);
			}
			if (*(list->count) > 3)
			{
				element = &(list->list[3]);
				if (element == NULL) {
					printf("Received malformed Begin header: outgoing-window can't be null\n");
					return;
				}
				begin->outgoing_window = unwrap_uint(element);
			}
			if (*(list->count) > 4)
			{
				element = &(list->list[4]);
				if (element->code != 64)
					begin->handle_max = unwrap_uint(element);
			}
			if (*(list->count) > 5)
			{
				element = &(list->list[5]);
				if (element->code != 64)
					begin->offered_capabilities = (struct AmqpSymbol * )(unwrap_array(element)[0]);
			}
			if (*(list->count) > 6)
			{
				element = &(list->list[6]);
				if (element->code != 64)
					begin->desired_capabilities = (struct AmqpSymbol * )(unwrap_array(element)[0]);
			}
			if (*(list->count) > 7)
			{
				element = &(list->list[7]);
				if (element->code != 64)
					begin->properties = (struct AmqpSymbolVoidEntry * )unwrap_map(element);
			}
			header->packet = begin;
			break;
		}
		case ATTACH : {
			struct AmqpAttach * attach = malloc(sizeof(struct AmqpAttach));

			if (*(list->count) < 3) {
				printf("Received malformed Attach header: mandatory fields name, handle and role must not be null\n");
				return;
			}

			if (*(list->count) > 14) {
				printf("Received malformed Attach header. Invalid number of arguments: %i\n", *(list->count));
				return;
			}

			struct TlvAmqp * element = NULL;

			if (*(list->count) > 0)
			{
				element = &(list->list[0]);
				if (element == NULL)
					printf("Received malformed Attach header: name can't be null\n");
				attach->name = unwrap_string(element);
			}
			if (*(list->count) > 1)
			{
				element = &(list->list[1]);
				if (element == NULL) {
					printf("Received malformed Attach header: handle can't be null\n");
					return;
				}
				attach->handle = unwrap_uint(element);
			}
			if (*(list->count) > 2)
			{
				element = &(list->list[2]);
				if (element == NULL) {
					printf("Received malformed Attach header: role can't be null\n");
					return;
				}
				attach->role = malloc(sizeof(enum RoleCode));
				*(attach->role) = *unwrap_bool(element);
			}
			if (*(list->count) > 3)
			{
				element = &(list->list[3]);
				if (element->code != 64) {
					attach->snd_settle_mode = malloc(sizeof(enum SendCode));
					*(attach->snd_settle_mode) = *unwrap_ubyte(element);
				}
			}
			if (*(list->count) > 4)
			{
				element = &(list->list[4]);
				if (element->code != 64) {
					attach->rcv_settle_mode = malloc(sizeof(enum ReceiveCode));
					*(attach->rcv_settle_mode) = *unwrap_ubyte(element);
				}
			}
			if (*(list->count) > 5)
			{
				element = &(list->list[5]);
				if (element->code != 64)
				{
					enum AmqpType code = element->code;
					if (code != LIST_0 && code != LIST_8 && code != LIST_32) {
						printf("Expected type SOURCE - received: %i\n", element->code);
						return;
					}
					struct AmqpSource * source = malloc(sizeof(struct AmqpSource));

					if (*(list->list[5].count) > 0)
					{
						element = &(list->list[5].list[0]);
						if (element->code != 64) {
							source->address = unwrap_string(element);
						}
					}
					if (*(list->list[5].count) > 1)
					{
						element = &(list->list[5].list[1]);
						if (element->code != 64) {
							source->durable = malloc(sizeof(enum TerminusDurability));
							*(source->durable) = *unwrap_uint(element);
						}
					}
					if (*(list->list[5].count) > 2)
					{
						element = &(list->list[5].list[2]);
						if (element->code != 64) {
							source->expiry_period = unwrap_symbol(element)->value;
						}
					}
					if (*(list->list[5].count) > 3)
					{
						element = &(list->list[5].list[3]);
						if (element->code != 64) {
							source->timeout = unwrap_uint(element);
						}
					}
					if (*(list->list[5].count) > 4)
					{
						element = &(list->list[5].list[4]);
						if (element->code != 64) {
							source->dynamic = unwrap_bool(element);
						}
					}
					if (*(list->list[5].count) > 5)
					{
						element = &(list->list[5].list[5]);
						if (element -> code != 64)
						{
							if (source->dynamic != NULL)
							{
								if (*(source->dynamic))
									source->dynamic_node_properties = (struct AmqpSymbolVoidEntry *) unwrap_map(element);
								else {
									printf("Received malformed Source: dynamic-node-properties can't be specified when dynamic flag is false\n");
									return;
								}
							}
							else{
								printf("Received malformed Source: dynamic-node-properties can't be specified when dynamic flag is not set\n");
								return;
							}
						}
					}
					if (*(list->list[5].count) > 6)
					{
						element = &(list->list[5].list[6]);
						if (element->code != 64)
							source->distribution_mode = unwrap_symbol(element)->value;
					}
					if (*(list->list[5].count) > 7)
					{
						element = &(list->list[5].list[7]);
						if (element->code != 64)
							source->filter = (struct AmqpSymbolVoidEntry*)unwrap_map(element);
					}
					if (*(list->list[5].count) > 8)
					{
						element = &(list->list[5].list[8]);
						if (element->code != 64)
						{
							enum AmqpType code = element->code;
							if (code != LIST_0 && code != LIST_8 && code != LIST_32) {
								printf("Expected type 'OUTCOME' - received: %i \n",element->code);
								return;
							}

							//get outcome
							struct AmqpOutcome * outcome = get_outcome(*element);
							source->default_outcome = outcome;
							//add arguments
						}
					}
					if (*(list->list[5].count) > 9)
					{
						element = &(list->list[5].list[9]);
						if (element->code != 64)
							source->outcomes = (struct AmqpSymbol*) unwrap_array(element);
					}
					if (*(list->list[5].count) > 10)
					{
						element = &(list->list[5].list[10]);
						if (element->code != 64)
							source->capabilities = (struct AmqpSymbol*)unwrap_array(element);
					}

					attach->source = source;
				}
			}
			if (*(list->count) > 6)
			{
				element = &(list->list[6]);
				if (element->code != 64)
				{
					enum AmqpType code = element->code;
					if (code != LIST_0 && code != LIST_8 && code != LIST_32)
						printf("Expected type TARGET - received: %i\n", element->code);

					struct AmqpTarget * target = malloc(sizeof(struct AmqpTarget));

					if (*(list->list[6].count) > 0)
					{
						element = &(list->list[6].list[0]);
						if (element->code != 64) {
							target->address = unwrap_string(element);
						}
					}
					if (*(list->list[6].count) > 1)
					{
						element = &(list->list[6].list[1]);
						if (element->code != 64) {
							target->durable = malloc(sizeof(enum TerminusDurability));
							*(target->durable)= *unwrap_uint(element);
						}
					}
					if (*(list->list[6].count) > 2)
					{
						element = &(list->list[6].list[2]);
						if (element->code != 64) {
							target->expiry_period = unwrap_symbol(element)->value;
						}
					}
					if (*(list->list[6].count) > 3)
					{
						element = &(list->list[6].list[3]);
						if (element->code != 64) {
							target->timeout = unwrap_uint(element);
						}
					}
					if (*(list->list[6].count) > 4)
					{
						element = &(list->list[6].list[4]);
						if (element->code != 64) {
							target->dynamic = unwrap_bool(element);
						}
					}
					if (*(list->list[6].count) > 5)
					{
						element = &(list->list[6].list[5]);
						if (element -> code != 64)
						{
							if (target->dynamic != NULL)
							{
								if (*(target->dynamic))
									target->dynamic_node_properties = (struct AmqpSymbolVoidEntry * )unwrap_map(element);
								else {
									printf("Received malformed Target: dynamic-node-properties can't be specified when dynamic flag is false\n");
									return;
								}
							}
							else{
								printf("Received malformed Target: dynamic-node-properties can't be specified when dynamic flag is not set\n");
								return;
							}
						}
					}
					if (*(list->list[6].count) > 6)
					{
						element = &(list->list[6].list[6]);
						if (element->code != 64)
							target->capabilities = (struct AmqpSymbol * )unwrap_array(element);
					}
				}
			}
			if (*(list->count) > 7)
			{
				element = &(list->list[7]);
				if (element->code != 64)
					attach->unsettled = (struct AmqpSymbolVoidEntry *)unwrap_map(element);
			}
			if (*(list->count) > 8)
			{
				element = &(list->list[8]);
				if (element->code != 64)
					attach->incomplete_unsettled = unwrap_bool(element);
			}
			if (*(list->count) > 9)
			{
				element = &(list->list[9]);
				if (element->code != 64)
					attach->initial_delivery_count = unwrap_uint(element);

				else if (*(attach->role) == SENDER) {
					printf("Received an attach header with a null initial-delivery-count\n");
					return;
				}
			}
			if (*(list->count) > 10)
			{
				element = &(list->list[10]);
				if (element->code != 64){
					attach->max_message_size = unwrap_ulong(element);
				}
			}
			if (*(list->count) > 11)
			{
				element = &(list->list[11]);
				if (element->code != 64)
					attach->offered_capabilities = (struct AmqpSymbol *)unwrap_array(element);
			}
			if (*(list->count) > 12)
			{
				element = &(list->list[12]);
				if (element->code != 64)
					attach->desired_capabilities = (struct AmqpSymbol *)unwrap_array(element);
			}
			if (*(list->count) > 13)
			{
				element = &(list->list[13]);
				if (element->code != 64)
					attach->properties = (struct AmqpSymbolVoidEntry *)unwrap_map(element);
			}

			header->packet = attach;
			break;
		}
		case FLOW : {
			struct AmqpFlow * flow = malloc(sizeof(struct AmqpFlow));
			header->packet = flow;
			break;
		}
		case TRANSFER : {
			struct AmqpTransfer * transfer = malloc(sizeof(struct AmqpTransfer));

			if (*(list->count) == 0)
				printf("Received malformed Transfer header: handle can't be null\n");

			if (*(list->count) > 11)
				printf("Received malformed Transfer header. Invalid number of arguments: %i \n", *(list->count));

			struct TlvAmqp * element = NULL;

			if (*(list->count) > 0)
			{
				element = &(list->list[0]);
				if (element->code == 64)
					printf("Received malformed Transfer header: handle can't be null\n");
				transfer->handle = unwrap_uint(element);
			}
			if (*(list->count) > 1)
			{
				element = &(list->list[1]);
				if (element->code != 64)
					transfer->delivery_id = unwrap_uint(element);
			}
			if (*(list->count) > 2)
			{
				element = &(list->list[2]);
				if (element->code != 64)
					transfer->delivery_tag = unwrap_binary(element);
			}
			if (*(list->count) > 3)
			{
				element = &(list->list[3]);
				if (element->code != 64) {
					transfer->message_format = malloc(sizeof(struct AmqpMessageFormat));
					transfer->message_format->message_format = *unwrap_uint(element);
				}
			}
			if (*(list->count) > 4)
			{
				element = &(list->list[4]);
				transfer->settled = NULL;
				if (element->code != 64)
					transfer->settled = unwrap_bool(element);
			}
			if (*(list->count) > 5)
			{
				element = &(list->list[5]);
				if (element->code != 64)
					transfer->more = unwrap_bool(element);
			}
			if (*(list->count) > 6)
			{
				element = &(list->list[6]);
				if (element->code != 64){
					transfer->rcv_settle_mode = malloc(sizeof(enum ReceiveCode));
					*(transfer->rcv_settle_mode) = *unwrap_ubyte(element);
				}
			}
			if (*(list->count) > 7)
			{
				element = &(list->list[7]);
				if (element->code != 64)
				{
					enum AmqpType code = element->code;
					if (code != LIST_0 && code != LIST_8 && code != LIST_32) {
						printf("Expected type 'STATE' - received: %i\n", code);

					}
					transfer->state = malloc(sizeof(struct AmqpState));
					transfer->state->code = element->constructor->tlv->value_array[0];
				}
			}
			if (*(list->count) > 8)
			{
				element = &(list->list[8]);
				if (element->code != 64)
					transfer->resume = unwrap_bool(element);
			}
			if (*(list->count) > 9)
			{
				element = &(list->list[9]);
				if (element->code != 64)
					transfer->aborted = unwrap_bool(element);
			}
			if (*(list->count) > 10)
			{
				element = &(list->list[10]);
				if (element->code != 64)
					transfer->batchable = unwrap_bool(element);
			}

			header->packet = transfer;
			break;
		}
		case DISPOSITION : {
			struct AmqpDisposition * disposition = malloc(sizeof(struct AmqpDisposition));

			if (*(list->count) < 2)
				printf("Received malformed Disposition header: role and first can't be null\n");

			if (*(list->count) > 6)
				printf("Received malformed Disposition header. Invalid number of arguments: %i \n", *(list->count));

			struct TlvAmqp * element = NULL;

			if (*(list->count) > 0)
			{
				element = &(list->list[0]);
				if (element->code == 64) {
					printf("Received malformed Disposition header: role can't be null\n");
					return;
				}
				disposition->role = malloc(sizeof(enum RoleCode));
				*(disposition->role) = *unwrap_bool(element);
			}
			if (*(list->count) > 1)
			{
				element = &(list->list[1]);
				if (element->code == 64)
					printf("Received malformed Disposition header: first can't be null\n");
				disposition->first = unwrap_uint(element);
			}
			if (*(list->count) > 2)
			{
				element = &(list->list[2]);
				if (element->code != 64)
					disposition->last = unwrap_uint(element);
			}
			if (*(list->count) > 3)
			{
				element = &(list->list[3]);
				if (element->code != 64)
					disposition->settled = unwrap_bool(element);
			}
			if (*(list->count) > 4)
			{
				element = &(list->list[4]);
				if (element->code != 64)
				{
					enum AmqpType code = element->code;
					if (code != LIST_0 && code != LIST_8 && code != LIST_32) {
						printf("Expected type 'STATE' - received: %i\n", code);
						return;
					}
					disposition->state = malloc(sizeof(struct AmqpState));
					disposition->state->code = element->constructor->tlv->value_array[0];
				}
			}
			if (*(list->count) > 5)
			{
				element = &(list->list[5]);
				if (element->code != 64)
					disposition->batchable = unwrap_bool(element);
			}

			header->packet = disposition;
			break;
		}
		case DETACH : {
			struct AmqpDetach * detach = malloc(sizeof(struct AmqpDetach));

			if (*(list->count) == 0) {
				printf("Received malformed Detach header: handle can't be null\n");
				return;
			}

			if (*(list->count) > 3) {
				printf("Received malformed Detach header. Invalid number of arguments: %i \n", *(list->count));
				return;
			}

			struct TlvAmqp * element = NULL;
			if (*(list->count) > 0)
			{
				element = &(list->list[0]);
				if (element->code == 64){
					printf("Received malformed Detach header: handle can't be null");
					return;
				}
				detach->handle = unwrap_uint(element);
			}
			if (*(list->count) > 1)
			{
				element = &(list->list[1]);
				if (element->code != 64)
					detach->closed = unwrap_bool(element);
			}
			if (*(list->count) > 2)
			{
				element = &(list->list[1]);
				if (element->code != 64)
				{
					enum AmqpType code = element->code;
					if (code != LIST_0 && code != LIST_8 && code != LIST_32) {
						printf("Expected type ERROR - received: %i\n", element->code);
						return;
					}
					struct AmqpError * error = malloc(sizeof(struct AmqpError));

					if (*(list->list[2].count) > 0)
					{
						element = &(list->list[5].list[0]);
						if (element->code != 64) {
							error->condition = malloc(sizeof(enum ErrorCode));
							*(error->condition) = atoi(unwrap_symbol(element)->value);
						}
					}
					if (*(list->list[2].count) > 1)
					{
						element = &(list->list[5].list[1]);
						if (element->code != 64) {
							error->description = unwrap_string(element);
						}
					}
					if (*(list->list[2].count) > 2)
					{
						element = &(list->list[5].list[2]);
						if (element->code != 64) {
							error->info = (struct AmqpSymbolVoidEntry *)unwrap_map(element);
						}
					}
					detach->error = error;
				}
			}

			header->packet = detach;
			break;
		}
		case END : {
			struct AmqpEnd * response = malloc(sizeof(struct AmqpEnd));
			header->packet = response;
			break;
		}
		case CLOSE : {
			struct AmqpClose * response = malloc(sizeof(struct AmqpClose));
			header->packet = response;
			break;
		}
		default:
			printf("Received header with unrecognized arguments code : %i\n", header->code);
	}
}

static struct AmqpOutcome * get_outcome(struct TlvAmqp tlv)
{
	struct AmqpOutcome * outcome = malloc(sizeof(struct AmqpOutcome));
	//from descriptor of constructor
	enum StateCode code = tlv.constructor->tlv->value_array[0];
	switch (code)
	{
		case AMQP_ACCEPTED: {
			outcome->code = AMQP_ACCEPTED;
			break;
		}
		case AMQP_MODIFIED: {
			outcome->code = AMQP_MODIFIED;
			break;
		}
		case AMQP_RECEIVED: {
			outcome->code = AMQP_RECEIVED;
			break;
		}
		case AMQP_REJECTED: {
			outcome->code = AMQP_REJECTED;
			break;
		}
		case AMQP_RELEASED: {
			outcome->code = AMQP_RELEASED;
			break;
		}
		default:{
			printf("Received header with unrecognized state code : %x\n",code);
			return NULL;
		}
	}

	return outcome;
}
