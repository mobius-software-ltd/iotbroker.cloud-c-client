/*
 * header_factory.h
 *
 *  Created on: Nov 1, 2018
 *      Author: konstantinnosach
 */

struct AmqpHeader * get_amqp(char * data, int * i);
struct AmqpHeader * get_sasl(char *data, int * i);
struct SectionEntry * get_section(char * data, int * i);
