/*
AI_SCENARIO_DEFINITIONS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "ai_scenario_definitions.h"
#include "actors.h"
#include "encounters.h"
#include "actor_types.h"
#include "ai_script.h"
#include "collision_bsp.h"
#include "render.h"
#include "units.h"
#include "sound_definitions.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

char const *global_ai_default_state_names[NUMBER_OF_ACTOR_DEFAULT_STATES] =
{
	"none",
	"sleep",
	"alert",
	"move_repeat",
	"move_loop",
	"move_loop_back_and_forth",
	"move_loop_random",
	"move_random",
	"guard",
	"guard_at_position",
	"search",
	"flee"
};

/* ---------- public code */

short choose_random_array_element(
	void const *array,
	short element_size,
	short element_count,
	short weight_field_offset,
	unsigned long const *used_bit_vector)
{
	real const *weight;
	short element_index;
	real random_weight;
	short result = NONE;
	real total_weight = 0.f;

	for (element_index = 0, weight = (real const *)((byte const *)array + weight_field_offset);
		element_index < element_count;
		element_index++, weight = offset_pointer((void *)weight, element_size))
	{
		if (!BIT_VECTOR_TEST_FLAG(used_bit_vector, element_index))
		{
			total_weight += *weight;
		}
	}

	if (total_weight > 0.f)
	{
		random_weight = real_random_range(0.f, total_weight);
		total_weight = 0.f;

		for (element_index = 0, weight = (real const *)((byte const *)array + weight_field_offset);
			element_index < element_count;
			element_index++, weight = offset_pointer((void *)weight, element_size))
		{
			if (!BIT_VECTOR_TEST_FLAG(used_bit_vector, element_index))
			{
				total_weight += *weight;

				if (random_weight <= total_weight)
				{
					result = element_index;
					break;
				}
			}
		}
	}

	return result;
}

long scenario_get_encounter_by_name(
	struct scenario const *scenario,
	char const *encounter_name)
{
	long encounter_index;
	long result = NONE;

	for (encounter_index = 0; encounter_index < scenario->ai_encounters.count; encounter_index++)
	{
		struct encounter_definition const *encounter = TAG_BLOCK_GET_ELEMENT(
			&scenario->ai_encounters,
			encounter_index,
			struct encounter_definition);

		if (!_strnicmp(encounter->name, encounter_name, TAG_STRING_LENGTH + 1))
		{
			result = encounter_index;
			break;
		}
	}

	return result;
}

long encounter_definition_get_squad_by_name(
	struct encounter_definition const *encounter,
	char const *squad_name)
{
	long squad_index;
	long result = NONE;

	for (squad_index = 0; squad_index < encounter->squads.count; squad_index++)
	{
		struct squad_definition const *squad = TAG_BLOCK_GET_ELEMENT(
			&encounter->squads,
			squad_index,
			struct squad_definition);

		if (!_strnicmp(squad->name, squad_name, TAG_STRING_LENGTH + 1))
		{
			result = squad_index;
			break;
		}
	}

	return result;
}

long encounter_definition_get_platoon_by_name(
	struct encounter_definition const *encounter,
	char const *platoon_name)
{
	long platoon_index;
	long result = NONE;

	for (platoon_index = 0; platoon_index < encounter->platoons.count; platoon_index++)
	{
		struct platoon_definition const *platoon = TAG_BLOCK_GET_ELEMENT(
			&encounter->platoons,
			platoon_index,
			struct platoon_definition);

		if (!_strnicmp(platoon->name, platoon_name, TAG_STRING_LENGTH + 1))
		{
			result = platoon_index;
			break;
		}
	}

	return result;
}

/* ---------- private code */
