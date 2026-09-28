/*
FOG_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __FOG_DEFINITIONS_H
#define __FOG_DEFINITIONS_H
#pragma once

/* ---------- headers */

#include "real_math.h"
#include "tag_groups.h"

/* ---------- constants */

enum
{
	PLANAR_FOG_DEFINITION_TAG = 'fog ',
	PLANAR_FOG_DEFINITION_VERSION = 1,
};

/* ---------- macros */

#define fog_definition_get(index) ((struct fog_definition *)tag_get(PLANAR_FOG_DEFINITION_TAG, index))

/* ---------- structures */

struct fog_screen
{
	unsigned short flags;
	short layer_count;
	real near_distance;
	real far_distance;
	real near_density;
	real far_density;
	real start_distance_from_fog_plane;
	long unused1[1];
	unsigned long color;
	real rotation_multiplier;
	real strafing_multiplier;
	real zoom_multiplier;
	long unused2[2];
	real map_scale;
	struct tag_reference map;
	real animation_period;
	real animation_unused[1];
	real wind_velocity_lower_bound;
	real wind_velocity_upper_bound;
	real wind_period_lower_bound;
	real wind_period_upper_bound;
	real wind_acceleration_weight;
	real wind_perpendicular_weight;
	long wind_unused[2];
};

struct fog_definition
{
	unsigned long flags;
	real animation_distance;
	long animation_unused[19];
	long unused1[1];
	real maximum_density;
	long unused2[1];
	real maximum_distance;
	long unused3[1];
	real maximum_depth;
	long unused4[2];
	real distance_to_water_plane;
	real_rgb_color color;
	struct fog_screen screen;
	struct tag_reference background_sound;
	struct tag_reference sound_environment;
	long sound_unused[30];
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __FOG_DEFINITIONS_H
