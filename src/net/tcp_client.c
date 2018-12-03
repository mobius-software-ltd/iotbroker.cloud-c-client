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

#include "dyad.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../tcp_listener.h"
#include "../helpers.h"

static dyad_Stream *s;
static pthread_t worker;
struct TcpListener * tcp_listener;

void *update_dyad(void *threadid)
{
	while (dyad_getStreamCount() > 0) {
		dyad_update();
	}
	dyad_shutdown();
	printf("Net service stopped.\n");
	return 0;
}


static void onConnect(dyad_Event *e) {

}

static void onData(dyad_Event *e) {

	tcp_listener->prd_pt(e->data, e->size);
}

int init_net_service(const char * host, int port, int sock_type, struct TcpListener * client) {

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

void stop_net_service() {
	dyad_shutdown();
}

void write_to_net (void * data, int size) {

	dyad_write(s, data, size);

}
