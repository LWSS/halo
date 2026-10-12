/*
WEAPON_DEFINITIONS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "weapon_definitions.h"
#include "weapon_interface_definitions.h"
#include "equipment_definitions.h"
#include "projectile_definitions.h"
#include "objects.h"
#include "ai.h"
#include "unit_definitions.h"
#include "actor_definitions.h"
#include "text_group.h"
#include "meter_definitions.h"
#include "sound_definitions.h"
#include "hud_definitions.h"
#include "effect_definitions.h"
#include "damage_effect_definitions.h"
#include "predicted_resources.h"
#include "weapon_hud_interface_definition.h"

/* ---------- public code */

real weapon_definition_get_damage_potential(
	long weapon_definition_index,
	real *rounds_per_second)
{
	struct weapon_definition const *weapon_definition = weapon_definition_get(weapon_definition_index);
	struct weapon_trigger_definition const *trigger_definition = TAG_BLOCK_GET_ELEMENT(
		&weapon_definition->weapon.triggers,
		_weapon_trigger_primary,
		struct weapon_trigger_definition);
	real damage_potential = 0.f;

	if (rounds_per_second)
	{
		*rounds_per_second = trigger_definition->final_rate_of_fire;
	}

	if (trigger_definition->projectile.index != NONE)
	{
		struct projectile_definition const *projectile_definition = projectile_definition_get(trigger_definition->projectile.index);

		if (projectile_definition->projectile.impact_damage.index != NONE)
		{
			struct damage_effect_definition const *damage_effect_definition = damage_effect_definition_get(projectile_definition->projectile.impact_damage.index);

			damage_potential += (damage_effect_definition->damage.damage_lower_bound + damage_effect_definition->damage.damage_upper_bound) / 2.f;
		}

		if (projectile_definition->projectile.detonation_damage.index != NONE)
		{
			struct damage_effect_definition const *damage_effect_definition = damage_effect_definition_get(projectile_definition->projectile.detonation_damage.index);

			damage_potential += (damage_effect_definition->damage.damage_lower_bound + damage_effect_definition->damage.damage_upper_bound) / 2.f;
		}
	}

	return damage_potential;
}
