/*
DEVICE_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __DEVICE_DEFINITIONS_H
#define __DEVICE_DEFINITIONS_H
#pragma once

/* ---------- headers */

#include "object_definitions.h"

/* ---------- constants */

enum
{
	DEVICE_DEFINITION_TAG = 'devi',
	DEVICE_DEFINITION_VERSION = 1,
};

enum
{
	_device_position_loops_bit = 0,
	_device_position_animation_not_interpolated_bit,
	NUMBER_OF_DEVICE_DEFINITION_FLAGS,
};

enum
{
	_device_function_none = 0,
	_device_function_power,
	_device_function_change_in_power,
	_device_function_position,
	_device_function_change_in_position,
	_device_function_locked,
	_device_function_delay,
	NUMBER_OF_DEVICE_FUNCTION_MODES,
};

enum
{
	_device_animation_position = 0,
	_device_animation_power,
	NUMBER_OF_DEVICE_ANIMATIONS,
};

enum
{
	MACHINE_DEFINITION_TAG = 'mach',
	MACHINE_DEFINITION_VERSION = 1,
};

enum
{
	_machine_door = 0,
	_machine_platform,
	_machine_gear,
	NUMBER_OF_MACHINE_TYPES,
};

enum
{
	_machine_is_pathfinding_obstacle_bit = 0,
	_machine_is_not_pathfinding_obstacle_when_open_bit,
	_machine_is_elevator_bit,
	NUMBER_OF_MACHINE_FLAGS,
};

enum
{
	CONTROL_DEFINITION_TAG = 'ctrl',
	CONTROL_DEFINITION_VERSION = 1,
};

enum
{
	_control_toggle_switch = 0,
	_control_on_button,
	_control_off_button,
	_control_call_button,
	NUMBER_OF_CONTROL_TYPES,
};

enum
{
	_control_trigger_player = 0,
	_control_trigger_destruction,
	NUMBER_OF_CONTROL_TRIGGERS,
};

enum
{
	LIGHT_FIXTURE_DEFINITION_TAG = 'lifi',
	LIGHT_FIXTURE_DEFINITION_VERSION = 1,
};

/* ---------- macros */

#define device_definition_get(index) ((struct device_definition *)tag_get(DEVICE_DEFINITION_TAG, index))
#define machine_definition_get(index) ((struct machine_definition *)tag_get(MACHINE_DEFINITION_TAG, index))
#define control_definition_get(index) ((struct control_definition *)tag_get(CONTROL_DEFINITION_TAG, index)) // [fake name?]
#define light_fixture_definition_get(index) ((struct light_fixture_definition *)tag_get(LIGHT_FIXTURE_DEFINITION_TAG, index)) // [fake name?]

/* ---------- structures */

struct _device_definition
{
	unsigned long flags;
	real power_transition_time;
	real power_acceleration_time;
	real powered_position_transition_time;
	real powered_position_acceleration_time;
	real depowered_position_transition_time;
	real depowered_position_acceleration_time;
	short function_modes[4];
	struct tag_reference positive_start_effect;
	struct tag_reference negative_start_effect;
	struct tag_reference positive_stop_effect;
	struct tag_reference negative_stop_effect;
	struct tag_reference depowered_effect;
	struct tag_reference repowered_effect;
	real delay_time;
	unsigned long delay_unused[2];
	struct tag_reference delay_effect;
	real automatic_activation_radius;
	unsigned long unused[21];
	real runtime_maximum_power_acceleration;
	real runtime_maximum_power_velocity;
	real runtime_maximum_depowered_position_acceleration;
	real runtime_maximum_depowered_position_velocity;
	real runtime_maximum_powered_position_acceleration;
	real runtime_maximum_powered_position_velocity;
	real runtime_delay_ticks;
};

struct device_definition
{
	struct _object_definition object;
	struct _device_definition device;
};

struct _machine_definition
{
	short type;
	word flags;
	real door_open_time;
	unsigned long unused1[20];
	short collision_response;
	short elevator_node_index;
	unsigned long unused2[13];
	long runtime_door_open_ticks;
};

struct machine_definition
{
	struct _object_definition object;
	struct _device_definition device;
	struct _machine_definition machine;
};

struct _control_definition
{
	short type;
	short trigger;
	real call_value;
	unsigned long unused[20];
	struct tag_reference on_effect;
	struct tag_reference off_effect;
	struct tag_reference deny_effect;
};

struct control_definition
{
	struct _object_definition object;
	struct _device_definition device;
	struct _control_definition control;
};

struct _light_fixture_definition
{
	unsigned long unused[16];
};

struct light_fixture_definition
{
	struct _object_definition object;
	struct _device_definition device;
	struct _light_fixture_definition light_fixture;
};

/* ---------- globals */

/* ---------- public code */

#endif // __DEVICE_DEFINITIONS_H
