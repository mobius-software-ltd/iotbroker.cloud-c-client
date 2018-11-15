/*
 * amqp_calc.c
 *
 *  Created on: Nov 7, 2018
 *      Author: konstantinnosach
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

#include "avps/amqp_type.h"
#include "tlv/tlv_amqp.h"
#include "tlv/tlv_list.h"
#include "tlv/tlv_constructor.h"
#include "header_factory.h"
#include "amqp_calc.h"


char * get_bytes (struct TlvList * list) {

	unsigned char * size_bytes = malloc(list->width*sizeof(size_bytes));
	switch (list->width)
	{
		case 0:
			break;
		default: {
			memcpy(size_bytes, &list->size, list->width);
			reverse(size_bytes, list->width);
			break;
		}
	}
	unsigned char * count_bytes = malloc(list->width*sizeof(count_bytes));
	switch (list->width)
	{
		case 0:
			break;
		default:{
			memcpy(count_bytes, &list->count, list->width);
			reverse(count_bytes, list->width);
			break;
		}
	}
	char * value_bytes = malloc((list->size /*- list->width*/)*sizeof(value_bytes));
	int pos = 0;

	for(int i = 0; i < list->count; i++){
		memcpy(&value_bytes[pos], list->list[i].bytes, list->list[i].length);
		pos += list->list[i].length;
	}
	list->length = list->constructor.length + list->width + list->width + pos;
	char * bytes = malloc(list->length * sizeof(bytes));
	pos = 0;
	memcpy(&bytes[pos], list->constructor.bytes, list->constructor.length);
	pos+=list->constructor.length;
	if (list->size > 0)
	{
		memcpy(&bytes[pos], size_bytes, list->width);
		pos+=list->width;
		memcpy(&bytes[pos], count_bytes, list->width);
		pos+=list->width;
		memcpy(&bytes[pos], value_bytes, (list->size));
	}
	return bytes;
}

void calculate_bytes(struct TlvList * tlv_list, int header_code){

	tlv_list->code = LIST_8;
	if (tlv_list->size > 255)
	{
		tlv_list->code = LIST_32;
		tlv_list->width = 4;
		tlv_list->size += 3;
	}
	tlv_list->constructor.code = tlv_list->code;
	tlv_list->constructor.tlv = malloc(sizeof(struct TlvAmqp));
	tlv_list->constructor.tlv->code = SMALL_ULONG;
	tlv_list->constructor.tlv->value_array = malloc(sizeof(tlv_list->constructor.tlv->value_array));
	tlv_list->constructor.tlv->value_array[0] = header_code;
	//calculate constructor length = value.length+simpleconstructor.lenght+ 2
	tlv_list->constructor.length = 1 + 1 + 2;
	//calculate constructor bytes
	tlv_list->constructor.bytes = malloc(tlv_list->constructor.length * sizeof(tlv_list->constructor.bytes));
	tlv_list->constructor.bytes[0] = 0;
	tlv_list->constructor.bytes[1] = tlv_list->constructor.tlv->code;
	tlv_list->constructor.bytes[2] = tlv_list->constructor.tlv->value_array[0];
	tlv_list->constructor.bytes[3] = tlv_list->constructor.code;

	//calculate length
	//tlv_list->length = tlv_list->length + tlv_list->width + tlv_list->constructor.length;
	//calculate bytes
	tlv_list->bytes = get_bytes(tlv_list);
}
