/*
 * iot_client.h
 *
 *  Created on: Jul 22, 2018
 *      Author: konstantinnosach
 */

#ifndef TCP_LISTENER_H_
#define TCP_LISTENER_H_

typedef void (*ProcessResponseData)(char * data, int length);

struct TcpListener {

	ProcessResponseData  prd_pt;
};

#endif /* TCP_LISTENER_H_ */
