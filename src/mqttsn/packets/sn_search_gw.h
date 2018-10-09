/*
 * sn_search_gw.h
 *
 *  Created on: Aug 17, 2018
 *      Author: konstantinnosach
 */

#ifndef MQTTSN_PACKETS_SN_SEARCH_GW_H_
#define MQTTSN_PACKETS_SN_SEARCH_GW_H_

enum Radius { BROADCAST = 0, RADIUS_1 = 1, RADIUS_2 = 2, RADIUS_3 = 3 };

struct SnSearchGw {

	enum Radius radius;

};

#endif /* MQTTSN_PACKETS_SN_SEARCH_GW_H_ */
