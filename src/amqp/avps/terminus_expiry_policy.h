/*
 * terminus_expiry_policy.h
 *
 *  Created on: Oct 26, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_AVPS_TERMINUS_EXPIRY_POLICY_H_
#define SRC_AMQP_AVPS_TERMINUS_EXPIRY_POLICY_H_

enum TerminusExpiryPolicy {

	LINK_DETACH, SESSION_END, CONNETION_CLOSE, NEVER
};

static inline char *string_terminus_expiry_policy(enum TerminusExpiryPolicy p)
{
    static char *strings[] = { "link-detach", "session-end", "connection-close", "never" };

    return strings[p];
}


#endif /* SRC_AMQP_AVPS_TERMINUS_EXPIRY_POLICY_H_ */
