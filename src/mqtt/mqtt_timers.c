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
#include "../mqtt/mqtt_client.h"
#include "../mqtt/packets/subscribe.h"
#include "../mqtt/packets/unsubscribe.h"
#include "../mqtt/packets/message.h"
#include "../mqtt/packets/publish.h"

int got_conack = 0;
static pthread_t pinger;
static pthread_t messager;
static pthread_t connecter;
static unsigned int delay_in_seconds = 10;
int stop_ping = 0;

static map_void_t m;

static void *ping_task(void *arg)
{
    for(;;)
    {
        sleep(delay_in_seconds);
        send_ping();
    }
    return 0;
}

static void *connect_task(void *arg)
{
    for(int i = 0; i < 5; i++)
    {
        sleep(3);
        const char *key;
        map_iter_t iter = map_iter(&m);

        while ((key = map_next(&m, &iter))) {
           struct Message * message = malloc(sizeof(struct Message *));
           message = * map_get(&m, key);
           encode_and_fire(message);
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

        while ((key = map_next(&m, &iter))) {
           struct Message * message = malloc(sizeof(struct Message *));
           message = * map_get(&m, key);
           encode_and_fire(message);
        }
    }
    return 0;
}

void start_connect_timer() {

	map_init(&m);

	long t = 10;
	int rc = pthread_create(&connecter, NULL, connect_task, (void *)t);
	if (rc) {
		printf("ERROR; return code from pthread_create() connect_timer is %d\n", rc);
		send_disconnect();
		exit(-1);
	}

}

void start_mqtt_ping_timer(unsigned int delay) {

	delay_in_seconds = delay;

	long t = 11;
	int rc = pthread_create(&pinger, NULL, ping_task, (void *)t);
	if (rc) {
		printf("ERROR; return code from pthread_create() start_ping_timer is %d\n", rc);
		send_disconnect();
		exit(-1);
	}
}

void start_message_timer() {

	long t = 12;
	int rc = pthread_create(&messager, NULL, message_resend_task, (void *)t);
	if (rc) {
		printf("ERROR; return code from pthread_create() start_message_timer is %d\n", rc);
		send_disconnect();
		exit(-1);
	}
}

void stop_connect_timer() {
	pthread_cancel(connecter);
}

void stop_ping_timer() {
	map_deinit(&m);
	pthread_cancel(pinger);
}

void stop_message_timer() {
	pthread_cancel(messager);
}

void add_message_in_map(struct Message * message) {

	unsigned short packet_id = 0;
	enum MessageType type = message->message_type;
	switch(type) {

		case PUBLISH: {
			struct Publish * p = (struct Publish*) message->packet;
			packet_id = p->packet_id;
			break;
		}
		case CONNECT: {
			packet_id = 0;
			break;
		}
		case SUBSCRIBE:  {
			struct Subscribe * s = (struct Subscribe*) message->packet;
			packet_id = s->packet_id;
			break;
		}
		case UNSUBSCRIBE:  {
			struct Unsubscribe * us = (struct Unsubscribe*) message->packet;
			packet_id = us->packet_id;
			break;
		}
		case DISCONNECT:
		case CONNACK:
		case PUBACK:
		case PUBREC:
		case PUBREL:
		case PUBCOMP:
		case SUBACK:
		case UNSUBACK:
		case PINGREQ:
		case PINGRESP:
			break;
	}

	char str[12];
	sprintf(str, "%d", packet_id);
	map_set(&m, str, message);
}

struct Message * get_message_from_map(unsigned short packet_id){

	char str[12];
	sprintf(str, "%d", packet_id);
	const char *key;
	map_iter_t iter = map_iter(&m);

	struct Message * message = malloc(sizeof(struct Message *));
	while ((key = map_next(&m, &iter))) {
		 message = *map_get(&m, key);
		 if(strcmp(key,str)==0) {
			 return message;
		 }
	}


	return NULL;
}

void remove_message_from_map (unsigned short packet_id) {
	char str[12];
	sprintf(str, "%d", packet_id);
	const char *key = str;
	map_remove(&m, key);
}

void stop_all_timers(){
	stop_ping_timer();
	stop_message_timer();
}

