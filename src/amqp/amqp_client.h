/*
 * amqp_client.h
 *
 *  Created on: Oct 31, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_AMQP_CLIENT_H_
#define SRC_AMQP_AMQP_CLIENT_H_

#include "packets/amqp_header.h"
#include "../mqtt_listener.h"


int init_amqp_client(struct Account * acc, struct MqttListener * listener);
void amqp_encode_and_fire(struct AmqpHeader * header);
void send_amqp_ping (void);
void send_amqp_end(void);
#endif /* SRC_AMQP_AMQP_CLIENT_H_ */
