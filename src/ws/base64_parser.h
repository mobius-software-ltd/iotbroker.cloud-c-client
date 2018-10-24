/*
 * base64_parser.h
 *
 *  Created on: Oct 23, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_WS_BASE64_PARSER_H_
#define SRC_WS_BASE64_PARSER_H_

#include <stdlib.h>

char *base64_encode(const unsigned char *data, size_t input_length, size_t *output_length);
unsigned char *base64_decode(const char *data, size_t input_length, size_t *output_length);

#endif /* SRC_WS_BASE64_PARSER_H_ */
