/*
PLAYER_EFFECTS.H

header included in hcex build.
*/

#ifndef __PLAYER_EFFECTS_H
#define __PLAYER_EFFECTS_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/PLAYER_EFFECTS.C */

void player_effect_get_damage_indicators(short local_player_index, byte *damage_indicators);
void player_effect_clear_damage_indicators(short local_player_index);

/* ---------- globals */

/* ---------- public code */

#endif // __PLAYER_EFFECTS_H
