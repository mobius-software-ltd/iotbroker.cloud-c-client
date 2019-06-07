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
#include <glib.h>
#include "../net/tcp_client.h"
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

static GHashTable *messages_map = NULL;

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
        int key = 0;
		gpointer value = g_hash_table_lookup(messages_map, &key);
		if(value != NULL)
			encode_and_fire((struct Message *)value);
    }
    return 0;
}

static void *message_resend_task(void *arg)
{
    for(;;)
    {
        sleep(5);
        GHashTableIter iter;
		gpointer key, value;

		g_hash_table_iter_init (&iter, messages_map);
		while (g_hash_table_iter_next (&iter, &key, &value))
		{
			int diff = time(NULL)-((struct Message *)value)->time_stamp;
			if( (diff) > 10)
				encode_and_fire((struct Message *)value);
		}
    }
    return 0;
}

void start_connect_timer() {

	messages_map = g_hash_table_new (g_int_hash, g_int_equal);

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
	if(messages_map != NULL)
		g_hash_table_destroy (messages_map);
	pthread_cancel(pinger);
}

void stop_message_timer() {
	pthread_cancel(messager);
}

void add_message_in_map(struct Message * message) {

	message->time_stamp = time(NULL);
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


	int* packet_id_int = (int*)malloc(sizeof(int));
	packet_id_int[0]=packet_id;
	g_hash_table_insert (messages_map, packet_id_int, message);
}

struct Message * get_message_from_map(unsigned short packet_id){

	int* packet_id_int = (int*)malloc(sizeof(int));
	packet_id_int[0] = packet_id;
	gpointer value = g_hash_table_lookup(messages_map, packet_id_int);
	if(value == NULL)
		return NULL;
	else
		return (struct Message *)value;
}

void remove_message_from_map (unsigned short packet_id) {
	int* packet_id_int = (int*)malloc(sizeof(int));
	packet_id_int[0]=packet_id;
	g_hash_table_remove(messages_map, packet_id_int);
}

void stop_all_timers(){
	stop_ping_timer();
	stop_message_timer();
}

