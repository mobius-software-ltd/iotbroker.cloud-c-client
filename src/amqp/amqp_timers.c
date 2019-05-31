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
#include "../net/lws_net_client.h"
#include "../amqp/amqp_client.h"

static pthread_t pinger;
static pthread_t messager;
static pthread_t connecter;
static unsigned int delay_in_seconds = 10;

//map for storing transfer messages for send after attach
static GHashTable * m_handler = NULL;
//map for storing transfer messages for resend if needed
static GHashTable * m_delivery_id = NULL;
//<topic_name,topic_id>
static GHashTable * topic_name_map = NULL;
//<topic_name,topic_id>
static GHashTable * topic_name_map_outgoing = NULL;
//<topic_id,topic_name>
static GHashTable * reverse_topic_name_map = NULL;

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
    lws_close_tcp_connection();
    return 0;
}

static void *amqp_message_resend_task(void *arg)
{
    for(;;)
    {
        sleep(5);

        GHashTableIter iter;
		gpointer key, value;

		g_hash_table_iter_init (&iter, m_delivery_id);
		while (g_hash_table_iter_next (&iter, &key, &value))
		{
			struct AmqpHeader * message = (struct AmqpHeader *)value;
			amqp_encode_and_fire(message);
		}
    }
    return 0;
}


void start_amqp_ping_timer(unsigned int delay) {

	m_handler = g_hash_table_new (g_int_hash, g_int_equal);
	m_delivery_id = g_hash_table_new (g_int_hash, g_int_equal);
	topic_name_map = g_hash_table_new (g_str_hash, g_str_equal);
	topic_name_map_outgoing = g_hash_table_new (g_str_hash, g_str_equal);
	reverse_topic_name_map = g_hash_table_new (g_int_hash, g_int_equal);

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
	if(m_handler!=NULL) {
		g_hash_table_destroy(m_handler);
		g_hash_table_destroy(m_delivery_id);
		g_hash_table_destroy(topic_name_map);
		g_hash_table_destroy(topic_name_map_outgoing);
		g_hash_table_destroy(reverse_topic_name_map);
	}
	pthread_cancel(pinger);
}

void amqp_stop_message_timer() {
	pthread_cancel(messager);
}

void amqp_add_message_in_map_by_handler(unsigned short handler, struct AmqpHeader * message) {

	int* handler_int = (int*)malloc(sizeof(int));
	handler_int[0] = handler;
	g_hash_table_insert (m_handler, handler_int, message);

}

void amqp_add_message_in_map_by_delivery_id(unsigned short delivery_id, struct AmqpHeader * message) {

	int* delivery_id_int = (int*)malloc(sizeof(int));
	delivery_id_int[0] = delivery_id;
	g_hash_table_insert (m_delivery_id, delivery_id_int, message);

}


struct AmqpHeader * amqp_get_message_from_map_by_handler(unsigned short handler){

	int* handler_int = (int*)malloc(sizeof(int));
	handler_int[0] = handler;

	gpointer value = g_hash_table_lookup(m_handler, handler_int);
	if(value == NULL)
		return NULL;
	else
		return (struct AmqpHeader *)value;

}

struct AmqpHeader * amqp_get_message_from_map_by_delivery_id(unsigned short delivery_id){

	int* delivery_id_int = (int*)malloc(sizeof(int));
	delivery_id_int[0] = delivery_id;
	gpointer value = g_hash_table_lookup(m_delivery_id, delivery_id_int);
	if(value == NULL)
		return NULL;
	else
		return (struct AmqpHeader *)value;

}


void amqp_remove_message_from_map_handler (unsigned short handler) {

	int* handler_int = (int*)malloc(sizeof(int));
	handler_int[0]=handler;
	g_hash_table_remove(m_handler, handler_int);

}

void amqp_remove_message_from_map_delivery_id (unsigned short delivery_id) {

	int* delivery_id_int = (int*)malloc(sizeof(int));
	delivery_id_int[0]=delivery_id;
	g_hash_table_remove(m_delivery_id, delivery_id_int);

}


void amqp_remove_topic_name_from_map (char * topic_name) {

	g_hash_table_remove(topic_name_map, topic_name);

}

void amqp_remove_topic_name_from_map_outgoing (char * topic_name) {

	g_hash_table_remove(topic_name_map_outgoing, topic_name);

}


void amqp_remove_handle_from_map (unsigned short packet_id) {

	int* packet_id_int = (int*)malloc(sizeof(int));
	packet_id_int[0]=packet_id;
	g_hash_table_remove(reverse_topic_name_map, packet_id_int);

}

void amqp_stop_all_timers(){
	amqp_stop_ping_timer();
	amqp_stop_message_timer();
}

void amqp_add_handler_in_map(const char * topic_name, unsigned short handler) {

	int* handler_int = (int*)malloc(sizeof(int));
	handler_int[0] = handler;
	g_hash_table_insert (topic_name_map, (char*)topic_name, handler_int);

}

void amqp_add_handler_in_map_outgoing(const char * topic_name, unsigned short handler) {

	int* handler_int = (int*)malloc(sizeof(int));
	handler_int[0] = handler;
	g_hash_table_insert (topic_name_map_outgoing, (char*)topic_name, handler_int);

}

void amqp_add_topic_name_in_map(unsigned short handler, const char * topic_name) {


	int* handler_int = (int*)malloc(sizeof(int));
	handler_int[0] = handler;
	g_hash_table_insert (reverse_topic_name_map, handler_int, (char*)topic_name);

}

unsigned short amqp_get_handler_from_map(const char * topic_name) {

	gpointer value = g_hash_table_lookup(topic_name_map, topic_name);
	if(value == NULL)
	{
		return 0;
	}
	else
	{
		return *(int*)value;
	}

}

unsigned short amqp_get_handler_from_map_outgoing(const char * topic_name) {

	gpointer value = g_hash_table_lookup(topic_name_map_outgoing, topic_name);
	if(value == NULL)
	{
		return 0;
	}
	else
	{
		return *(int*)value;
	}

}

char * amqp_get_topic_name_from_map(unsigned short handler) {

	int* handler_int = (int*)malloc(sizeof(int));
	handler_int[0] = handler;
	gpointer value = g_hash_table_lookup(reverse_topic_name_map, handler_int);
	if(value != NULL)
		return (char *)value;
	else
		return NULL;

}
