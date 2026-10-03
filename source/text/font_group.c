/*
FONT_GROUP.C

*/

/* ---------- headers */

#include "cseries.h"
#include "font_group.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "rasterizer.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

struct font_character *font_get_character_by_ascii_code(
	struct font_header *header,
	word character)
{
	struct font_character_tables_entry *character_table = TAG_BLOCK_GET_ELEMENT(&header->character_tables, character >> 8, struct font_character_tables_entry);
	struct font_character *font_character = NULL;

	if (character_table->table.count > 0)
	{
		struct font_character_table_entry *entry = character_table->table.count == 256 ?
			TAG_BLOCK_GET_ELEMENT(&character_table->table, character & UNSIGNED_CHAR_MAX, struct font_character_table_entry) : NULL;

		if (entry->character_index != NONE)
		{
			font_character = TAG_BLOCK_GET_ELEMENT(&header->characters, entry->character_index, struct font_character);
		}
	}

	return font_character;
}

/* ---------- private code */
