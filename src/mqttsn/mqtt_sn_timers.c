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
#include "../mqttsn/mqtt_sn_client.h"
#include "../mqttsn/packets/sn_subscribe.h"
#include "../mqttsn/packets/sn_unsubscribe.h"
#include "../mqttsn/packets/sn_message.h"
#include "../mqttsn/packets/sn_publish.h"

//static int got_conack = 0;
static pthread_t pinger;
static pthread_t messager;
static pthread_t connecter;
static unsigned int delay_in_seconds = 10;
//int stop_ping = 0;

static map_void_t messages_map;
//<topic_name,topic_id>
static map_int_t topic_name_map;
//<topic_id,topic_name>
static map_str_t reverse_topic_name_map;


static void *sn_ping_task(void *arg)
{
    for(;;)
    {
        sleep(delay_in_seconds);
        sn_send_ping();
    }
    return 0;
}

static void *connect_task(void *arg)
{
    for(int i = 0; i < 5; i++)
    {
        sleep(5);
        const char *key;
        map_iter_t iter = map_iter(&messages_map);

        while ((key = map_next(&messages_map, &iter))) {
        	struct SnMessage * sn_message = malloc(sizeof(struct SnMessage *));
            sn_message = * map_get(&messages_map, key);
            sn_encode_and_fire(sn_message);
        }
    }
    return 0;
}

static void *message_resend_task(void *arg)
{
    for(;;)
    {
        sleep(5);
        const char *key;
        map_iter_t iter = map_iter(&m);

        while ((key = map_next(&messages_map, &iter))) {
        	struct SnMessage * sn_message = malloc(sizeof(struct SnMessage *));
            sn_message = * map_get(&messages_map, key);
            enum SnMessageType type = sn_message->message_type;
            if(type == SN_PUBLISH) {
        	   struct SnPublish * p = (struct SnPublish*) sn_message->packet;
        	   if(p->topic.id==0)
        		   continue;
           	}

           sn_encode_and_fire(sn_message);
        }
    }
    return 0;
}

void sn_start_connect_timer() {

	map_init(&messages_map);

	long t = 10;
	int rc = pthread_create(&connecter, NULL, connect_task, (void *)t);
	if (rc) {
		printf("ERROR; return code from pthread_create() connect_timer is %d\n", rc);
		send_sn_disconnect();
		exit(-1);
	}

}

void start_mqtt_sn_ping_timer(unsigned int delay) {

	map_init(&messages_map);
	map_init(&topic_name_map);
	map_init(&reverse_topic_name_map);
	delay_in_seconds = delay;

	long t = 11;
	int rc = pthread_create(&pinger, NULL, sn_ping_task, (void *)t);
	if (rc) {
		printf("ERROR; return code from pthread_create() sn_start_ping_timer is %d\n", rc);
		send_sn_disconnect();
		exit(-1);
	}
}

void sn_start_message_timer() {

	long t = 12;
	int rc = pthread_create(&messager, NULL, message_resend_task, (void *)t);
	if (rc) {
		printf("ERROR; return code from pthread_create() start_message_timer is %d\n", rc);
		send_sn_disconnect();
		exit(-1);
	}
}

void sn_stop_connect_timer() {
	pthread_cancel(connecter);
}

void sn_stop_ping_timer() {
	map_deinit(&messages_map);
	map_deinit(&topic_name_map);
	map_deinit(&reverse_topic_name_map);
	pthread_cancel(pinger);
}

void sn_stop_message_timer() {
	pthread_cancel(messager);
}

void sn_add_message_in_map(struct SnMessage * sn_message) {

	unsigned short packet_id = 0;
	enum SnMessageType type = sn_message->message_type;
	switch(type) {

		case SN_PUBLISH: {
			struct SnPublish * p = (struct SnPublish*) sn_message->packet;
			packet_id = p->message_id;
			break;
		}
		case SN_CONNECT: {
			packet_id = 0;
			break;
		}
		case SN_SUBSCRIBE:  {
			struct SnSubscribe * s = (struct SnSubscribe*) sn_message->packet;
			packet_id = s->message_id;
			break;
		}
		case SN_UNSUBSCRIBE:  {
			struct SnUnsubscribe * us = (struct SnUnsubscribe*) sn_message->packet;
			packet_id = us->message_id;
			break;
		}
		default:
			break;
	}

	char str[12];
	sprintf(str, "%d", packet_id);
	map_set(&messages_map, str, sn_message);
}

void sn_add_topic_id_in_map(const char * topic_name, unsigned short topic_id) {
	map_set(&topic_name_map, (char *)topic_name, topic_id);
}

void sn_add_topic_name_in_map(unsigned short topic_id, const char * topic_name) {
	char str[12];
	sprintf(str, "%d", topic_id);
	map_set(&reverse_topic_name_map, str, (char *)topic_name);
}

struct SnMessage * sn_get_message_from_map(unsigned short packet_id){

	char str[12];
	sprintf(str, "%d", packet_id);
	const char *key;
	map_iter_t iter = map_iter(&messages_map);

	struct SnMessage * message = malloc(sizeof(struct SnMessage *));
	while ((key = map_next(&messages_map, &iter))) {
		 message = *map_get(&messages_map, key);
		 if(strcmp(key,str)==0) {
			 return message;
		 }
	}

	return NULL;
}

unsigned short sn_get_topic_id_from_map(const char * topic_name) {

	const char *key;
	map_iter_t iter = map_iter(&messages_map);

	unsigned short topic_id = 0;
	while ((key = map_next(&topic_name_map, &iter))) {
		topic_id = *map_get(&topic_name_map, key);
		 if(strcmp(key,topic_name)==0) {
			 return topic_id;
		 }
	}
	return 0;
}

char * sn_get_topic_name_from_map(unsigned short topic_id) {

	char str[12];
		sprintf(str, "%d", topic_id);
		const char *key;
		map_iter_t iter = map_iter(&messages_map);

		char * topic_name = malloc(sizeof(topic_name));
		while ((key = map_next(&reverse_topic_name_map, &iter))) {
			topic_name = *map_get(&reverse_topic_name_map, key);
			 if(strcmp(key,str)==0) {
				 return topic_name;
			 }
		}

		return NULL;
}

void sn_remove_message_from_map (unsigned short packet_id) {
	char str[12];
	sprintf(str, "%d", packet_id);
	const char *key = str;
	map_remove(&messages_map, key);
}

void sn_stop_all_timers() {
	sn_stop_ping_timer();
	sn_stop_message_timer();
}

