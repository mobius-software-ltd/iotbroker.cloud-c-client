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
#include <uuid/uuid.h>
#include "../avps/amqp_type.h"
#include "../tlv/tlv_amqp.h"
#include "amqp_symbol.h"
#include "amqp_decimal.h"
#include "../../helpers.h"
#include "amqp_symbol.h"


static void * unwrap(struct TlvAmqp * value);

char * unwrap_binary(struct TlvAmqp * tlv) {
	if (tlv->code != BINARY_8 && tlv->code != BINARY_32) {
		printf("Error trying to parse BINARY: received code %i\n", tlv->code);
		return NULL;
	}
	return (char*)tlv->value_array;
}

struct AmqpSymbol * unwrap_symbol(struct TlvAmqp * tlv) {

	struct AmqpSymbol * symbol = malloc(sizeof(struct AmqpSymbol));

	if (tlv->code != SYMBOL_8 && tlv->code != SYMBOL_32) {
		printf("Error trying to parse SYMBOL: received %i\n", tlv->code);
		return NULL;
	}
	//symbol->value = (char*)tlv->value_array;
	char * str_value = malloc((1+(tlv->length))*sizeof(char));
	memcpy(str_value, tlv->value_array, tlv->length);
	str_value[tlv->length] = '\0';
	symbol->value = str_value;
	return symbol;
}

char * unwrap_string (struct TlvAmqp * tlv) {

	if (tlv->code != STRING_8 && tlv->code != STRING_32) {
		printf("Error trying to parse STRING: received %i\n", tlv->code);
		return NULL;
	}
	char * str_value = malloc((1+(tlv->length))*sizeof(char));
	memcpy(str_value, tlv->value_array, tlv->length);
	str_value[tlv->length] = '\0';
	return str_value;

}

short * unwrap_ubyte(struct TlvAmqp * tlv)
{
	if (tlv->code != UBYTE) {
		printf("Error trying to parse UBYTE: received %i\n",tlv->code);
		return NULL;
	}
	//FIXME Is it correct?
	short * short_value = malloc(sizeof(short));
	*short_value = tlv->value_array[0];

	return short_value;
}

int * unwrap_bool(struct TlvAmqp * tlv)
{
	int * value = malloc(sizeof(int));
	switch (tlv->code)
	{
		case BOOLEAN: {
			char val = tlv->value_array[0];
			if (val == 0)
				*value = 0;
			else if (val == 1)
				*value = 1;
			else {
				printf("Invalid Boolean type value: %i\n",val);
				free(value);
				value = NULL;
			}
			break;
		}
		case BOOLEAN_TRUE:{
			*value = 1;
			break;
		}
		case BOOLEAN_FALSE: {
			*value = 0;
			break;
		}
		default:{
			printf("Error trying to parse BOOLEAN: received %i\n",tlv->code);
			free(value);
			value = NULL;
		}
	}
	return value;
}

char * unwrap_byte(struct TlvAmqp * tlv)
{
	if (tlv->code != BYTE) {
		printf("Error trying to parse BYTE: received %i\n", tlv->code);
		return NULL;
	}
	return (char*)tlv->value_array;
}

int * unwrap_char(struct TlvAmqp * tlv)
{
	int * value = malloc(sizeof(int));
	if (tlv->code != CHAR) {
		printf("Error trying to parse CHAR: received %i\n",tlv->code);
		return NULL;
	}
	*value = get_int((char*)tlv->value_array, 0);
	return value;
}

double * unwrap_double(struct TlvAmqp * tlv)
{
	double * value = malloc(sizeof(double));
	if (tlv->code != DOUBLE) {
		printf("Error trying to parse DOUBLE: received %i\n", tlv->code);
		return NULL;
	}
	*value = get_double((char*)tlv->value_array, 0);
	return value;
}

float * unwrap_float(struct TlvAmqp * tlv) {

	float * value = malloc(sizeof(float));
	if (tlv->code != FLOAT) {
		printf("Error trying to parse FLOAT: received %i\n", tlv->code);
		return NULL;
	}
	*value = get_float((char*)tlv->value_array, 0);
	return value;
}

int * unwrap_int(struct TlvAmqp * tlv) {

	int * value = malloc(sizeof(int));
	if (tlv->code != INT && tlv->code != SMALL_INT) {
		printf("Error trying to parse INT: received %i\n", tlv->code);
		return NULL;
	}

	if (tlv->length == 0) {
		*value = 0;
	} else if (tlv->length == 1) {
		*value = tlv->value_array[0];
	} else {
		 *value = get_int((char*)tlv->value_array,0);
	}
	return value;
}

long * unwrap_long (struct TlvAmqp * tlv) {

	long * value = malloc(sizeof(long));
	if (tlv->code != LONG && tlv->code != SMALL_LONG) {
		printf("Error trying to parse LONG: received %i\n",tlv->code);
		return NULL;
	}

	if (tlv->length == 0) {
		*value = 0;
	}else if (tlv->length == 1) {
		*value = tlv->value_array[0];
	}else {
		*value = get_long((char*)tlv->value_array,0);
	}
	return value;
}

short * unwrap_short (struct TlvAmqp * tlv) {
	short * value = malloc(sizeof(short));
	if (tlv->code != AMQP_SHORT) {
		printf("Error trying to parse SHORT: received %i\n", tlv->code);
		return NULL;
	}
	*value = get_short((char*)tlv->value_array, 0);
	return value;
}

long * unwrap_uint(struct TlvAmqp * tlv) {

	long * value = malloc(sizeof(long));
	if (tlv->code != UINT && tlv->code != SMALL_UINT && tlv->code != UINT_0) {
		printf("Error trying to parse UINT: received %i\n", tlv->code);
		return NULL;
	}
	if (tlv->length == 0) {
		*value = 0;
	} else if (tlv->length == 1) {
		*value = tlv->value_array[0];
	} else {
		*value = get_int((char*)tlv->value_array, 0);
	}
	return value;
}

int * unwrap_ushort(struct TlvAmqp * tlv) {
	if (tlv->code != USHORT) {
		printf("Error trying to parse USHORT: received %i\n",tlv->code);
		return NULL;
	}
	int * value = malloc(sizeof(int));
	* value = get_short((char*)tlv->value_array,0);
	return value;
}

unsigned long * unwrap_ulong(struct TlvAmqp * tlv) {

	if (tlv->code != ULONG && tlv->code != SMALL_ULONG && tlv->code != ULONG_0) {
		printf("Error trying to parse ULONG: received %i\n", tlv->code);
		return NULL;
	}
	unsigned long * value = malloc(sizeof(unsigned long));
	if (tlv->length == 0) {
		*value = 0;
	} else if (tlv->length == 1) {
		*value = tlv->value_array[0];
	} else {
		*value = get_long((char*)tlv->value_array,0);
	}

	return value;
}

uuid_t * unwrap_uuid(struct TlvAmqp * tlv) {
	if (tlv->code != UUID) {
		printf("Error trying to parse UUID: received %i\n", tlv->code);
		return NULL;
	}
	printf("UUID not implemented yet\n");
	return NULL;
}

struct Tlv * unwrap_list(struct TlvAmqp * tlv) {
	if (tlv->code != LIST_0 && tlv->code != LIST_8 && tlv->code != LIST_32 ) {
		printf("Error trying to parse LIST: received %i\n", tlv->code);
		return NULL;
	}
	printf("LIST not implemented yet\n");
	return NULL;
}

struct TlvTlvEntry * unwrap_map(struct TlvAmqp * tlv) {
	if (tlv->code != MAP_8 && tlv->code != MAP_32) {
		printf("Error trying to parse MAP: received %i\n", tlv->code);
		return NULL;
	}
	printf("MAP not implemented yet\n");
	return NULL;
}

struct AmqpDecimal * unwrap_decimal(struct TlvAmqp * tlv) {

	if (tlv->code != DECIMAL_32 && tlv->code != DECIMAL_64 && tlv->code != DECIMAL_128) {
		printf("Error trying to parse DECIMAL: received %i\n", tlv->code);
		return NULL;
	}
	struct AmqpDecimal * value = malloc(sizeof(struct AmqpDecimal));
	value->value = (char*)tlv->value_array;
	return value;
}

void ** unwrap_array(struct TlvAmqp * tlv)	{

	if (tlv->code != ARRAY_8 && tlv->code != ARRAY_32) {
		printf("Error trying to parse ARRAY: received %i\n", tlv->code);
		return NULL;
	}

	//void ** array = malloc(tlv->length);
	void **array = malloc(* tlv->count * sizeof(void *) );
	for (int i = 0; i < *(tlv->count); i++) {
		array[i] = unwrap(&(tlv->list[i]));
	}

	//struct AmqpSymbol * symbol = (struct AmqpSymbol*) array[0];
	return array;
}

static void * unwrap(struct TlvAmqp * value) {

	//printf("UNWRAP CODE : %x\n", value->code);
	switch (value->code)
	{
		case AMQP_NULL:
			return NULL;

		case ARRAY_32:
		case ARRAY_8:
			return unwrap_array(value);

		case BINARY_32:
		case BINARY_8:
			return unwrap_binary(value);

		case UBYTE:
			return unwrap_ubyte(value);

		case BOOLEAN:
		case BOOLEAN_FALSE:
		case BOOLEAN_TRUE:
			return unwrap_bool(value);

		case BYTE:
			return unwrap_byte(value);

		case CHAR:
			return unwrap_char(value);

		case DOUBLE:
			return unwrap_double(value);

		case FLOAT:
			return unwrap_float(value);

		case INT:
		case SMALL_INT:
			return unwrap_int(value);

		case LIST_0:
		case LIST_32:
		case LIST_8:
			return unwrap_list(value);

		case LONG:
		case SMALL_LONG:
			return unwrap_long(value);

		case MAP_32:
		case MAP_8:
			return unwrap_map(value);

		case AMQP_SHORT:
			return unwrap_short(value);

		case STRING_32:
		case STRING_8:
			return unwrap_string(value);

		case SYMBOL_32:
		case SYMBOL_8:
			return unwrap_symbol(value);

		case TIMESTAMP:
			return unwrap_long(value);

		case UINT:
		case SMALL_UINT:
		case UINT_0:
			return unwrap_uint(value);

		case ULONG:
		case SMALL_ULONG:
		case ULONG_0:
			return unwrap_ulong(value);

		case USHORT:
			return unwrap_ushort(value);

		case UUID:
			return unwrap_uuid(value);

		case DECIMAL_128:
		case DECIMAL_32:
		case DECIMAL_64:
			return unwrap_decimal(value);

		default:{
			printf("Received unrecognized type\n");
			return NULL;
		}
	}
}
