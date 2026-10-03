/*
PLAYER_PROFILE.H

header included in hcex build.
*/

#ifndef __PLAYER_PROFILE_H
#define __PLAYER_PROFILE_H
#pragma once

/* ---------- headers */

#include "input_abstraction.h"
#include "input_windows.h"
#include "event_manager.h"

/* ---------- constants */

enum
{
	_joystick_preset_standard = 0,
	_joystick_preset_south_paw,
	_joystick_preset_legacy,
	_joystick_preset_legacy_south_paw,
	NUMBER_OF_JOYSTICK_PRESETS,
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/PLAYER_PROFILE.C */

void player_profile_save_last_level_played(short local_player_index);
void player_profile_save_level_completed(short local_player_index);

/* ---------- globals */

/* ---------- public code */

#endif // __PLAYER_PROFILE_H
