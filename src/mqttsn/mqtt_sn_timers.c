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
#include <glib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../net/tcp_client.h"
#include "../mqttsn/mqtt_sn_client.h"
#include "../mqttsn/packets/sn_subscribe.h"
#include "../mqttsn/packets/sn_unsubscribe.h"
#include "../mqttsn/packets/sn_message.h"
#include "../mqttsn/packets/sn_publish.h"


static pthread_t pinger;
static pthread_t messager;
static pthread_t connecter;
static unsigned int delay_in_seconds = 10;


static GHashTable *messages_map = NULL;
//<topic_name,topic_id>
static GHashTable *topic_name_map = NULL;
//<topic_id,topic_name>
static GHashTable *reverse_topic_name_map = NULL;

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
        int key = 0;
        gpointer value = g_hash_table_lookup(messages_map, &key);
        if(value != NULL)
			sn_encode_and_fire((struct SnMessage *)value);
    }
    fin_mqtt_sn_client();
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
        	struct SnMessage * sn_message = (struct SnMessage *)value;
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

	messages_map = g_hash_table_new (g_int_hash, g_int_equal);

	long t = 10;
	int rc = pthread_create(&connecter, NULL, connect_task, (void *)t);
	if (rc) {
		printf("ERROR; return code from pthread_create() connect_timer is %d\n", rc);
		send_sn_disconnect();
		exit(-1);
	}

}

void start_mqtt_sn_ping_timer(unsigned int delay) {


	topic_name_map = g_hash_table_new (g_str_hash, g_str_equal);
	reverse_topic_name_map = g_hash_table_new (g_int_hash, g_int_equal);
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
	if(messages_map!=NULL)
		g_hash_table_destroy (messages_map);
	if(topic_name_map!=NULL)
		g_hash_table_destroy (topic_name_map);
	if(reverse_topic_name_map!=NULL)
		g_hash_table_destroy (reverse_topic_name_map);
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

	int* packet_id_int = (int*)malloc(sizeof(int));
	packet_id_int[0]=packet_id;
	g_hash_table_insert (messages_map, packet_id_int, sn_message);

}

void sn_add_topic_id_in_map(const char * topic_name, unsigned short topic_id) {

	int* topic_id_int = (int*)malloc(sizeof(int));
	topic_id_int[0]=topic_id;
	g_hash_table_insert (topic_name_map, (char*)topic_name, topic_id_int);

}

void sn_add_topic_name_in_map(unsigned short topic_id, const char * topic_name) {

	int* topic_id_int = (int*)malloc(sizeof(int));
	topic_id_int[0]=topic_id;
	g_hash_table_insert (reverse_topic_name_map, topic_id_int, (char*)topic_name);

}

struct SnMessage * sn_get_message_from_map(unsigned short packet_id) {

	int* packet_id_int = (int*)malloc(sizeof(int));
	packet_id_int[0] = packet_id;

	gpointer value = g_hash_table_lookup(messages_map, packet_id_int);
	if(value == NULL)
		return NULL;
	else
		return (struct SnMessage *)value;

}

unsigned short sn_get_topic_id_from_map(const char * topic_name) {

	char* topic_name_str = (char*)malloc((sizeof(char)*strlen(topic_name)+1));
	strcpy(topic_name_str,topic_name);

	gpointer value = g_hash_table_lookup(topic_name_map, topic_name_str);
	free(topic_name_str);
	if(value == NULL)
	{
		return 0;
	}
	else
	{
		return *(int*)value;
	}
}

char * sn_get_topic_name_from_map(unsigned short topic_id) {

	int* topic_id_int = (int*)malloc(sizeof(int));
	topic_id_int[0] = topic_id;
	gpointer value = g_hash_table_lookup(reverse_topic_name_map, topic_id_int);
	if(value != NULL)
		return (char *)value;
	else
		return NULL;
}

void sn_remove_message_from_map (unsigned short packet_id) {

	int* packet_id_int = (int*)malloc(sizeof(int));
	packet_id_int[0]=packet_id;
	g_hash_table_remove(messages_map, packet_id_int);
}

void sn_stop_all_timers() {
	sn_stop_ping_timer();
	sn_stop_message_timer();
}

