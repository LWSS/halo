/*
TEXT_GROUP.H

header included in hcex build.
*/

#ifndef __TEXT_GROUP_H
#define __TEXT_GROUP_H
#pragma once

/* ---------- headers */

#include "unicode.h"

/* ---------- constants */

enum
{
	STRING_LISTS_GROUP_TAG = 'str#',
	STRING_LISTS_GROUP_VERSION = 1,
	UNICODE_STRING_LISTS_GROUP_TAG = 'ustr',
	UNICODE_STRING_LISTS_GROUP_VERSION = 1
};

/* ---------- macros */

#define unicode_string_list_definition_get(index) ((struct unicode_string_list_group_header *)tag_get(UNICODE_STRING_LISTS_GROUP_TAG, (index))) /* fake name */

/* ---------- structures */

struct string_list_string_reference
{
	struct tag_data string;
};

struct string_list_group_header
{
	struct tag_block string_references;		// string_list_string_reference
};

struct unicode_string_list_string_reference
{
	struct tag_data string;
};

struct unicode_string_list_group_header
{
	struct tag_block string_references;		// unicode_string_list_string_reference
};

/* ---------- prototypes/TEXT_GROUP.C */

char *string_list_get_string(long tag_index, short string_index);
wchar_t *unicode_string_list_get_string(long tag_index, short string_index);

/* ---------- globals */

/* ---------- public code */

#endif // __TEXT_GROUP_H
