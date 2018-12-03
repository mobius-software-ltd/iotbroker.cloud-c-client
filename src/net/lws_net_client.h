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

#ifndef SRC_NET_LWS_NET_CLIENT_H_
#define SRC_NET_LWS_NET_CLIENT_H_

#include "../tcp_listener.h"
#include "../account.h"

int open_lws_net_connection(const char * host, int port, struct TcpListener * client, int _is_secure, const char * cert, const char * cert_password, enum Protocol protocol);
void fire(char * s);
void raw_fire(unsigned char * s, int len);
void lws_close_tcp_connection(void);

#endif /* SRC_NET_LWS_NET_CLIENT_H_ */
