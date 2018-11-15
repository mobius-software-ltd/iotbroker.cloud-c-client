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
