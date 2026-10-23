/*
LIGHTNING_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __LIGHTNING_DEFINITIONS_H
#define __LIGHTNING_DEFINITIONS_H
#pragma once

/* ---------- constants */

enum
{
	LIGHTNING_DEFINITION_TAG = 'elec',
	LIGHTNING_DEFINITION_VERSION = 1,
	MAXIMUM_MARKERS_PER_LIGHTNING = 16,
	MAXIMUM_LIGHTNING_MARKER_OCTAVES = 8,
};

enum
{
	_lightning_marker_not_connected_to_next_marker_bit = 0,
	NUMBER_OF_LIGHTNING_MARKER_FLAGS,
};

/* ---------- macros */

#define lightning_definition_get(index) ((struct lightning_definition *)tag_get(LIGHTNING_DEFINITION_TAG, (index))) /* fake name */
#define lightning_definition_try_and_get(index) ((index) == NONE ? (struct lightning_definition *)NULL : lightning_definition_get(index)) /* fake name */

/* ---------- structures */

struct lightning_marker_definition
{
	char attachment_marker[TAG_STRING_LENGTH+1];
	word flags;
	short type;
	short octaves_to_next_marker;
	word pad;
	long unused1[19];
	real_vector3d random_position_bounds;
	real random_jitter_offset;
	real thickness;
	real_argb_color tint;
	long unused2[19];
};

struct lightning_definition
{
	word flags;
	short count;
	long unused1[4];
	real near_fade_distance;
	real far_fade_distance;
	long unused2[4];
	short jitter_scale_source;
	short thickness_scale_source;
	short tint_modulation_source;
	short brightness_scale_source;
	struct tag_reference map;				// bitmap_group
	long unused3[21];
	struct tag_block markers;				// lightning_marker_definition
	struct tag_block shaders;				// shader_effect
	long unused4[22];
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __LIGHTNING_DEFINITIONS_H
