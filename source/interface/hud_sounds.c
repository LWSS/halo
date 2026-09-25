/*
HUD_SOUNDS.C
*/
/* ---------- headers */

#include "cseries.h"
#include "hud.h"
#include "hud_definitions.h"
#include "objects.h"
#include "game_sound.h"
#include "sound_definitions.h"
#include "sound_manager.h"

/* ---------- public code */
void hud_play_sound(
	short local_player_index,
	long type_flags,
	struct tag_block *sounds,
	long *sound_handles,
	word *sound_flags)
{
	short sound_index;
	for (sound_index = 0; sound_index < sounds->count; sound_index++)
	{
		struct sound_hud_element_definition *sound = TAG_BLOCK_GET_ELEMENT(sounds, sound_index, struct sound_hud_element_definition);
		if (sound->type_flags & type_flags)
		{
			switch (sound->sound.group_tag)
			{
			case SOUND_DEFINITION_TAG:
				if (sound_handles[sound_index] == NONE || !TEST_FLAG(*sound_flags, sound_index))
				{
					if (sound_handles[sound_index] != NONE)
					{
						sound_stop_impulse(sound_handles[sound_index]);
					}
					sound_handles[sound_index] = unspatialized_impulse_sound_new(sound->sound.index, sound->scale);
				}
				break;
			case LOOPING_SOUND_DEFINITION_TAG:
				if (sound_handles[sound_index] == NONE)
				{
					sound_handles[sound_index] = unattached_looping_sound_start(sound->sound.index, NONE, sound->scale);
				}
				break;
			default:
				match_assert("c:\\halo\\SOURCE\\interface\\hud_sounds.c", 47, !"unreachable");
				break;
			}
			SET_FLAG(*sound_flags, sound_index, TRUE);
		}
		else if (sound_handles[sound_index] != NONE)
		{
			switch (sound->sound.group_tag)
			{
			case SOUND_DEFINITION_TAG:
				break;

			case LOOPING_SOUND_DEFINITION_TAG:
				unattached_looping_sound_stop(sound_handles[sound_index]);
				break;

			default:
				match_assert("c:\\halo\\SOURCE\\interface\\hud_sounds.c", 64, !"unreachable");
				break;
			}
			sound_handles[sound_index] = NONE;
			SET_FLAG(*sound_flags, sound_index, FALSE);
		}
	}

	return;
}
