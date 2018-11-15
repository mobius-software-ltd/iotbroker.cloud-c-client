/**
* Mobius Software LTD
* Copyright 2015-2018, Mobius Software LTD
*
* This is free software; you can redistribute it and/or modify it
* under the terms of the GNU Lesser General Public License as
* published by the Free Software Foundation; either version 2.1 of
* the License, or (at your option) any later version.
*
* This software is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this software; if not, write to the Free
* Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
* 02110-1301 USA, or see the FSF site: http://www.fsf.org.
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
