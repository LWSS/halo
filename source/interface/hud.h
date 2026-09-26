/*
HUD.H

header included in hcex build.
*/

#ifndef __HUD_H
#define __HUD_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/HUD_SOUNDS.C */

void hud_play_sound(short local_player_index, long type_flags, struct tag_block *sounds, long *sound_handles, word *sound_flags);

/* ---------- globals */

/* ---------- public code */

#endif // __HUD_H
