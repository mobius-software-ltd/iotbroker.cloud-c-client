/*
 * tcp_client.c
 *
 *  Created on: Jul 20, 2018
 *      Author: konstantinnosach
 */

#include "dyad.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#include "../tcp_listener.h"

static dyad_Stream *s;
static pthread_t worker;
struct TcpListener * tcp_listener;

void *update_dyad(void *threadid)
{
   while (dyad_getStreamCount() > 0) {
     dyad_update();
   }
   dyad_shutdown();
   pthread_exit(0);
}


static void onConnect(dyad_Event *e) {

}

static void onData(dyad_Event *e) {

  tcp_listener->prd_pt(e->data, e->size);

}

int open_tcp_connection(const char * host, int port, int sock_type, struct TcpListener * client) {

	tcp_listener = client;
	dyad_init();

	s = dyad_newStream();
	dyad_addListener(s, DYAD_EVENT_CONNECT, onConnect, NULL);
	dyad_addListener(s, DYAD_EVENT_DATA,    onData,    NULL);
	int result = dyad_connect(s, host, port, sock_type);
	long t = 1;
	int rc = pthread_create(&worker, NULL, update_dyad, (void *)t);
	if (rc){
		printf("ERROR; return code from pthread_create() is %d\n", rc);
		dyad_shutdown();
	    return -1;
	 }

	return result;
}

void close_tcp_connection() {
	dyad_shutdown();
}

void write_to_tcp_connection (void * data, int size) {

	dyad_write(s, data, size);

}
