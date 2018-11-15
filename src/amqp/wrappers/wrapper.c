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
#include "../tlv/tlv_amqp.h"
#include "../tlv/tlv_list.h"
#include "../tlv/amqp_error.h"
#include "../terminus/amqp_target.h"
#include "../terminus/amqp_source.h"
#include "../amqp_calc.h"
#include "../packets/amqp_state.h"
#include "../../helpers.h"
#include "amqp_symbol.h"

static struct TlvAmqp * wrap_ubyte(short * value);
static struct TlvAmqp * _wrap_short(short * value);
struct TlvAmqp * wrap_map (void * p);
struct TlvAmqp * wrap_list (void * p);
static struct TlvAmqp * wrap_uint(long * value);
static struct TlvAmqp * _wrap_long(long * value);
static struct TlvAmqp * wrap_ushort(int * value);
static struct TlvAmqp * _wrap_int(int * value);
struct TlvAmqp * wrap_outcome (void * p);
struct TlvAmqp * wrap_array (void * p);

static void init_tlv_variable(struct TlvAmqp * tlv, int str_len) {

	tlv->width = malloc(sizeof(int));
	*(tlv->width) =  str_len > 255 ? 4 : 1;
	//set length
	//length of simple constructor is 1
	tlv->length = str_len + 1 + *(tlv->width);

	char constructor_bytes = tlv->code;
	char * width_bytes = malloc(*(tlv->width) * sizeof(width_bytes));
	if (*tlv->width == 1)
		width_bytes[0] = str_len;
	else if (*tlv->width == 4)
		memcpy(&width_bytes[0], &str_len, sizeof(int));

	char * bytes = malloc((1 + *(tlv->width) + str_len)*sizeof(bytes));
	int i = 0;
	//copy constructor bytes
	memcpy(&bytes[i++], &constructor_bytes, sizeof(char));
	//copy width bytes
	memcpy(&bytes[i], width_bytes, *tlv->width);
	i += *tlv->width;

	if (str_len > 0)
		memcpy(&bytes[i], tlv->value_array, str_len);

	tlv->bytes = bytes;
}

static void init_tlv_fixed(struct TlvAmqp * tlv, int len) {

	//length of simple constructor is 1
	tlv->length = 1 + len;
	char constructor_bytes = tlv->code;
	char * bytes = malloc((tlv->length)*sizeof(bytes));
	int i = 0;
	//copy constructor bytes
	memcpy(&bytes[i++], &constructor_bytes, sizeof(char));
	if(len > 0)
		memcpy(&bytes[i], tlv->value_array, len);
	tlv->bytes = bytes;
}

struct TlvAmqp * wrap_null () {

	struct TlvAmqp * tlv = malloc(sizeof(struct TlvAmqp));
	tlv->code = AMQP_NULL;
	tlv->length = 1;
	tlv->bytes = malloc(sizeof(char));
	memcpy(tlv->bytes, &(tlv->code), tlv->length);
	return tlv;
}

struct TlvAmqp * wrap_string (char * s) {


	size_t str_len = strlen(s);

	struct TlvAmqp * tlv = malloc(sizeof(struct TlvAmqp));
	tlv->width = malloc(sizeof(int));

	tlv->code = str_len > 255 ? STRING_32 : STRING_8;
	tlv->value_array = (unsigned char *)s;

	*tlv->width =  str_len > 255 ? 4 : 1;
	//set length
	//length of simple constructor is 1
	tlv->length = str_len + 1 + *(tlv->width);

	char constructor_bytes = tlv->code;
	char * width_bytes = malloc(*(tlv->width) * sizeof(width_bytes));
	if (*tlv->width == 1)
		width_bytes[0] = str_len;
	else if (*tlv->width == 4)
		memcpy(&width_bytes[0], &str_len, sizeof(int));

	char * bytes = malloc((1 + *(tlv->width) + str_len)*sizeof(bytes));
	int i = 0;
	//copy constructor bytes
	memcpy(&bytes[i++], &constructor_bytes, sizeof(char));
	//copy width bytes
	memcpy(&bytes[i], width_bytes, *tlv->width);
	i += *tlv->width;

	if (str_len > 0)
		memcpy(&bytes[i], s, str_len);
	tlv->bytes = bytes;

	return tlv;
}

struct TlvAmqp * wrap_symbol (struct AmqpSymbol * symbol) {

	struct TlvAmqp * tlv = malloc(sizeof(struct TlvAmqp));
	size_t str_len = strlen(symbol->value);
	tlv->value_array = malloc((str_len + 1)*sizeof(char));
	strcpy((char *)tlv->value_array, symbol->value);
	tlv->code = str_len > 255 ? SYMBOL_32 : SYMBOL_8;

	init_tlv_variable(tlv, str_len);

	return tlv;
}

struct TlvAmqp * wrap_binary (char * array, int length) {

	struct TlvAmqp * tlv = malloc(sizeof(struct TlvAmqp));
	tlv->code = length > 255 ? BINARY_32 : BINARY_8;
	tlv->value_array = (unsigned char *)array;
	init_tlv_variable(tlv, length);

	return tlv;
}

struct TlvAmqp * wrap_bool (int * value) {

	struct TlvAmqp * tlv = malloc(sizeof(struct TlvAmqp));
	tlv->code = *value ? BOOLEAN_TRUE : BOOLEAN_FALSE;
	tlv->value_array = malloc(sizeof(int));
	//tlv->value_array[0] = 0x00;
	init_tlv_fixed(tlv, 0);

	return tlv;
}

struct TlvAmqp * wrap_long (long * value) {
	if(*value >= 0) {
		return wrap_uint(value);
	} else {
		return _wrap_long(value);
	}
}

struct TlvAmqp * wrap_short (short * value) {
	if(*value >= 0) {
		return wrap_ubyte(value);
	} else {
		return _wrap_short(value);
	}
}
static struct TlvAmqp * wrap_ubyte(short * value) {

	int len = 1;
	struct TlvAmqp * tlv = malloc(sizeof(struct TlvAmqp));
	tlv->code = UBYTE;
	tlv->value_array = malloc(len*sizeof(int));
	tlv->value_array[0] = *value;
	init_tlv_fixed(tlv, len);
	return tlv;
}

static struct TlvAmqp * _wrap_short(short * value) {

	int len = 2;
	struct TlvAmqp * tlv = malloc(sizeof(struct TlvAmqp));
	tlv->code = AMQP_SHORT;
	tlv->value_array = malloc(len * sizeof(int));
	memcpy(tlv->value_array, value, len);
	reverse(tlv->value_array, len);
	init_tlv_fixed(tlv, len);
	return tlv;
}


static struct TlvAmqp * wrap_uint(long * value) {

	struct TlvAmqp * tlv = malloc(sizeof(struct TlvAmqp));

	int len = 0;
	if (*value == 0) {
		len = 0;
		tlv->value_array = malloc(sizeof(tlv->value_array));
		tlv->value_array[0] = 0X00;
	} else if (*value > 0 && *value <= 255) {
		len = 1;
		tlv->value_array = malloc(sizeof(tlv->value_array));
		tlv->value_array[0] = *value;
	} else {
		len = 4;
		tlv->value_array = malloc(4*sizeof(tlv->value_array));
		memcpy(tlv->value_array, value, len);
		reverse(tlv->value_array, len);
	}

	if (len == 0)
		tlv->code = UINT_0;
	else if (len == 1)
		tlv->code = SMALL_UINT;
	else if (len > 1)
		tlv->code = UINT;

	//length of simple constructor is 1
	tlv->length = 1 + len;
	char constructor_bytes = tlv->code;
	char * bytes = malloc((tlv->length)*sizeof(bytes));
	int i = 0;
	//copy constructor bytes
	memcpy(&bytes[i++], &constructor_bytes, sizeof(char));
	memcpy(&bytes[i], tlv->value_array, len);

	tlv->bytes = bytes;

	return tlv;
}

static struct TlvAmqp * _wrap_long(long * value) {

	struct TlvAmqp * tlv = malloc(sizeof(struct TlvAmqp));
	//convert long to char array
	int len = 0;
	if (*value == 0) {
		len = 1;
		tlv->value_array = malloc(sizeof(tlv->value_array));
		tlv->value_array[0] = 0X00;
	} else if (*value >= -128 && *value <= 127) {
		len = 1;
		tlv->value_array = malloc(sizeof(tlv->value_array));
		tlv->value_array[0] = *value;
	} else {
		len = 8;
		tlv->value_array = malloc(len*sizeof(tlv->value_array));
		memcpy(tlv->value_array, value, len);
		reverse(tlv->value_array, 2);
	}

	tlv->code = len > 1 ? LONG : SMALL_LONG;
	//length of simple constructor is 1
	tlv->length = 1 + len;
	char constructor_bytes = tlv->code;
	char * bytes = malloc((tlv->length)*sizeof(bytes));
	int i = 0;
	//copy constructor bytes
	memcpy(&bytes[i++], &constructor_bytes, sizeof(char));
	memcpy(&bytes[i], &tlv->value_array, len);

	tlv->bytes = bytes;

	return tlv;
}

struct TlvAmqp * wrap_int (int * value) {
	if(*value >= 0) {
		return wrap_ushort(value);
	} else {
		return _wrap_int(value);
	}
}

static struct TlvAmqp * wrap_ushort(int * value) {
	int len = 2;
	struct TlvAmqp * tlv = malloc(sizeof(struct TlvAmqp));
	tlv->code = USHORT;
	tlv->value_array = malloc(len*sizeof(tlv->value_array));
	memcpy(tlv->value_array, value, len);
	tlv->length = 1 + len;
	char constructor_bytes = tlv->code;
	char * bytes = malloc((tlv->length)*sizeof(bytes));
	int i = 0;
	//copy constructor bytes
	memcpy(&bytes[i++], &constructor_bytes, sizeof(char));
	memcpy(&bytes[i], tlv->value_array, len);

	tlv->bytes = bytes;

	return tlv;

}

static struct TlvAmqp * _wrap_int(int * value) {

	struct TlvAmqp * tlv = malloc(sizeof(struct TlvAmqp));
	//convert int
	int len = 0;
	if (*value == 0) {
		len = 1;
		tlv->value_array = malloc(sizeof(tlv->value_array));
		tlv->value_array[0] = 0X00;
	} else if (*value >= -128 && *value <= 127) {
		len = 1;
		tlv->value_array = malloc(sizeof(tlv->value_array));
		tlv->value_array[0] = *value;
	} else {
		len = 4;
		tlv->value_array = malloc(len*sizeof(tlv->value_array));
		memcpy(tlv->value_array, value, len);
		reverse(tlv->value_array, 2);
	}
	tlv->code = len > 1 ? INT : SMALL_INT;
	tlv->length = 1 + len;
	char constructor_bytes = tlv->code;
	char * bytes = malloc((tlv->length)*sizeof(bytes));
	int i = 0;
	//copy constructor bytes
	memcpy(&bytes[i++], &constructor_bytes, sizeof(char));
	//copy data bytes
	memcpy(&bytes[i], tlv->value_array, len);
	tlv->bytes = bytes;

	return tlv;

}

struct TlvAmqp * wrap_amqp_source (struct AmqpSource * source) {

	struct TlvList * tlv_list = malloc(sizeof(struct TlvList));
	tlv_list->list = malloc(11 * sizeof(struct TlvAmqp));
	int count_not_null = 0;
	tlv_list->size = 0;
	int list_index = 0;
	tlv_list->width = 1;

	struct TlvAmqp * address = NULL;
	if (source->address != NULL){
		address = wrap_string((char*)source->address);
		count_not_null++;
		tlv_list->count = 1;
		tlv_list->size += address->length;
		tlv_list->list[list_index++] = *address;
	} else {
		tlv_list->list[list_index++] = *wrap_null();
	}

	struct TlvAmqp * durable = NULL;
	if (source->durable != NULL){
		long * _durable = malloc(sizeof(long));
		*_durable = *(source->durable);
		durable = wrap_long(_durable);
		count_not_null++;
		tlv_list->count = 2;
		tlv_list->size += durable->length;
		tlv_list->list[list_index++] = *durable;
	} else {
		tlv_list->list[list_index++] = *wrap_null();
	}

	struct TlvAmqp * expiry_period = NULL;
	if (source->expiry_period != NULL){
		struct AmqpSymbol * symbol = malloc(sizeof(struct AmqpSymbol));
		symbol->value = string_terminus_expiry_policy(*(source->expiry_period));
		expiry_period = wrap_symbol(symbol);
		count_not_null++;
		tlv_list->count = 3;
		tlv_list->size += expiry_period->length;
		tlv_list->list[list_index++] = *expiry_period;
	} else {
		tlv_list->list[list_index++] = *wrap_null();
	}

	struct TlvAmqp * timeout = NULL;
	if (source->timeout != NULL){
		timeout = wrap_long(source->timeout);
		count_not_null++;
		tlv_list->count = 4;
		tlv_list->size += timeout->length;
		tlv_list->list[list_index++] = *timeout;
	} else {
		tlv_list->list[list_index++] = *wrap_null();
	}

	struct TlvAmqp * dynamic = NULL;
	if (source->dynamic != NULL){
		dynamic = wrap_bool(source->dynamic);
		count_not_null++;
		tlv_list->count = 5;
		tlv_list->size += dynamic->length;
		tlv_list->list[list_index++] = *dynamic;
	} else {
		tlv_list->list[list_index++] = *wrap_null();
	}

	struct TlvAmqp * dynamic_node_properties = NULL;

	if (source->dynamic_node_properties != NULL) {
		if (source->dynamic != NULL) {
			if (*(source->dynamic)) {
				dynamic_node_properties = wrap_map(source->dynamic_node_properties);
				count_not_null++;
				tlv_list->count = 6;
				tlv_list->size += dynamic_node_properties->length;
				tlv_list->list[list_index++] = *dynamic_node_properties;
			} else {
				printf("Source's dynamic-node-properties can't be specified when dynamic flag is false\n");
				return NULL;
			}
		} else {
			printf("Source's dynamic-node-properties can't be specified when dynamic flag is not set\n");
			return NULL;
		}
	} else {
		tlv_list->list[list_index++] = *wrap_null();
	}

	struct TlvAmqp * distribution_mode = NULL;
	if (source->distribution_mode != NULL){
		struct AmqpSymbol * symbol = malloc(sizeof(struct AmqpSymbol));
		symbol->value = string_distribution_mode(*(source->distribution_mode));
		expiry_period = wrap_symbol(symbol);
		count_not_null++;
		tlv_list->count = 7;
		tlv_list->size += distribution_mode->length;
		tlv_list->list[list_index++] = *distribution_mode;
	} else {
		tlv_list->list[list_index++] = *wrap_null();
	}

	struct TlvAmqp * filter = NULL;
	if (source->filter != NULL){
		filter = wrap_map(source->filter);
		count_not_null++;
		tlv_list->count = 8;
		tlv_list->size += filter->length;
		tlv_list->list[list_index++] = *filter;
	} else {
		tlv_list->list[list_index++] = *wrap_null();
	}

	struct TlvAmqp * default_outcome = NULL;
	if (source->default_outcome != NULL){
		default_outcome = wrap_outcome(source->default_outcome);
		count_not_null++;
		tlv_list->count = 9;
		tlv_list->size += default_outcome->length;
		tlv_list->list[list_index++] = *default_outcome;
	} else {
		tlv_list->list[list_index++] = *wrap_null();
	}

	struct TlvAmqp * outcomes = NULL;
	if (source->outcomes != NULL){
		outcomes = wrap_array(source->outcomes);
		count_not_null++;
		tlv_list->count = 10;
		tlv_list->size += outcomes->length;
		tlv_list->list[list_index++] = *outcomes;
	} else {
		tlv_list->list[list_index++] = *wrap_null();
	}

	struct TlvAmqp * capabilities = NULL;
	if (source->capabilities != NULL){
		capabilities = wrap_array(source->capabilities);
		count_not_null++;
		tlv_list->count = 11;
		tlv_list->size += capabilities->length;
		tlv_list->list[list_index++] = *capabilities;
	} else {
		tlv_list->list[list_index++] = *wrap_null();
	}

	tlv_list->size += tlv_list->count - count_not_null;

	calculate_bytes(tlv_list, SOURCE);

	struct TlvAmqp * tlv = malloc(sizeof(struct TlvAmqp));
	tlv->bytes = tlv_list->bytes;
	tlv->length = tlv_list->length;
	return tlv;
}

struct TlvAmqp * wrap_amqp_error (struct AmqpError * error) {

	struct TlvList * tlv_list = malloc(sizeof(struct TlvList));
	tlv_list->list = malloc(7 * sizeof(struct TlvAmqp));
	int count_not_null = 0;
	tlv_list->size = 0;
	int list_index = 0;
	tlv_list->width = 1;

	struct TlvAmqp * condition = NULL;
	if (error->condition != NULL){
		struct AmqpSymbol * symbol = malloc(sizeof(struct AmqpSymbol));
		symbol->value = string_error_code(*(error->condition));
		condition = wrap_symbol(symbol);
		count_not_null++;
		tlv_list->count = 1;
		tlv_list->size += condition->length;
		tlv_list->list[list_index++] = *condition;
	} else {
		tlv_list->list[list_index++] = *wrap_null();
	}

	struct TlvAmqp * description = NULL;
	if (error->description != NULL){
		description = wrap_string(error->description);
		count_not_null++;
		tlv_list->count = 2;
		tlv_list->size += description->length;
		tlv_list->list[list_index++] = *description;
	} else {
		tlv_list->list[list_index++] = *wrap_null();
	}

	struct TlvAmqp * info = NULL;
	if (error->info != NULL){
		info = wrap_map(error->info);
		count_not_null++;
		tlv_list->count = 2;
		tlv_list->size += info->length;
		tlv_list->list[list_index++] = *info;
	} else {
		tlv_list->list[list_index++] = *wrap_null();
	}

	tlv_list->size += tlv_list->count - count_not_null;

	calculate_bytes(tlv_list, ERROR);

	struct TlvAmqp * tlv = malloc(sizeof(struct TlvAmqp));
	tlv->bytes = tlv_list->bytes;
	tlv->length = tlv_list->length;
	return tlv;
}


struct TlvAmqp * wrap_amqp_target (struct AmqpTarget * target) {

	struct TlvList * tlv_list = malloc(sizeof(struct TlvList));
	tlv_list->list = malloc(7 * sizeof(struct TlvAmqp));
	int count_not_null = 0;
	tlv_list->size = 0;
	int list_index = 0;
	tlv_list->width = 1;

	struct TlvAmqp * address = NULL;
	if (target->address != NULL){
		address = wrap_string((char*)target->address);
		count_not_null++;
		tlv_list->count = 1;
		tlv_list->size += address->length;
		tlv_list->list[list_index++] = *address;
	} else {
		tlv_list->list[list_index++] = *wrap_null();
	}

	struct TlvAmqp * durable = NULL;
	if (target->durable != NULL){
		long * _durable = malloc(sizeof(long));
		*_durable = *(target->durable);
		durable = wrap_long(_durable);
		count_not_null++;
		tlv_list->count = 2;
		tlv_list->size += durable->length;
		tlv_list->list[list_index++] = *durable;
	} else {
		tlv_list->list[list_index++] = *wrap_null();
	}

	struct TlvAmqp * expiry_period = NULL;
	if (target->expiry_period != NULL){
		struct AmqpSymbol * symbol = malloc(sizeof(struct AmqpSymbol));
		symbol->value = string_terminus_expiry_policy(*(target->expiry_period));
		expiry_period = wrap_symbol(symbol);
		count_not_null++;
		tlv_list->count = 3;
		tlv_list->size += expiry_period->length;
		tlv_list->list[list_index++] = *expiry_period;
	} else {
		tlv_list->list[list_index++] = *wrap_null();
	}

	struct TlvAmqp * timeout = NULL;
	if (target->timeout != NULL){
		timeout = wrap_long(target->timeout);
		count_not_null++;
		tlv_list->count = 4;
		tlv_list->size += timeout->length;
		tlv_list->list[list_index++] = *timeout;
	} else {
		tlv_list->list[list_index++] = *wrap_null();
	}

	struct TlvAmqp * dynamic = NULL;
	if (target->dynamic != NULL){
		dynamic = wrap_bool(target->dynamic);
		count_not_null++;
		tlv_list->count = 5;
		tlv_list->size += dynamic->length;
		tlv_list->list[list_index++] = *dynamic;
	} else {
		tlv_list->list[list_index++] = *wrap_null();
	}

	struct TlvAmqp * dynamic_node_properties = NULL;

	if (target->dynamic_node_properties != NULL) {
		if (target->dynamic != NULL) {
			if (*(target->dynamic)) {
				dynamic_node_properties = wrap_map(target->dynamic_node_properties);
				count_not_null++;
				tlv_list->count = 6;
				tlv_list->size += dynamic_node_properties->length;
				tlv_list->list[list_index++] = *dynamic_node_properties;
			} else {
				printf("Target's dynamic-node-properties can't be specified when dynamic flag is false\n");
				return NULL;
			}
		} else {
			printf("Target's dynamic-node-properties can't be specified when dynamic flag is not set\n");
			return NULL;
		}
	} else {
		tlv_list->list[list_index++] = *wrap_null();
	}

	struct TlvAmqp * capabilities = NULL;
	if (target->capabilities != NULL){
		capabilities = wrap_list(target->capabilities);
		count_not_null++;
		tlv_list->count = 7;
		tlv_list->size += capabilities->length;
		tlv_list->list[list_index++] = *capabilities;
	} else {
		tlv_list->list[list_index++] = *wrap_null();
	}

	tlv_list->size += tlv_list->count - count_not_null;

	calculate_bytes(tlv_list, TARGET);

	struct TlvAmqp * tlv = malloc(sizeof(struct TlvAmqp));
	tlv->bytes = tlv_list->bytes;
	tlv->length = tlv_list->length;
	return tlv;
}

struct TlvAmqp * wrap_source (void * p) {

	return NULL;
}


struct TlvAmqp * wrap_array (void * p) {
	printf("NOT IMPLEMENTED YET\n");
	return NULL;
}

struct TlvAmqp * wrap_map (void * p) {
	printf("NOT IMPLEMENTED YET\n");
	return NULL;
}

struct TlvAmqp * wrap_list (void * p) {
	printf("NOT IMPLEMENTED YET\n");
	return NULL;
}

struct TlvAmqp * wrap_outcome (void * p) {
	printf("NOT IMPLEMENTED YET\n");
	return NULL;
}

struct TlvAmqp * wrap_state (struct AmqpState * state) {

	struct TlvAmqp * tlv = malloc(sizeof(struct TlvAmqp));
	tlv->length = 4;
	tlv->bytes = malloc(tlv->length*sizeof(char));
	tlv->bytes[0] = 0x00;
	tlv->bytes[1] = SMALL_ULONG;
	tlv->bytes[2] = state->code;
	tlv->bytes[3] = LIST_0;
	return tlv;

}
