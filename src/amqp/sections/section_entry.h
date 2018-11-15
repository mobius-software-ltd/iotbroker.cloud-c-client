/*
 * section_entry.h
 *
 *  Created on: Oct 26, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_SECTIONS_SECTION_ENTRY_H_
#define SRC_AMQP_SECTIONS_SECTION_ENTRY_H_

#include "../avps/section_code.h"

struct SectionEntry {

	enum SectionCode code;
	void * section;
};

#endif /* SRC_AMQP_SECTIONS_SECTION_ENTRY_H_ */
