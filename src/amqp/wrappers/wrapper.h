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
