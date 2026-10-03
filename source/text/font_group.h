/*
FONT_GROUP.H

header included in hcex build.
*/

#ifndef __FONT_GROUP_H
#define __FONT_GROUP_H
#pragma once

/* ---------- headers */

#include "draw_string.h"

/* ---------- constants */

enum
{
	FONT_GROUP_TAG = 'font',
	FONT_GROUP_VERSION = 1,
};

/* ---------- macros */

#define font_definition_get(index) ((struct font_header *)tag_get(FONT_GROUP_TAG, index))

/* ---------- structures */

struct font_character_table_entry
{
	short character_index;
};

struct font_character_tables_entry
{
	struct tag_block table;		// font_character_table_entry
};

struct font_character
{
	word character;
	short character_width;
	short bitmap_width;
	short bitmap_height;
	short bitmap_origin_x;
	short bitmap_origin_y;
	short hardware_character_index;
	word pad;
	long pixels_offset;
};

struct font_header
{
	unsigned long flags;
	short ascending_height;
	short descending_height;
	short leading_height;
	short leading_width;
	long pad[9];
	struct tag_block character_tables;		// font_character_tables_entry
	struct tag_reference style_fonts[NUMBER_OF_TEXT_STYLES];
	struct tag_block characters;			// font_character
	struct tag_data pixels;
};

/* ---------- prototypes/FONT_GROUP.C */

struct font_character *font_get_character_by_ascii_code(struct font_header *header, word character);

/* ---------- globals */

/* ---------- public code */

#endif // __FONT_GROUP_H
