/*
INTERMEDIATE_GEOMETRY.H

header included in hcex build.
*/

#ifndef __INTERMEDIATE_GEOMETRY_H
#define __INTERMEDIATE_GEOMETRY_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct intermediate_vertex // [fake name?]
{
	char __unknown0[8];
	real_point3d position;
	char __unknown14[60];
};

struct intermediate_triangle // [fake name?]
{
	char __unknown0[8];
	long vertex_indices[NUMBER_OF_VERTICES_PER_TRIANGLE];
	char __unknown14[32];
};

struct intermediate_geometry
{
	char __unknown0[308];
	struct dynamic_array triangles;
	struct dynamic_array vertices;
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __INTERMEDIATE_GEOMETRY_H
