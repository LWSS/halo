/*
LIGHT_VOLUME_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __LIGHT_VOLUME_DEFINITIONS_H
#define __LIGHT_VOLUME_DEFINITIONS_H
#pragma once

/* ---------- constants */

enum
{
	LIGHT_VOLUME_DEFINITION_TAG = 'mgs2',
	LIGHT_VOLUME_DEFINITION_VERSION = 1,
};

enum
{
	_light_volume_color_interpolate_in_hsv_bit = 0,
	_light_volume_color_interpolate_along_farthest_hue_path_bit,
	NUMBER_OF_LIGHT_VOLUME_FLAGS,
};

/* ---------- macros */

#define light_volume_definition_get(index) ((struct light_volume_definition *)tag_get(LIGHT_VOLUME_DEFINITION_TAG, (index))) /* fake name */
#define light_volume_definition_try_and_get(index) ((index) == NONE ? (struct light_volume_definition *)NULL : light_volume_definition_get(index)) /* fake name */

/* ---------- structures */

struct light_volume_frame
{
	long unused1[4];
	real offset_from_marker;
	real offset_exponent;
	real length;
	long length_unused[8];
	real radius_hither;
	real radius_yon;
	real radius_exponent;
	long radius_unused[8];
	real_argb_color color_hither;
	real_argb_color color_yon;
	real color_exponent;
	real brightness_exponent;
	long color_unused[8];
};

struct light_volume_definition
{
	char attachment_marker[TAG_STRING_LENGTH+1];
	short type;
	word flags;
	long unused1[4];
	real near_fade_distance;
	real far_fade_distance;
	real perpendicular_brightness_scale;
	real parallel_brightness_scale;
	short brightness_scale_source;
	word pad1;
	long fade_unused[5];
	struct tag_reference map;	// bitmap_group
	short sequence_index;
	short count;
	long map_unused[18];
	short frame_animation_source;
	word pad2;
	long frame_animation_unused[9];
	long unused2[16];
	struct tag_block frames;	// light_volume_frame
	long unused3[8];
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __LIGHT_VOLUME_DEFINITIONS_H
