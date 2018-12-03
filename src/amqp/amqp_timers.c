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
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "../net/tcp_client.h"
#include "../map/map.h"
#include "../amqp/amqp_client.h"

static pthread_t pinger;
static pthread_t messager;
static pthread_t connecter;
static unsigned int delay_in_seconds = 10;

//map for storing transfer messages for send after attach
static map_void_t m_handler;
//map for storing transfer messages for resend if needed
static map_void_t m_delivery_id;
//<topic_name,topic_id>
static map_int_t topic_name_map;
//<topic_name,topic_id>
static map_int_t topic_name_map_outgoing;
//<topic_id,topic_name>
static map_str_t reverse_topic_name_map;

static void *amqp_ping_task(void *arg)
{
    for(;;)
    {
        sleep(delay_in_seconds);
        send_amqp_ping();
    }
    return 0;
}

static void *amqp_connect_task(void *arg)
{
    //wait 5 sec until successful connection
    sleep(5);
    stop_net_service();
    return 0;
}

static void *amqp_message_resend_task(void *arg)
{
    for(;;)
    {
        sleep(5);
        const char *key;
        map_iter_t iter = map_iter(&m_delivery_id);

        while ((key = map_next(&m_delivery_id, &iter))) {
           struct AmqpHeader * message = malloc(sizeof(struct AmqpHeader *));
           message = * map_get(&m_delivery_id, key);
           amqp_encode_and_fire(message);
        }
    }
    return 0;
}


void start_amqp_ping_timer(unsigned int delay) {

	map_init(&m_handler);
	map_init(&m_delivery_id);
	map_init(&topic_name_map);
	map_init(&topic_name_map_outgoing);
	map_init(&reverse_topic_name_map);

	delay_in_seconds = delay;

	long t = 11;
	int rc = pthread_create(&pinger, NULL, amqp_ping_task, (void *)t);
	if (rc) {
		printf("ERROR; return code from pthread_create() start_ping_timer is %d\n", rc);
		send_amqp_end();
		exit(-1);
	}
}

void amqp_start_message_timer() {

	long t = 12;
	int rc = pthread_create(&messager, NULL, amqp_message_resend_task, (void *)t);
	if (rc) {
		printf("ERROR; return code from pthread_create() start_message_timer is %d\n", rc);
		send_amqp_end();
		exit(-1);
	}
}

void amqp_start_connect_timer() {

	long t = 13;
	int rc = pthread_create(&connecter, NULL, amqp_connect_task, (void *)t);
	if (rc) {
		printf("ERROR; return code from pthread_create() amqp_start_connect_timer is %d\n", rc);
		send_amqp_end();
		exit(-1);
	}
}

void amqp_stop_connect_timer() {
	pthread_cancel(connecter);
}



void amqp_stop_ping_timer() {
	map_deinit(&m_handler);
	map_deinit(&m_delivery_id);
	map_deinit(&topic_name_map);
	map_deinit(&topic_name_map_outgoing);
	map_deinit(&reverse_topic_name_map);
	pthread_cancel(pinger);
}

void amqp_stop_message_timer() {
	pthread_cancel(messager);
}

void amqp_add_message_in_map_by_handler(unsigned short handler, struct AmqpHeader * message) {

	char str[12];
	sprintf(str, "%d", handler);
	map_set(&m_handler, str, message);

}

void amqp_add_message_in_map_by_delivery_id(unsigned short delivery_id, struct AmqpHeader * message) {

	char str[12];
	sprintf(str, "%d", delivery_id);
	map_set(&m_delivery_id, str, message);

}


struct AmqpHeader * amqp_get_message_from_map_by_handler(unsigned short handler){

	char str[12];
	sprintf(str, "%d", handler);
	const char *key;
	map_iter_t iter = map_iter(&m_handler);

	struct AmqpHeader * message = malloc(sizeof(struct AmqpHeader *));
	while ((key = map_next(&m_handler, &iter))) {
		 message = *map_get(&m_handler, key);
		 if(strcmp(key,str)==0) {
			 return message;
		 }
	}
	return NULL;
}

struct AmqpHeader * amqp_get_message_from_map_by_delivery_id(unsigned short delivery_id){

	char str[12];
	sprintf(str, "%d", delivery_id);
	const char *key;
	map_iter_t iter = map_iter(&m_delivery_id);

	struct AmqpHeader * message = malloc(sizeof(struct AmqpHeader *));
	while ((key = map_next(&m_delivery_id, &iter))) {
		 message = *map_get(&m_delivery_id, key);
		 if(strcmp(key,str)==0) {
			 return message;
		 }
	}
	return NULL;
}


void amqp_remove_message_from_map_handler (unsigned short handler) {
	char str[12];
	sprintf(str, "%d", handler);
	const char *key = str;
	map_remove(&m_handler, key);
}

void amqp_remove_message_from_map_delivery_id (unsigned short delivery_id) {
	char str[12];
	sprintf(str, "%d", delivery_id);
	const char *key = str;
	map_remove(&m_delivery_id, key);
}


void amqp_remove_topic_name_from_map (char * topic_name) {
	map_remove(&topic_name_map, topic_name);
}

void amqp_remove_topic_name_from_map_outgoing (char * topic_name) {
	map_remove(&topic_name_map_outgoing, topic_name);
}


void amqp_remove_handle_from_map (unsigned short packet_id) {
	char str[12];
	sprintf(str, "%d", packet_id);
	const char *key = str;
	map_remove(&reverse_topic_name_map, key);
}

void amqp_stop_all_timers(){
	amqp_stop_ping_timer();
	amqp_stop_message_timer();
}

void amqp_add_handler_in_map(const char * topic_name, unsigned short handler) {
	map_set(&topic_name_map, (char *)topic_name, handler);
}

void amqp_add_handler_in_map_outgoing(const char * topic_name, unsigned short handler) {
	map_set(&topic_name_map_outgoing, (char *)topic_name, handler);
}

void amqp_add_topic_name_in_map(unsigned short handler, const char * topic_name) {
	char str[12];
	sprintf(str, "%d", handler);
	map_set(&reverse_topic_name_map, str, (char *)topic_name);
}

unsigned short amqp_get_handler_from_map(const char * topic_name) {

	const char *key;
	map_iter_t iter = map_iter(&topic_name_map);

	unsigned short handler = 0;
	while ((key = map_next(&topic_name_map, &iter))) {
		handler = *map_get(&topic_name_map, key);
		 if(strcmp(key,topic_name)==0) {
			 return handler;
		 }
	}
	return 0;
}

unsigned short amqp_get_handler_from_map_outgoing(const char * topic_name) {

	const char *key;
	map_iter_t iter = map_iter(&topic_name_map_outgoing);

	unsigned short handler = 0;
	while ((key = map_next(&topic_name_map_outgoing, &iter))) {
		handler = *map_get(&topic_name_map_outgoing, key);
		 if(strcmp(key,topic_name)==0) {
			 return handler;
		 }
	}
	return 0;
}

char * amqp_get_topic_name_from_map(unsigned short handler) {

	char str[12];
	sprintf(str, "%d", handler);
	const char *key;
	map_iter_t iter = map_iter(&reverse_topic_name_map);

	char * topic_name = malloc(sizeof(topic_name));
	while ((key = map_next(&reverse_topic_name_map, &iter))) {
		topic_name = *map_get(&reverse_topic_name_map, key);
		 if(strcmp(key,str)==0) {
			 return topic_name;
		 }
	}

	return NULL;
}
