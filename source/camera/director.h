/*
DIRECTOR.H

header included in hcex build.
*/

#ifndef __DIRECTOR_H
#define __DIRECTOR_H
#pragma once

/* ---------- constants */

enum
{
	_director_perspective_first_person = 0,
	_director_perspective_third_person,
	_director_perspective_scripted,
	_director_perspective_neutral,
	NUMBER_OF_DIRECTOR_PERSPECTIVE_MODES,
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/DIRECTOR.C */

void director_update(real dt);
short director_get_perspective(short local_player_index);
void director_script_camera(boolean enabled);

void director_initialize_for_saved_game(void);
short director_get_perspective(short local_player_index);

/* ---------- globals */

/* ---------- public code */

#endif // __DIRECTOR_H
