/*
 * sn_encapsulated.h
 *
 *  Created on: Aug 17, 2018
 *      Author: konstantinnosach
 */

#ifndef MQTTSN_PACKETS_SN_ENCAPSULATED_H_
#define MQTTSN_PACKETS_SN_ENCAPSULATED_H_

struct SnEncapsulated {

    enum Radius radius;
    char * wireless_node_id;
    struct SnMessage * message;

};

#endif /* MQTTSN_PACKETS_SN_ENCAPSULATED_H_ */
