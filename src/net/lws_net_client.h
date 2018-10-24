/*
 * lws_net_client.h
 *
 *  Created on: Oct 23, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_NET_LWS_NET_CLIENT_H_
#define SRC_NET_LWS_NET_CLIENT_H_

#include "../tcp_listener.h"

int open_lws_net_connection(const char * host, int port, int sock_type, struct TcpListener * client);
void fire(char * s);

#endif /* SRC_NET_LWS_NET_CLIENT_H_ */
