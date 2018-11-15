/*
 * error_code.h
 *
 *  Created on: Oct 26, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_AVPS_ERROR_CODE_H_
#define SRC_AMQP_AVPS_ERROR_CODE_H_

enum ErrorCode {
	INTERNAL_ERROR,
	NOT_FOUND,
	UNAUTHORIZED_ACCESS,
	DECODE_ERROR,
	RESOURCE_LIMIT_EXCEEDED,
	NOT_ALLOWED,
	INVALID_FIELD,
	NOT_IMPLEMENTED,
	RESOURCE_LOCKED,
	PRECONDITION_FAILED,
	RESOURCE_DELETED,
	ILLEGAL_STATE,
	FRAME_SIZE_TOO_SMALL,
	CONNECTION_FORCED,
	FRAMING_ERROR,
	REDIRECTED,
	WINDOW_VIOLATION,
	ERRANT_LINK,
	HANDLE_IN_USE,
	UNATTACHED_HANDLE,
	DETACH_FORCED,
	TRANSFER_LIMIT_EXCEEDED,
	MESSAGE_SIZE_EXCEEDED,
	REDIRECT,
	STOLEN
};

static inline char *string_error_code(enum ErrorCode p)
{
    static char *strings[] = { "amqp:internal-error","amqp:not-found", "amqp:unauthorized-access", "amqp:decode-error","amqp:resource-limit-exceeded","amqp:not-allowed", "amqp:invalid-field", "amqp:not-implemented", "amqp:resource-locked", "amqp:precondition-failed", "amqp:resource-deleted", "amqp:illegal-state", "amqp:frame-size-too-small", "amqp:connection-forced", "amqp:framing-error", "amqp:redirected", "amqp:window-violation", "amqp:errant-link", "amqp:handle-in-use", "amqp:unattached-handle", "amqp:detach-forced", "amqp:transfer-limit-exceeded", "amqp:message-size-exceeded", "amqp:redirect", "amqp:stolen"};

    return strings[p];
}

#endif /* SRC_AMQP_AVPS_ERROR_CODE_H_ */
