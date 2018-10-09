/*
 * SnRegister.h
 *
 *  Created on: Aug 17, 2018
 *      Author: konstantinnosach
 */

#ifndef MQTTSN_PACKETS_SN_REGISTER_H_
#define MQTTSN_PACKETS_SN_REGISTER_H_

struct SnRegister {

    unsigned short topic_id;
    unsigned short msg_id;
    const char * topic_name;
};


#endif /* MQTTSN_PACKETS_SN_REGISTER_H_ */
