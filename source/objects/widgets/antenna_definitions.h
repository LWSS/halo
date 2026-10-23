/*
ANTENNA_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __ANTENNA_DEFINITIONS_H
#define __ANTENNA_DEFINITIONS_H
#pragma once

/* ---------- constants */

enum
{
	ANTENNA_DEFINITION_TAG = 'ant!',
	ANTENNA_DEFINITION_VERSION = 1,
};

enum
{
	MAXIMUM_NUMBER_OF_ANTENNA_VERTICES = 20,
};

/* ---------- macros */

#define antenna_definition_get(index) ((struct antenna_definition *)tag_get(ANTENNA_DEFINITION_TAG, (index))) /* fake name */
#define antenna_definition_try_and_get(index) ((index) == NONE ? (struct antenna_definition *)NULL : antenna_definition_get(index)) /* fake name */

/* ---------- structures */

struct antenna_vertex
{
	real spring_coefficient;
	long physics_unused[6];
	real_euler_angles2d angle_to_next;
	real length_to_next;
	short sequence_index;
	word pad;
	real_argb_color color;
	real_argb_color lod_color;
	long unused[10];
	real_vector3d vector_to_next;
};

struct antenna_definition
{
	char attachment_marker[TAG_STRING_LENGTH+1];
	struct tag_reference texture;			// bitmap_group
	struct tag_reference physics;			// point_physics_definition
	long reference_unused[20];
	real spring_coefficient;
	real falloff_pixels;
	real cutoff_pixels;
	real runtime_total_length;
	long unused[9];
	struct tag_block vertices;				// antenna_vertex
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __ANTENNA_DEFINITIONS_H
