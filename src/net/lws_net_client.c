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
#include <libwebsockets.h>
#include <glib.h>
#include <string.h>
#include <signal.h>
#include <jansson.h>
#include <pthread.h>
#include <unistd.h>
#include "../tcp_listener.h"
#include "../account.h"
#include "data.h"

static char file_name_template [] = "/tmp/c_client_temp.XXXXXX";
int interrupted;
struct lws_context *context;
static pthread_t worker;
struct TcpListener * tcp_listener;
struct per_vhost_data__minimal *vhd;
static const char * peer_host;
static int peer_port;
static int is_secure;
static char * filename = NULL;

static enum Protocol current_protocol;

static GQueue *q = NULL;

struct per_vhost_data__minimal {
	struct lws_context *context;
	struct lws_vhost *vhost;
	const struct lws_protocols *protocol;

	struct lws_client_connect_info i;
	struct lws *client_wsi;

	int counter;
	char finished;
	char established;
};

static void * net_service_task(void *thread_id)
{
	int n = 0;
	while (n >= 0 && !interrupted) {
		n = lws_service(context, 500);
	}
	lws_context_destroy(context);
	//printf("Net service stopped\n");
	//pthread_cancel(worker);
	g_queue_free(q);
	q=NULL;
	vhd = NULL;
	context = NULL;
	interrupted = 0;
	return 0;
}

static int connect_ws_client(struct per_vhost_data__minimal *vhd)
{
	vhd->i.context = vhd->context;
	vhd->i.port = peer_port;
	vhd->i.address = peer_host;
	if(current_protocol != WEBSOCKETS)
		vhd->i.path = "/";
	else
		vhd->i.path = "/ws";
	vhd->i.host = vhd->i.address;
	vhd->i.origin = vhd->i.address;
	if(is_secure)
		vhd->i.ssl_connection = LCCSCF_USE_SSL;
	else
		vhd->i.ssl_connection = 0;
	if(current_protocol != WEBSOCKETS)
		vhd->i.method = "RAW";

	vhd->i.protocol = "lws-minimal-broker";
	vhd->i.pwsi = &vhd->client_wsi;

	return !lws_client_connect_via_info(&vhd->i);
}

static int callback_minimal_broker(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	vhd = (struct per_vhost_data__minimal *) lws_protocol_vh_priv_get(lws_get_vhost(wsi), lws_get_protocol(wsi));

	switch (reason) {

	case LWS_CALLBACK_PROTOCOL_INIT:
		vhd = lws_protocol_vh_priv_zalloc(lws_get_vhost(wsi), lws_get_protocol(wsi), sizeof(struct per_vhost_data__minimal));
		vhd->context = lws_get_context(wsi);
		vhd->protocol = lws_get_protocol(wsi);
		vhd->vhost = lws_get_vhost(wsi);
		if (connect_ws_client(vhd))
			lws_timed_callback_vh_protocol(vhd->vhost, vhd->protocol, LWS_CALLBACK_USER, 1);
		break;

	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		lwsl_err("CLIENT_CONNECTION_ERROR: %s\n", in ? (char *)in : "(null)");
		vhd->client_wsi = NULL;
		lws_timed_callback_vh_protocol(vhd->vhost, vhd->protocol, LWS_CALLBACK_USER, 1);
		break;

	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		lwsl_user("%s: established\n", __func__);
		vhd->established = 1;
		break;

	case LWS_CALLBACK_CLIENT_CLOSED:
		vhd->client_wsi = NULL;
		vhd->established = 0;
		lws_timed_callback_vh_protocol(vhd->vhost, vhd->protocol, LWS_CALLBACK_USER, 1);
		break;

	case LWS_CALLBACK_CLIENT_RECEIVE:
		tcp_listener->prd_pt((char *)in, len);

		break;
	case LWS_CALLBACK_RAW_RX: {
		tcp_listener->prd_pt((char *)in, len);
	}
		break;

	case LWS_CALLBACK_RAW_CLOSE: {
		tcp_listener->stop_pt();
		interrupted = 1;
	}
	break;
	case LWS_CALLBACK_CLIENT_WRITEABLE: {

		struct DataWrapper * pending_data  = g_queue_pop_head(q);
		if(pending_data != NULL)
		{
			char buf[LWS_PRE + pending_data->length];
			memcpy(&buf[LWS_PRE],pending_data->data, pending_data->length);
			lws_write(vhd->client_wsi, &buf[LWS_PRE], pending_data->length, LWS_WRITE_TEXT);
			if(!g_queue_is_empty(q))
				lws_callback_on_writable(vhd->client_wsi);
		}

	}
	break;
	case LWS_CALLBACK_RAW_WRITEABLE: {

		struct DataWrapper * pending_data  = g_queue_pop_head(q);
		lws_write(vhd->client_wsi, pending_data->data, pending_data->length , LWS_WRITE_RAW);
		if(!g_queue_is_empty(q))
			lws_callback_on_writable(vhd->client_wsi);
	}
	break;

	case LWS_CALLBACK_RAW_ADOPT: {
		vhd->established = 1;
	}
	break;
	default:
		break;
	}

	return 0;
}

static const struct lws_protocols protocols[] = {
	{
		"lws-minimal-broker",
		callback_minimal_broker,
		0,
		0,
	},
	{ NULL, NULL, 0, 0 }
};

static void sigint_handler(int sig)
{
	interrupted = 1;
}

int open_lws_net_connection(const char * host, int port, struct TcpListener * client,
		int _is_secure, const char * cert, const char * cert_password, enum Protocol protocol) {

	lws_set_log_level(0, NULL);

	q = g_queue_new();

	current_protocol = protocol;
	tcp_listener = client;
	is_secure = _is_secure;
	struct lws_context_creation_info info;
	peer_host = host;
	peer_port = port;

	signal(SIGINT, sigint_handler);

	memset(&info, 0, sizeof info); /* otherwise uninitialized garbage */
	info.port = CONTEXT_PORT_NO_LISTEN; /* we do not run any server */
	info.protocols = protocols;
	if(is_secure) {
		info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
		if(cert != NULL) {
			filename = malloc((strlen(file_name_template) + 1) * sizeof(char));
			strcpy(filename, file_name_template);
			int fd = mkstemp(filename);
			if (fd == -1) {
				printf("cannot open file!!!\n");
				return -1;
			}
			int size_data = write(fd, cert, strlen(cert));
			if(size_data <= 0) {
				printf("Error : Cannot write certificate in file!!!\n");
				close(fd);
				unlink(filename);
				return -1;
			}
			close(fd);
			//we can add here protocol name MQTT,COAP, etc.
			info.alpn = "";
			info.ssl_private_key_password = cert_password;
			//possibly it's libwebsocket bug client_ssl_private_key_password not working
			info.client_ssl_private_key_password = cert_password;
			info.client_ssl_private_key_filepath = filename;
			info.client_ssl_cert_filepath = filename;

		}
	}

	context = lws_create_context(&info);
	if (!context) {
		printf("lws init failed\n");
		return -1;
	}
	long t = 1;
	int rc = pthread_create(&worker, NULL, net_service_task, (void *)t);
	if (rc){
		printf("ERROR; return code from pthread_create() is %d\n", rc);
		return -1;
	 }

	int counter = 0;
	while(vhd == NULL ||( !vhd->established && counter < 15)) {
		counter++;
		usleep(200*1000);
	}

	if(filename!= NULL)
		unlink(filename);

	if(vhd->established)
		return 0;
	else
		return -1;
}

void fire(char * s) {

	struct DataWrapper * data = malloc (sizeof (struct DataWrapper));
	data->length = strlen(s);
	data->data = malloc(sizeof(char*)*(data->length+1));
	strcpy(data->data, s);

	if(q!=NULL)
		g_queue_push_tail(q, data);
	lws_callback_on_writable(vhd->client_wsi);

}

void raw_fire(char * s, int len) {

	struct DataWrapper * data = malloc (sizeof (struct DataWrapper));
	data->data = malloc(sizeof(char*)*len);
	memcpy(data->data, s, len);
	data->length = len;
	if(q!=NULL)
		g_queue_push_tail(q, data);
	lws_callback_on_writable(vhd->client_wsi);

}

void lws_close_tcp_connection() {
	interrupted = 1;
}
