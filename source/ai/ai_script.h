/*
AI_SCRIPT.H

header included in hcex build.
*/

#ifndef __AI_SCRIPT_H
#define __AI_SCRIPT_H
#pragma once

/* ---------- headers */

#include "encounters.h"

/* ---------- constants */

enum
{
	_ai_index_encounter = 0,
	_ai_index_platoon,
	_ai_index_squad,
};

/* ---------- macros */

#define AI_INDEX_GET_TYPE(ai_index) (((ai_index)&0xC0000000)>>30) /* fake name */
#define AI_INDEX_GET_ENCOUNTER_INDEX(ai_index) DATUM_INDEX_TO_ABSOLUTE_INDEX(ai_index) /* fake name */
#define AI_INDEX_GET_SUBINDEX(ai_index) (((unsigned long)(ai_index)&0x00FF0000)>>16) /* fake name */

#define AI_INDEX_NEW(type, encounter_index, subindex) (((subindex)&0xFF)<<16 | (type)<<30 | DATUM_INDEX_TO_ABSOLUTE_INDEX(encounter_index)) /* fake name */

/* ---------- structures */

struct ai_index_platoon_iterator
{
	long encounter_index;
	long platoon_index;
	long last_platoon_index;
};

struct ai_index_squad_iterator
{
	long encounter_index;
	long required_platoon_index;
	long squad_index;
	long next_squad_index;
	long last_squad_index;
};

struct ai_index_actor_iterator
{
	long encounter_index;
	long squad_index;
	long platoon_index;
	struct encounter_actor_iterator iterator;
};

/* ---------- prototypes/AI_SCRIPT.C */

void ai_script_initialize(void);
void ai_script_dispose(void);
void ai_script_initialize_for_new_map(void);
void ai_script_dispose_from_old_map(void);
boolean ai_index_from_string(struct scenario *scenario, char const *ai_string, long *ai_index_reference);
void ai_index_to_string(long ai_index, struct scenario *scenario, char *buffer, long bufsize);
void ai_index_platoon_iterator_new(long ai_index, struct ai_index_platoon_iterator *iterator);
struct platoon_datum *ai_index_platoon_iterator_next(struct ai_index_platoon_iterator *iterator);
void ai_index_squad_iterator_new(long ai_index, struct ai_index_squad_iterator *iterator);
struct squad_datum *ai_index_squad_iterator_next(struct ai_index_squad_iterator *iterator);
void ai_index_actor_iterator_new(long ai_index, struct ai_index_actor_iterator *iterator);
struct actor_datum *ai_index_actor_iterator_next(struct ai_index_actor_iterator *iterator);
long object_list_from_ai_reference(long ai_index);
void ai_scripting_attach_unit(long unit_index, long ai_index);
void ai_scripting_attach_units(long object_list_index, long ai_index);
void ai_scripting_detach_unit(long unit_index);
void ai_scripting_detach_units(long object_list_index);
void ai_scripting_place(long ai_index);
void ai_scripting_kill(long ai_index);
void ai_scripting_kill_silent(long ai_index);
void ai_scripting_erase(long ai_index);
void ai_scripting_erase_all(void);
void ai_scripting_deselect(void);
void ai_scripting_select(long ai_index);
void ai_scripting_spawn_actor(long ai_index);
void ai_scripting_set_respawn(long ai_index, boolean respawn);
void ai_scripting_set_deaf(long ai_index, boolean deaf);
void ai_scripting_set_blind(long ai_index, boolean blind);
void ai_scripting_magically_see_encounter(long ai_index, long target_ai_index);
void ai_scripting_magically_see_players(long ai_index);
void ai_scripting_magically_see_unit(long ai_index, long unit_index);
void ai_scripting_magically_see_units(long ai_index, long object_list_index);
void ai_scripting_timer_start(long ai_index);
void ai_scripting_timer_expire(long ai_index);
long ai_scripting_swarm_count(long ai_index);
long ai_scripting_nonswarm_count(long ai_index);
long ai_scripting_living_count(long ai_index);
real ai_scripting_living_fraction(long ai_index);
real ai_scripting_strength(long ai_index);
boolean ai_scripting_is_attacking(long ai_index);
void ai_scripting_attack(long ai_index);
void ai_scripting_defend(long ai_index);
void ai_scripting_retreat(long ai_index);
void ai_scripting_maneuver(long ai_index);
void ai_scripting_maneuver_enable(long ai_index, boolean enabled);
void ai_scripting_migrate(long source_index, long target_index);
void ai_scripting_migrate_by_unit(long object_list_index, long ai_index);
void ai_scripting_migrate_and_speak(long source_index, long target_index, char const *speech_type);
void ai_scripting_allegiance(short team1_index, short team2_index);
void ai_scripting_allegiance_remove(short team1_index, short team2_index);
boolean ai_scripting_allegiance_broken(short team1_index, short team2_index);
void ai_scripting_go_to_vehicle(long ai_index, long unit_index, char const *seat_substring_name);
void ai_scripting_go_to_vehicle_override(long ai_index, long unit_index, char const *seat_substring_name);
short ai_scripting_going_to_vehicle(long unit_index);
void ai_scripting_exit_vehicle(long ai_index);
void ai_scripting_braindead(long ai_index, boolean braindead);
void ai_scripting_braindead_by_unit(long object_list_index, boolean braindead);
void ai_scripting_ignore(long object_list_index, boolean ignore);
void ai_scripting_prefer_target(long object_list_index, boolean prefer);
void ai_scripting_teleport_starting_location_if_unsupported(long ai_index);
void ai_scripting_teleport_starting_location(long ai_index);
void ai_scripting_renew(long ai_index);
void ai_scripting_try_to_fight_nothing(long ai_index);
void ai_scripting_try_to_fight(long ai_index, long target_ai_index);
void ai_scripting_try_to_fight_player(long ai_index);
void ai_scripting_allow_charge(long ai_index, boolean allow_charge);
void ai_scripting_command_list(long ai_index, short command_list_index);
void ai_scripting_command_list_by_unit(long unit_index, short command_list_index);
void ai_scripting_command_list_advance(long ai_index);
void ai_scripting_command_list_advance_by_unit(long unit_index);
short ai_scripting_command_list_status(long object_list_index);
void ai_scripting_free(long ai_index);
void ai_scripting_free_units(long object_list_index);
void ai_scripting_attach_free(long unit_index, long actor_variant_tag_index);
void ai_scripting_force_active(long ai_index, boolean active);
void ai_scripting_force_active_by_unit(long unit_index, boolean active);
void ai_scripting_set_return_state(long ai_index, short default_state);
void ai_scripting_set_current_state(long ai_index, short current_state);
short ai_scripting_status(long ai_index);
void ai_scripting_reconnect(void);
void ai_scripting_playfight(long ai_index, boolean playfight);
void ai_scripting_vehicle_encounter(long unit_index, long ai_index);
struct ai_vehicle_enterable *ai_scripting_find_vehicle_enterable(long vehicle_index);
void ai_scripting_vehicle_enterable_distance(long unit_index, real distance);
void ai_scripting_vehicle_enterable_team(long unit_index, short team_index);
void ai_scripting_vehicle_enterable_actor_type(long unit_index, short actor_type);
void ai_scripting_vehicle_enterable_actors(long unit_index, long ai_index);
void ai_scripting_vehicle_enterable_disable(long unit_index);
void ai_scripting_look_at_object(long unit_index, long object_index);
void ai_scripting_stop_looking(long unit_index);
void ai_scripting_automatic_migration_target(long ai_index, boolean enable);
void ai_scripting_follow_target_disable(long ai_index);
void ai_scripting_follow_target_players(long ai_index);
void ai_scripting_follow_target_unit(long ai_index, long unit_index);
void ai_scripting_follow_target_ai(long ai_index, long target_ai_index);
void ai_scripting_follow_distance(long ai_index, real follow_target_distance);
boolean ai_scripting_conversation(short conversation_index);
void ai_scripting_conversation_stop(short conversation_index);
void ai_scripting_conversation_advance(short conversation_index);
short ai_scripting_conversation_line(short conversation_index);
short ai_scripting_conversation_status(short conversation_index);
void ai_scripting_link_activation(long target_ai_index, long link_ai_index);
void ai_scripting_berserk(long ai_index, boolean berserk);
void ai_scripting_set_team(long ai_index, short team_index);
void ai_scripting_allow_dormant(long ai_index, boolean allow_dormant);

/* ---------- globals */

/* ---------- public code */

#endif // __AI_SCRIPT_H
