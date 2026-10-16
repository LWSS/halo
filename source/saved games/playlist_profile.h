/*
PLAYLIST_PROFILE.H

header included in hcex build.
*/

#ifndef __PLAYLIST_PROFILE_H
#define __PLAYLIST_PROFILE_H
#pragma once

/* ---------- headers */

#include "game_engine.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/PLAYLIST_PROFILE.C */

void playlist_profiles_initialize(void);
void playlist_profiles_dispose(void);
void playlist_profiles_enumerate_available_to_local_player_index(short local_player_index, word *number_of_profiles, long *playlist_profile_indices);
long playlist_profile_new(short local_player_index, wchar_t *name);
void playlist_profile_delete(long playlist_profile_index);
boolean playlist_profile_get(long playlist_profile_index, struct game_variant *variant);
boolean playlist_profile_get_from_path(char const *full_path, struct game_variant *variant);
boolean playlist_profile_get_display_name(long playlist_profile_index, wchar_t *display_name);
void playlist_profile_save(long playlist_profile_index, struct game_variant *variant);
word playlist_profile_number_of_default_profiles_on_disk(void);

/* ---------- globals */

/* ---------- public code */

#endif // __PLAYLIST_PROFILE_H
