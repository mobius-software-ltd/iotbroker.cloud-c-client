/*
 * lws_net_client.c
 *
 *  Created on: Oct 23, 2018
 *      Author: konstantinnosach
 */
#include <libwebsockets.h>
#include <string.h>
#include <signal.h>
#include <jansson.h>
#include <pthread.h>
#include "../tcp_listener.h"

int interrupted;
struct lws_context *context;
static pthread_t worker;
struct TcpListener * tcp_listener;
struct per_vhost_data__minimal *vhd;
static const char * peer_host;
static int peer_port;

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

void * net_service_task(void *thread_id)
{
	int n = 0;
	while (n >= 0 && !interrupted) {
		n = lws_service(context, 1000);
	}
		lws_context_destroy(context);
		lwsl_user("Completed\n");
		pthread_exit(0);
}

static int connect_ws_client(struct per_vhost_data__minimal *vhd)
{
	vhd->i.context = vhd->context;
	vhd->i.port = peer_port;
	vhd->i.address = peer_host;
	vhd->i.path = "/ws";
	vhd->i.host = vhd->i.address;
	vhd->i.origin = vhd->i.address;
	vhd->i.ssl_connection = 0;

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

	default:
		break;
	}

	return lws_callback_http_dummy(wsi, reason, user, in, len);
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

int open_lws_net_connection(const char * host, int port, int sock_type, struct TcpListener * client) {

	tcp_listener = client;
	struct lws_context_creation_info info;
	peer_host = host;
	peer_port = port;

	signal(SIGINT, sigint_handler);

	memset(&info, 0, sizeof info); /* otherwise uninitialized garbage */
	info.port = CONTEXT_PORT_NO_LISTEN; /* we do not run any server */
	info.protocols = protocols;

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
	sleep(1);

	return 0;
}

void fire(char * s) {

	int s_len = strlen(s)+1;
	uint8_t message[LWS_PRE + s_len];
	int n = 0;
	n = lws_snprintf((char *)message + LWS_PRE, s_len, s);
	lws_write(vhd->client_wsi, message + LWS_PRE, n, LWS_WRITE_TEXT);

}