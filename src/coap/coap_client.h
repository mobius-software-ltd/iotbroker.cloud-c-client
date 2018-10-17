/*
 * coap_client.h
 *
 *  Created on: Oct 11, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_COAP_COAP_CLIENT_H_
#define SRC_COAP_COAP_CLIENT_H_

#include "../mqtt_listener.h"
#include "../account.h"
#include "packets/coap_message.h"

int init_coap_client(struct Account * acc, struct MqttListener * listener);
void coap_send_ping(void);
void coap_encode_and_fire(struct CoapMessage * message);
void send_coap_disconnect(void);

#endif /* SRC_COAP_COAP_CLIENT_H_ */
