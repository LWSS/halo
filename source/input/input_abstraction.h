/*
INPUT_ABSTRACTION.H

header included in hcex build.
*/

#ifndef __INPUT_ABSTRACTION_H
#define __INPUT_ABSTRACTION_H
#pragma once

/* ---------- constants */

enum
{
	_button_jump = 0,
	_button_switch_grenade,
	_button_action_reload,
	_button_switch_weapon,
	_button_melee_attack,
	_button_flashlight,
	_button_throw_grenade,
	_button_fire,
	_button_start,
	_button_back,
	_button_crouch,
	_button_scope_zoom,
	NUMBER_OF_ACTION_CONTROL_BUTTONS,
};

/* ---------- macros */

/* ---------- structures */

struct game_input_preferences
{
	real yaw_rate;
	real pitch_rate;
	byte game_control_to_xbox_buttons[NUMBER_OF_ACTION_CONTROL_BUTTONS];
	short joystick_controls;
	boolean invert_look;
	boolean invert_look_aircraft_control;
};

struct game_input_state
{
	byte buttons[NUMBER_OF_ACTION_CONTROL_BUTTONS];
	real forward_movement;
	real strafe;
	real yaw;
	real pitch;
};

/* ---------- prototypes/INPUT_ABSTRACTION.C */

void input_abstraction_dispose(void);
void input_abstraction_reset_controller_detection_timer(void);
void input_abstraction_get_local_player_preferences(short local_player_index, struct game_input_preferences *preferences);
void input_abstraction_update_local_player_preferences(short controller_index, struct game_input_preferences *preferences);
struct game_input_state *input_abstraction_get_input_state(short local_player_index);
void input_abstraction_update_device_changes(unsigned long device_change_flags);
void input_abstraction_initialize(void);
void input_abstraction_update(void);

/* ---------- globals */

/* ---------- public code */

#endif // __INPUT_ABSTRACTION_H
