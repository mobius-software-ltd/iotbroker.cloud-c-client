/*
 * amqp_calc.h
 *
 *  Created on: Nov 7, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_AMQP_CALC_H_
#define SRC_AMQP_AMQP_CALC_H_

char * get_bytes (struct TlvList * list);
void calculate_bytes(struct TlvList * tlv_list, int header_code);

#endif /* SRC_AMQP_AMQP_CALC_H_ */
