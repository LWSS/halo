/*
PHYSICS_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __PHYSICS_DEFINITIONS_H
#define __PHYSICS_DEFINITIONS_H
#pragma once

/* ---------- headers */

#include "math/real_math.h"
#include "tag_files/tag_files.h"
#include "tag_files/tag_groups.h"

/* ---------- constants */

enum
{
	PHYSICS_DEFINITION_TAG = 'phys',
	PHYSICS_DEFINITION_VERSION = 4,
	MAXIMUM_POWERED_MASS_POINTS_PER_PHYSICS = 32,
	MAXIMUM_MASS_POINTS_PER_PHYSICS = 32,
};

enum
{
	_physics_inertial_matrix = 0,
	_physics_inertial_matrix_inverse,
	MAXIMUM_PHYSICS_INERTIAL_MATRICES,
};

enum
{
	_powered_mass_point_ground_friction_bit = 0,
	_powered_mass_point_water_friction_bit,
	_powered_mass_point_air_friction_bit,
	_powered_mass_point_water_lift_bit,
	_powered_mass_point_air_lift_bit,
	_powered_mass_point_thrust_bit,
	_powered_mass_point_antigrav_bit,
	NUMBER_OF_POWERED_MASS_POINT_DEFINITION_FLAGS,
};

enum
{
	_friction_type_point = 0,
	_friction_type_forward,
	_friction_type_left,
	_friction_type_up,
	NUMBER_OF_FRICTION_TYPES,
};

enum
{
	_mass_point_metallic_bit = 0,
	NUMBER_OF_MASS_POINT_DEFINITION_FLAGS,
};

/* ---------- macros */

#define physics_definition_get(index) ((struct physics_definition *)tag_get(PHYSICS_DEFINITION_TAG, index))

/* ---------- structures */

struct powered_mass_point_definition
{
	char name[TAG_STRING_LENGTH+1];
	unsigned long flags;
	real antigrav_strength;
	real antigrav_offset;
	real antigrav_height;
	real antigrav_damp_fraction;
	real antigrav_normal_k1;
	real antigrav_normal_k0;
	real unused[17];
};

struct mass_point_definition
{
	char name[TAG_STRING_LENGTH+1];
	short powered_mass_point_index;
	short model_node_index;
	unsigned long flags;
	real relative_mass;
	real mass;
	real relative_density;
	real density;
	real_point3d position;
	real_vector3d forward;
	real_vector3d up;
	short friction_type;
	short pad;
	real friction_parallel_scale;
	real friction_perpendicular_scale;
	real radius;
	real unused[5];
};

struct physics_definition
{
	real radius;
	real moment;
	real mass;
	real_point3d center_of_mass;
	real density;
	real gravity_scale;
	real ground_friction;
	real ground_depth;
	real ground_damp_fraction;
	real ground_normal_k1;
	real ground_normal_k0;
	real ground_unused;
	real water_friction;
	real water_depth;
	real water_density;
	real water_unused;
	real air_friction;
	real air_unused;
	real xx_moment;
	real yy_moment;
	real zz_moment;
	struct tag_block inertial_matrix;
	struct tag_block powered_mass_points;
	struct tag_block mass_points;
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __PHYSICS_DEFINITIONS_H
