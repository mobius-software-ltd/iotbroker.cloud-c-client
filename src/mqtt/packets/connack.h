/*
 * conack.c
 *
 *  Created on: Jun 22, 2018
 *      Author: konstantinnosach
 */
enum connack_code {
	ACCEPTED = 0,
	UNACCEPTABLE_PROTOCOL_VERSION = 1,
	IDENTIFIER_REJECTED = 2,
	SERVER_UNUVALIABLE = 3,
	BAD_USER_OR_PASS = 4,
	NOT_AUTHORIZED = 5
};

struct Connack {
	int session_present;
	enum connack_code return_code;
};
