/*
 * tcp_client.h
 *
 *  Created on: Jul 20, 2018
 *      Author: konstantinnosach
 */

#ifndef NET_TCP_CLIENT_H_
#define NET_TCP_CLIENT_H_

#include "../tcp_listener.h"

int open_tcp_connection(const char * host, int port, int sock_type, struct TcpListener * iot_client);
void close_tcp_connection();
void write_to_tcp_connection(char * buff, int size);

#endif /* NET_TCP_CLIENT_H_ */
