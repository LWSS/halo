/*
FLAG_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __FLAG_DEFINITIONS_H
#define __FLAG_DEFINITIONS_H
#pragma once

/* ---------- constants */

enum
{
	FLAG_DEFINITION_TAG = 'flag',
	FLAG_DEFINITION_VERSION = 1,
};

enum
{
	MAXIMUM_NUMBER_OF_ATTACHMENT_POINTS = 5,
};

enum
{
	_trailing_edge_shape_flat = 0,
	_trailing_edge_shape_concave_triangular,
	_trailing_edge_shape_convex_triangular,
	_trailing_edge_shape_trapezoid_short_top,
	_trailing_edge_shape_trapezoid_short_bottom,
	NUMBER_OF_TRAILING_EDGE_SHAPES,
};

enum
{
	_attached_edge_shape_flat = 0,
	_attached_edge_shape_triangular_notch,
	NUMBER_OF_ATTACHED_EDGE_SHAPES,
};

/* ---------- macros */

#define flag_definition_get(index) ((struct flag_definition *)tag_get(FLAG_DEFINITION_TAG, (index))) /* fake name */
#define flag_definition_try_and_get(index) ((index) == NONE ? (struct flag_definition *)NULL : flag_definition_get(index)) /* fake name */

/* ---------- structures */

struct flag_attachment_point
{
	short height_to_next_attachment;
	word pad;
	long unused[4];
	char marker_name[TAG_STRING_LENGTH+1];
};

struct flag_definition
{
	unsigned long flags;
	short trailing_edge_shape;
	short trailing_edge_offset;
	short attached_edge_shape;
	word pad;
	short width;
	short height;
	real cell_width_scale;
	real cell_height_scale;
	struct tag_reference shader_red;		// shader
	struct tag_reference physics;			// point_physics_definition
	real wind_noise;
	long unused2[2];
	struct tag_reference shader_blue;		// shader
	struct tag_block attachment_points;		// flag_attachment_point
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __FLAG_DEFINITIONS_H
