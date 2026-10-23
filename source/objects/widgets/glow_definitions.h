/*
GLOW_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __GLOW_DEFINITIONS_H
#define __GLOW_DEFINITIONS_H
#pragma once

/* ---------- constants */

enum
{
	GLOW_DEFINITION_TAG = 'glw!',
	GLOW_DEFINITION_VERSION = 1,
};

enum
{
	_glow_effect_boundary_type_bounce = 0,
	_glow_effect_boundary_type_wrap,
	NUMBER_OF_GLOW_EFFECT_BOUNDARY_TYPES,
};

enum
{
	_glow_particle_distribution_type_random = 0,
	_glow_particle_distribution_type_uniform,
	NUMBER_OF_NORMAL_PARTICLE_DISTRIBUTION_TYPES,
};

enum
{
	_glow_trailing_particle_emission_vertical = 0,
	_glow_trailing_particle_emission_normal,
	_glow_trailing_particle_emission_random,
	NUMBER_OF_TRAILING_PARTICLE_EMISSION_PATTERNS,
};

enum
{
	_glow_interpolate_color_per_particle_bit = 0,
	_glow_particles_initial_direction_negative_bit,
	_glow_particles_move_in_both_directions_bit,
	_glow_trailing_particles_fade_over_time_bit,
	_glow_trailing_particles_shrink_over_time_bit,
	_glow_trailing_particles_slow_over_time_bit,
	NUMBER_OF_GLOW_FLAGS,
};

/* ---------- macros */

#define glow_definition_get(index) ((struct glow_definition *)tag_get(GLOW_DEFINITION_TAG, (index))) /* fake name */
#define glow_definition_try_and_get(index) ((index) == NONE ? (struct glow_definition *)NULL : glow_definition_get(index)) /* fake name */

/* ---------- structures */

struct glow_definition
{
	char attachment_marker[TAG_STRING_LENGTH+1];
	short number_of_particles;
	short boundary_effect;
	short normal_particle_distribution;
	short trailing_particle_distribution;
	unsigned long flags;
	long unused0[7];
	short render_mode;
	short render_orientation;
	long render_flags;
	short particle_rotational_velocity_attachment_index;
	word uw0;
	real glow_particle_rotational_velocity;
	real glow_particle_rotational_velocity_scale_lower_bound;
	real glow_particle_rotational_velocity_scale_upper_bound;
	short effect_rotational_velocity_attachment_index;
	word uw1;
	real glow_effect_rotational_velocity;
	real glow_effect_rotational_velocity_scale_lower_bound;
	real glow_effect_rotational_velocity_scale_upper_bound;
	short effect_translational_velocity_attachment_index;
	word uw2;
	real glow_effect_translational_velocity;
	real glow_effect_translational_velocity_scale_lower_bound;
	real glow_effect_translational_velocity_scale_upper_bound;
	short distance_to_object_attachment_index;
	word uw3;
	real minimum_distance_glow_particle_to_object;
	real maximum_distance_glow_particle_to_object;
	real distance_to_object_scale_lower_bound;
	real distance_to_object_scale_upper_bound;
	long unused2[2];
	short particle_size_attachment_index;
	word uw4;
	real particle_size_lower_bound;
	real particle_size_upper_bound;
	real particle_size_scale_lower_bound;
	real particle_size_scale_upper_bound;
	short color_attachment_index;
	word uw5;
	real_argb_color color_bound_0;
	real_argb_color color_bound_1;
	real_argb_color scale_color_0;
	real_argb_color scale_color_1;
	real color_rate_of_change;
	real percentage_edge_fade;
	real trailing_particle_generation_frequency;
	real trailing_particle_lifetime;
	real trailing_particle_velocity;
	real trailing_particle_minimum_normalized_t;
	real trailing_particle_maximum_normalized_t;
	long unused1[13];
	struct tag_reference texture;	// bitmap_group
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __GLOW_DEFINITIONS_H
