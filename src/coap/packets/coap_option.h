/*
 * coap_option.h
 *
 *  Created on: Oct 11, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_COAP_PACKETS_COAP_OPTION_H_
#define SRC_COAP_PACKETS_COAP_OPTION_H_

struct CoapOption {

	int number;
	int length;
	const char * value;

};

#endif /* SRC_COAP_PACKETS_COAP_OPTION_H_ */
