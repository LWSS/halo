/*
VEHICLE_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __VEHICLE_DEFINITIONS_H
#define __VEHICLE_DEFINITIONS_H
#pragma once

/* ---------- headers */

#include "unit_definitions.h"
#include "physics_variables.h"

/* ---------- constants */

enum
{
	VEHICLE_DEFINITION_TAG = 'vehi',
	VEHICLE_DEFINITION_VERSION = 1,
};

enum
{
	_vehicle_human_tank = 0,
	_vehicle_human_jeep,
	_vehicle_human_boat,
	_vehicle_human_plane,
	_vehicle_alien_scout,
	_vehicle_alien_fighter,
	_vehicle_turret,
	NUMBER_OF_VEHICLE_DEFINITION_TYPES,
};

/* ---------- macros */

#define vehicle_definition_get(index) ((struct vehicle_definition *)tag_get(VEHICLE_DEFINITION_TAG, index))

/* ---------- structures */

struct _vehicle_definition
{
	unsigned long flags;
	short type;
	short pad;
	struct physics_variable_speed speed;
	struct physics_variable_position turn;
	real wheel_circumference;
	real turn_rate;
	real blur_speed;
	short function_modes[4];
	real unused0[3];
	struct physics_variable_speed slide;
	real flipping_angular_velocity_min;
	real flipping_angular_velocity_max;
	real unused1[6];
	real fixed_gun_yaw;
	real fixed_gun_pitch;
	real unused2[6];
	real ai_sideslip_distance;
	real ai_destination_radius;
	real ai_avoidance_distance;
	real ai_pathfinding_radius;
	real ai_charge_repeat_time;
	real ai_strafing_stop_range;
	real ai_oversteer_angle_lower_bound;
	real ai_oversteer_angle_upper_bound;
	real ai_steering_max_angle;
	real ai_steering_max_throttle;
	real ai_movement_max_time;
	real ai_unused;
	struct tag_reference suspension_sound;
	struct tag_reference crash_sound;
	struct tag_reference material_effects;
	struct tag_reference effect;
};

struct vehicle_definition
{
	struct _object_definition object;
	struct _unit_definition unit;
	struct _vehicle_definition vehicle;
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __VEHICLE_DEFINITIONS_H
