/*
 * sn_connect.h
 *
 *  Created on: Aug 14, 2018
 *      Author: konstantinnosach
 */

#ifndef MQTTSN_PACKETS_SN_CONNECT_H_
#define MQTTSN_PACKETS_SN_CONNECT_H_

struct SnConnect {

    int will_present;
    int clean_session;
    char protocol_id;
    unsigned short duration;
    const char * client_id;

};

#endif /* MQTTSN_PACKETS_SN_CONNECT_H_ */
