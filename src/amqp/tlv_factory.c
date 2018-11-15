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

#ifndef SRC_AMQP_TLV_FACTORY_C_
#define SRC_AMQP_TLV_FACTORY_C_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tlv/tlv_amqp.h"
#include "tlv/tlv_constructor.h"
#include "sections/tlv_tlv_entry.h"
#include "../helpers.h"

static struct TlvConstructor * get_constructor(char * buf, int * i);
static struct TlvAmqp * get_element(struct TlvConstructor * constructor, char * buf, int * i);
void init_wcs(struct TlvAmqp * tlv);

struct TlvAmqp * get_tlv (char * buf, int * i) {

	struct TlvConstructor * constructor = get_constructor(buf, i);
	struct TlvAmqp * tlv = get_element(constructor, buf, i);
	tlv->constructor = constructor;
	return tlv;

}

static struct TlvAmqp * get_element(struct TlvConstructor * constructor, char * buf, int * i) {

	struct TlvAmqp * tlv = malloc(sizeof(struct TlvAmqp));
	tlv->code = constructor->code;
	switch (constructor->code)
	{

	case AMQP_NULL: {
		tlv->code = AMQP_NULL;
		break;
	}
	case BOOLEAN_TRUE:
	case BOOLEAN_FALSE:
	case UINT_0:
	case ULONG_0: {
		tlv->value_array = malloc(sizeof(char));
		tlv->value_array[0] = 0x00;
		tlv->length = 0;
		break;
	}
	case BOOLEAN:
	case UBYTE:
	case BYTE:
	case SMALL_UINT:
	case SMALL_INT:
	case SMALL_ULONG:
	case SMALL_LONG: {
		char value_one = buf[(*i)++];
		tlv->value_array = malloc(sizeof(char));
		tlv->value_array[0] = value_one;
		tlv->length = 1;
		break;
	}
	case AMQP_SHORT:
	case USHORT: {
		tlv->value_array = malloc(2 * sizeof(char));
		memcpy(tlv->value_array, &buf[*i], 2);
		*i += 2;
		break;
	}
	case UINT:
	case INT:
	case FLOAT:
	case DECIMAL_32:
	case CHAR: {
		tlv->value_array = malloc(4 * sizeof(char));
		memcpy(tlv->value_array, &buf[*i], 4);
		*i += 4;
		tlv->length = 1;
		break;
	}
	case ULONG:
	case LONG:
	case DECIMAL_64:
	case DOUBLE:
	case TIMESTAMP: {
		tlv->value_array = malloc(8 * sizeof(char));
		memcpy(tlv->value_array, &buf[*i], 8);
		*i += 8;
		break;
	}
	case DECIMAL_128:
	case UUID: {
		tlv->value_array = malloc(16 * sizeof(char));
		memcpy(tlv->value_array, &buf[*i], 16);
		*i += 16;
		break;
	}
	case STRING_8:
	case SYMBOL_8:
	case BINARY_8: {
		int var_8_length = buf[(*i)++];
		tlv->length = var_8_length;
		tlv->value_array = malloc(var_8_length * sizeof(char));
		tlv->width = malloc(sizeof(int));
		memcpy(tlv->value_array, &buf[*i], var_8_length);
		*i += var_8_length;
		*tlv->width = var_8_length > 255 ? 4 : 1;
		break;
	}
	case STRING_32:
	case SYMBOL_32:
	case BINARY_32:{
		int var_32_length = get_int(buf, *i);
		*i += 4;
		tlv->value_array = malloc(var_32_length * sizeof(char));
		tlv->width = malloc(sizeof(int));
		memcpy(tlv->value_array, &buf[*i], var_32_length);
		*i += var_32_length;
		*tlv->width = var_32_length > 255 ? 4 : 1;
		break;
	}
	case LIST_0:{
		tlv->list = NULL;
		init_wcs(tlv);
		*tlv->width = 0;
		*tlv->count = 0;
		*tlv->size = 0;
		break;
	}
	case LIST_8: {
		int list_8_size = buf[(*i)++];
		int list_8_count = buf[(*i)++];
		struct TlvAmqp * list_8_values = malloc(list_8_count * sizeof(struct TlvAmqp));
		init_wcs(tlv);
		for (int j = 0; j < list_8_count; j++) {
			list_8_values[j] = *get_tlv(buf, i);
		}
		tlv->list = list_8_values;
		*tlv->width = 1;
		*tlv->count = list_8_count;
		*tlv->size = list_8_size;
		break;
	}
	case LIST_32: {
		int list_32_size = get_int(buf, *i);
		*i += 4;
		int list_32_count = get_int(buf, *i);
		*i += 4;
		struct TlvAmqp * list_32_values = malloc(list_32_count * sizeof(struct TlvAmqp));
		init_wcs(tlv);
		for (int j = 0; j < list_32_count; j++)
			list_32_values[j] = *get_tlv(buf, i);
		*tlv->width = 4;
		*tlv->count = list_32_count;
		*tlv->size = list_32_size;
		break;
	}
	case MAP_8: {

		int map_8_size = buf[(*i)++];
		int map_8_count = buf[(*i)++];
		struct TlvTlvEntry * map8 = malloc(map_8_count * sizeof(struct TlvTlvEntry));
		init_wcs(tlv);
		int stop8 = *i + map_8_size - 1;
		int j = 0;
		while (*i < stop8) {
			map8[j].key = get_tlv(buf, i);
			map8[j].value = get_tlv(buf, i);
			j++;
		}
		tlv->map = map8;
		*tlv->width = 1;
		*tlv->count = map_8_count;
		*tlv->size = map_8_size;

		break;
	}
	case MAP_32: {

		int map_32_size = get_int(buf, *i);
		*i += 4;
		int map_32_count = get_int(buf, *i);
		*i += 4;
		struct TlvTlvEntry * map32 = malloc(map_32_count * sizeof(struct TlvTlvEntry));
		init_wcs(tlv);
		int stop32 = *i + map_32_size - 4;
		int j = 0;
		while (*i < stop32) {
			map32[j].key = get_tlv(buf, i);
			map32[j].value = get_tlv(buf, i);
			j++;
		}
		tlv->map = map32;
		*tlv->width = 4;
		*tlv->count = map_32_count;
		*tlv->size = map_32_size;
		break;
	}
	case ARRAY_8: {

		int array_8_size = buf[(*i)++];
		int array_8_count = buf[(*i)++];
		struct TlvAmqp * arr8 = malloc(array_8_count * sizeof(struct TlvAmqp));
		init_wcs(tlv);
		struct TlvConstructor * arr_8_constructor = get_constructor(buf, i);
		for (int j = 0; j < array_8_count; j++) {
			arr8[j] = *get_element(arr_8_constructor, buf, i);
		}
		tlv->list = arr8;
		*tlv->width = 1;
		*tlv->count = array_8_count;
		*tlv->size = array_8_size;
		break;
	}
	case ARRAY_32: {

		int array_32_size = get_int(buf, *i);
		*i += 4;
		int array_32_count = get_int(buf, *i);
		*i += 4;
		struct TlvAmqp * arr32 = malloc(array_32_count * sizeof(struct TlvAmqp));
		init_wcs(tlv);
		struct TlvConstructor * arr_32_constructor = get_constructor(buf, i);
		for (int j = 0; j < array_32_count; j++)
			arr32[j] = *get_element(arr_32_constructor, buf, i);
		tlv->list = arr32;
		*tlv->width = 1;
		*tlv->count = array_32_count;
		*tlv->size = array_32_size;
		break;
	}
	default:
		break;
	}

	return tlv;
}

void init_wcs(struct TlvAmqp * tlv) {
	tlv->width = malloc(sizeof(int));
	tlv->count = malloc(sizeof(int));
	tlv->size = malloc(sizeof(int));
}

static struct TlvConstructor * get_constructor(char * buf, int * i) {

	struct TlvConstructor * constructor = malloc(sizeof(struct TlvConstructor));

	enum AmqpType code_byte = 0xFF & buf[(*i)++];

	if (code_byte == 0)
	{
		struct TlvAmqp * descriptor = get_tlv(buf, i);
		constructor->code = 0xFF & buf[(*i)++];
		constructor->tlv = descriptor;
	}
	else
	{
		constructor->code = code_byte;
	}
	//printf("constructor code is %02x\n",constructor->code);
	return constructor;
}


#endif /* SRC_AMQP_TLV_FACTORY_C_ */
