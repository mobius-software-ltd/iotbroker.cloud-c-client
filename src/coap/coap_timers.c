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
#include <glib.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "../net/tcp_client.h"
#include "../coap/coap_client.h"

static pthread_t pinger;
static pthread_t messager;
static unsigned int delay_in_seconds = 10;

static GHashTable *messages_map = NULL;

static void *coap_ping_task(void *arg)
{
    for(;;)
    {
        sleep(delay_in_seconds);
        coap_send_ping();
    }
    return 0;
}

static void *coap_message_resend_task(void *arg)
{
    for(;;)
    {
         sleep(5);
         GHashTableIter iter;
		 gpointer key, value;

		 g_hash_table_iter_init (&iter, messages_map);
		 while (g_hash_table_iter_next (&iter, &key, &value))
		 {
			struct CoapMessage * coap_message = (struct CoapMessage *)value;
			coap_encode_and_fire(coap_message);
		 }
    }
    return 0;
}


void start_coap_ping_timer(unsigned int delay) {

	delay_in_seconds = delay;

	long t = 11;
	int rc = pthread_create(&pinger, NULL, coap_ping_task, (void *)t);
	if (rc) {
		printf("ERROR; return code from pthread_create() start_ping_timer is %d\n", rc);
		send_coap_disconnect();
		exit(-1);
	}
}

void coap_start_message_timer() {

	messages_map = g_hash_table_new (g_int_hash, g_int_equal);
	long t = 12;
	int rc = pthread_create(&messager, NULL, coap_message_resend_task, (void *)t);
	if (rc) {
		printf("ERROR; return code from pthread_create() start_message_timer is %d\n", rc);
		send_coap_disconnect();
		exit(-1);
	}
}

void coap_stop_ping_timer() {
	g_hash_table_destroy (messages_map);
	pthread_cancel(pinger);
}

void coap_stop_message_timer() {
	pthread_cancel(messager);
}

void coap_add_message_in_map(struct CoapMessage * message) {

	int* packet_id_int = (int*)malloc(sizeof(int));
	packet_id_int[0]=message->message_id;
	g_hash_table_insert (messages_map, packet_id_int, message);

}

struct CoapMessage * coap_get_message_from_map(unsigned short packet_id){

	int* packet_id_int = (int*)malloc(sizeof(int));
	packet_id_int[0] = packet_id;
	gpointer value = g_hash_table_lookup(messages_map, packet_id_int);
	if(value == NULL)
		return NULL;
	else
		return (struct CoapMessage *)value;
}

void coap_remove_message_from_map (unsigned short packet_id) {

	int* packet_id_int = (int*)malloc(sizeof(int));
	packet_id_int[0]=packet_id;
	g_hash_table_remove(messages_map, packet_id_int);

}

void coap_stop_all_timers(){
	coap_stop_ping_timer();
	coap_stop_message_timer();
}
