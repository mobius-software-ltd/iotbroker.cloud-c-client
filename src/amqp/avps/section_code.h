/*
 * section_code.h
 *
 *  Created on: Oct 26, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_AVPS_SECTION_CODE_H_
#define SRC_AMQP_AVPS_SECTION_CODE_H_

enum SectionCode {

	HEADER = 0x70,
	DELIVERY_ANNOTATIONS = 0x71,
	MESSAGE_ANNOTATIONS = 0x72,
	PROPERTIES = 0x73,
	APPLICATION_PROPERTIES = 0x74,
	DATA = 0x75,
	SEQUENCE = 0x76,
	VALUE = 0x77,
	FOOTER = 0x78
};

#endif /* SRC_AMQP_AVPS_SECTION_CODE_H_ */
