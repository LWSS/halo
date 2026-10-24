/*
MESSAGE_HEADER.H

header included in hcex build.
*/

#ifndef __MESSAGE_HEADER_H
#define __MESSAGE_HEADER_H
#pragma once

/* ---------- constants */

enum
{
	MAXIMUM_MESSAGE_SIZE = 0xFFF,
	NUMBER_OF_MESSAGE_TYPES = 4,
	MESSAGE_FLAG_BITS_MASK = 3,
	_message_type_bit0 = 2,
	_message_size_bit0 = 4,
};

enum
{
	_message_type_unknown = 0,
	_message_type_error,
	_message_type_data,
	_message_type_packet,
};

enum
{
	MAXIMUM_ERROR_STRING_LENGTH = 128,
};

enum
{
	_message_flag_encrypted_bit = 0,
	_message_flag_new_encryption_key_bit,
	NUMBER_OF_MESSAGE_FLAGS,
};

enum
{
	_byte_order_host = 0,
	_byte_order_network,
};

/* ---------- macros */

#define GET_MESSAGE_FLAGS(message) ((message)&MESSAGE_FLAG_BITS_MASK) /* fake name */
#define GET_MESSAGE_TYPE(message) (((message)>>_message_type_bit0)&(NUMBER_OF_MESSAGE_TYPES-1))
#define GET_MESSAGE_SIZE(message) ((message)>>_message_size_bit0)
#define GET_MESSAGE_PACKET_TYPE(message, message_size) (((byte *)(message))[(message_size) - 1]) /* fake name */
#define SET_MESSAGE_FLAGS(message, flags) ((message) = ((message)&~MESSAGE_FLAG_BITS_MASK)|(flags)) /* fake name */
#define SET_MESSAGE_TYPE(message, type) ((message) = ((message)&(word)~((NUMBER_OF_MESSAGE_TYPES-1)<<_message_type_bit0))|(((type)&(NUMBER_OF_MESSAGE_TYPES-1))<<_message_type_bit0)) /* fake name */
#define SET_MESSAGE_SIZE(message, size) ((message) = ((message)&(FLAG(_message_size_bit0)-1))|((size)<<_message_size_bit0)) /* fake name */

/* ---------- structures */

typedef word message_header;

struct message_error /* fake name */
{
	char error_string[MAXIMUM_ERROR_STRING_LENGTH]; /* fake name */
	byte error_code; /* fake name */
};

/* ---------- prototypes/MESSAGE_HEADER.C */

message_header *create_message(byte type, void const *data, word data_size, void *buffer, word buffer_size);
void build_message_header(message_header *msg, word length, byte type, byte flags);
void byte_swap_message_header(message_header *header, long desired_order);

/* ---------- globals */

/* ---------- public code */

#endif // __MESSAGE_HEADER_H
