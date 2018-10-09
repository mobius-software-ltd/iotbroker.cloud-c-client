/*
 * sn_advertise.h
 *
 *  Created on: Aug 17, 2018
 *      Author: konstantinnosach
 */

#ifndef MQTTSN_PACKETS_SN_ADVERTISE_H_
#define MQTTSN_PACKETS_SN_ADVERTISE_H_

struct SnAdvertise {

    char gw_id;
    unsigned short duration;

};

#endif /* MQTTSN_PACKETS_SN_ADVERTISE_H_ */
