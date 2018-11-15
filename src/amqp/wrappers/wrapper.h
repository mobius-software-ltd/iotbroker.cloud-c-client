/*
 * wrapper.h
 *
 *  Created on: Oct 30, 2018
 *      Author: konstantinnosach
 */

#ifndef SRC_AMQP_WRAPPERS_WRAPPER_H_
#define SRC_AMQP_WRAPPERS_WRAPPER_H_

struct TlvAmqp * wrap_string (char * s);
struct TlvAmqp * wrap_long (long * value);
struct TlvAmqp * wrap_int (int * value);
struct TlvAmqp * wrap_symbol (struct AmqpSymbol * symbol);
struct TlvAmqp * wrap_binary (char * array, int length);
struct TlvAmqp * wrap_null ();
struct TlvAmqp * wrap_array (void * p);
struct TlvAmqp * wrap_map (void * p);
struct TlvAmqp * wrap_bool (int * value);
struct TlvAmqp * wrap_amqp_target (struct AmqpTarget * target);
struct TlvAmqp * wrap_amqp_source (struct AmqpSource * target);
struct TlvAmqp * wrap_short (short * value);
struct TlvAmqp * wrap_amqp_error (struct AmqpError * error);
struct TlvAmqp * wrap_state (struct AmqpState * state);

#endif /* SRC_AMQP_WRAPPERS_WRAPPER_H_ */
