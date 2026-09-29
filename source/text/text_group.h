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

/* ---------- structures */

struct unicode_string_list_group_header
{
	struct tag_block string_references;		// unicode_string_list_string_reference
};

/* ---------- prototypes/TEXT_GROUP.C */

wchar_t *unicode_string_list_get_string(long tag_index, short string_index);

/* ---------- globals */

/* ---------- public code */

#endif // __TEXT_GROUP_H
