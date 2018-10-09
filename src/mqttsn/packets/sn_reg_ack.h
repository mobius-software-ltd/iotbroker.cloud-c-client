/*
 * sn_reg_ack.h
 *
 *  Created on: Aug 17, 2018
 *      Author: konstantinnosach
 */

#ifndef MQTTSN_PACKETS_SN_REG_ACK_H_
#define MQTTSN_PACKETS_SN_REG_ACK_H_

struct SnRegAck {

    unsigned short topic_id;
    unsigned short message_id;
    enum SnReturnCode code;

};

#endif /* MQTTSN_PACKETS_SN_REG_ACK_H_ */
