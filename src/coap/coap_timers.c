/*
 * coap_timers.c
 *
 *  Created on: Oct 16, 2018
 *      Author: konstantinnosach
 */

#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "../net/tcp_client.h"
#include "../map/map.h"
#include "../coap/coap_client.h"

static pthread_t pinger;
static pthread_t messager;
static unsigned int delay_in_seconds = 10;

static map_void_t m;

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
        printf("resend\n");
        const char *key;
        map_iter_t iter = map_iter(&m);

        while ((key = map_next(&m, &iter))) {
           struct CoapMessage * message = malloc(sizeof(struct CoapMessage *));
           message = * map_get(&m, key);
           coap_encode_and_fire(message);
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

	long t = 12;
	int rc = pthread_create(&messager, NULL, coap_message_resend_task, (void *)t);
	if (rc) {
		printf("ERROR; return code from pthread_create() start_message_timer is %d\n", rc);
		send_coap_disconnect();
		exit(-1);
	}
}

void coap_stop_ping_timer() {
	map_deinit(&m);
	pthread_cancel(pinger);
}

void coap_stop_message_timer() {
	pthread_cancel(messager);
}

void coap_add_message_in_map(struct CoapMessage * message) {

	char str[12];
	sprintf(str, "%d", message->message_id);
	map_set(&m, str, message);

}

struct CoapMessage * coap_get_message_from_map(unsigned short packet_id){

	char str[12];
	sprintf(str, "%d", packet_id);
	const char *key;
	map_iter_t iter = map_iter(&m);

	struct CoapMessage * message = malloc(sizeof(struct CoapMessage *));
	while ((key = map_next(&m, &iter))) {
		 message = *map_get(&m, key);
		 if(strcmp(key,str)==0) {
			 return message;
		 }
	}

	return NULL;
}

void coap_remove_message_from_map (unsigned short packet_id) {
	char str[12];
	sprintf(str, "%d", packet_id);
	const char *key = str;
	map_remove(&m, key);
}

void coap_stop_all_timers(){
	coap_stop_ping_timer();
	coap_stop_message_timer();
}
