/*
 * helpers.h
 *
 *  Created on: Jun 25, 2018
 *      Author: konstantinnosach
 */
#ifndef HELPERS_H_
#define HELPERS_H_

char *utf8_check(char *s);
int add_short(char * buf, unsigned short s);
int get_int(char* array, int offset);
double get_double (char* array, int offset);
float get_float (char* array, int offset);
long get_long(char* array, int offset);
unsigned short get_short(char* array, int offset);
struct LengthDetails decode_remaining_length(char buf[]);
int add_packet_length(int length, char * remaining_length);
int remaining_length(int length);
void reverse (unsigned char * a, int len);

struct LengthDetails {
	int length;
	int bytes_used;
};

#endif /* HELPERS_H_ */
