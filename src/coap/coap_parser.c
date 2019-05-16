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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../helpers.h"
#include "packets/coap_message.h"

char * coap_encode(struct CoapMessage * message, int * length) {

	int buf_size = 256 + (message->payload != NULL ? strlen(message->payload) : 0);
	char * buf = malloc(sizeof(char) * buf_size);
	int i = 0;
	char first_byte = 0;
	unsigned short string_length = 0;

	string_length = strlen(message->token);
	first_byte += (char) (message->version << 6);
	first_byte += (char) (message->type << 4);
	first_byte |= (char) string_length;
	buf[i++] = first_byte;

	int coap_code = message->code;
	int code_msb = coap_code / 100;
	int code_lsb = coap_code % 100;
	int codeByte = (code_msb << 5) + code_lsb;

	buf[i++] = (codeByte & 0x0FF);
	i += add_short(&buf[i], message->message_id);

	memcpy(&buf[i], message->token, string_length);
	i+=string_length;

	int previous_number = 0;
	for(int k = 0; k < message->options_amount; k++)
	{
		//struct CoapOption option = message->options[i];

		int delta = message->options[k].number - previous_number;
		int nextByte = 0;

		int extended_delta = -1;
		if (delta < 13)
			nextByte += delta << 4;
		else
		{
			extended_delta = delta;
			if (delta < 0xFF)
				nextByte = 13 << 4;
			else
				nextByte = 14 << 4;
		}

		int extended_length = -1;
		if (message->options[k].length < 13)
			nextByte += message->options[k].length;
		else
		{
			extended_length = message->options[k].length;
			if (message->options[k].length < 0xFF)
				nextByte += 13;
			else
				nextByte += 14;
		}

		buf[i++] = nextByte;
		if (extended_delta != -1)
		{
			if (extended_delta < 0xFF)
				buf[i++] = extended_delta - 13;
			else
				i += add_short(&buf[i], extended_delta - 269);
		}

		if (extended_length != -1)
		{
			if (extended_length < 0xFF)
				buf[i++] = extended_length - 13;
			else
				i += add_short(&buf[i], extended_length - 269);
		}

		string_length = message->options[k].length;
		memcpy(&buf[i], message->options[k].value, string_length);
		i+=string_length;

		previous_number =  message->options[k].number;
	}

	buf[i++] = 0xFF;

	string_length = strlen(message->payload);
	if (message->payload != NULL && string_length > 0) {
		memcpy(&buf[i], message->payload, string_length);
		i+=string_length;
	}
    *length = i;
	return buf;

}

struct CoapMessage * coap_decode(char * buf, int length) {

	struct CoapMessage * message = malloc (sizeof (struct CoapMessage));

	int i = 0;
	char first_byte = buf[i++];

	int version = first_byte >> 6;
	if (version != 1) {
		printf("COAP decode : Invalid version : %i\n", version);
		free(message);
	    return NULL;

	} else {
		message->version = version;
	}

	int type_value = (first_byte >> 4) & 2;
	message->type = type_value;

	int token_length = first_byte & 0xf;
	if (token_length < 0 || token_length > 8) {
		printf("COAP decode : Invalid token length: %i\n", token_length);
		free(message);
		return NULL;
	} else {
		message->token_length = token_length;
	}

	char code_byte = buf[i++];
	int code_value = (code_byte >> 5) * 100;
	code_value += code_byte & 0x1F;
	message->code = code_value;

	message->message_id = get_short(buf, i);
	i += 2;

	if (message->message_id < 0 || message->message_id > 65535) {
		printf("COAP decode : Invalid message_id value: %i\n", message->message_id);
		free(message);
		return NULL;
	}


	if (token_length > 0) {
		char * token = malloc (sizeof (token)*(token_length+1));
		memcpy(token, &buf[i], token_length);
		token[token_length] = '\0';
		message->token = token;
		i+=token_length;
	}

	int number = 0;
	int readable_bytes =  length - i;
	int option_index = 0;

	for (int j = i; j < length;)
	{
		char next_byte = buf[j++];
	    if ((next_byte & 0XFF) == 0xFF)
	    	break;

	    if(option_index == 0)
	    	message->options = malloc (sizeof (struct CoapOption));
	    else
	    	message->options = realloc(message->options, ((option_index+1) * sizeof(struct CoapOption)));

	    unsigned int delta = (next_byte >> 4) & 15;

	    if (delta == 13)
	    	delta = buf[j++] + 13;
	    else if (delta == 14) {
	    	delta = get_short(buf, j) + 269;
	    	j+=2;
	    } else if (delta < 0 || delta > 14) {
	    	printf("COAP decode : Invalid option delta value: %i\n", delta);
	    	free(message);
	    	return NULL;
	    }

	    number += delta;
	    if (number < 0) {
	    	printf("COAP decode : invalid negative option number: %i , delta: %i\n", number, delta);
	    	free(message);
	    	return NULL;
	    }
	    int option_length = next_byte & 15;
	    if (option_length == 13)
	    	option_length = buf[j++] + 13;
	    else if (option_length == 14) {
	    	option_length = get_short(buf, j) + 269;
	    } else if (option_length < 0 || option_length > 14) {
	    	printf("COAP decode : invalid option length\n");
	    	free(message);
	    	return NULL;
	    }

	    char * option_value = NULL;
	    if (option_length > 0) {
	    	option_value = malloc (option_length * sizeof (char));
	    	memcpy(option_value, &buf[j], option_length);
	    	j+=option_length;
	    }

	    struct CoapOption co = {.number = number, .length = option_length, .value = option_value};
	    message->options[option_index] = co;
	    option_index++;
	    i=j;

	}
	message->options_amount = option_index;
	readable_bytes = length - i;

	if (readable_bytes > 0 && buf[++i] !=0x00)
	{
		char * payload_value = malloc (sizeof (payload_value)*(readable_bytes+1));
		memcpy(payload_value, &buf[i], readable_bytes);
		payload_value[readable_bytes] = '\0';
		message->payload = payload_value;
	}

	return message;
}
