/*
AI_PROFILE.C
*/

/* ---------- headers */

#include "cseries.h"
#include "ai_profile.h"
#include "actions.h"
#include "actors.h"
#include "encounters.h"
#include "props.h"
#include "console.h"
#include "draw_string.h"
#include "interface.h"
#include "observer.h"
#include "rasterizer.h"
#include "render_debug.h"
#include "units.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct ai_meter_definition
{
	short meter_id;
	short (*value_callback)(void);
};

/* ---------- prototypes */

static short ai_meter_encounter(void);
static short ai_meter_actor(void);
static short ai_meter_unit(void);
static short ai_meter_prop(void);
static short ai_meter_swarm_cache(void);
static short ai_meter_swarm_actor(void);
static short ai_meter_swarm_component(void);

/* ---------- globals */

char profilestring[AI_PROFILE_STRING_LENGTH];
short global_ai_profile_string_position;
struct ai_profile_state ai_profile;

struct ai_meter_definition global_ai_meter_definitions[NUMBER_OF_AI_METERS] =
{
	{ _ai_meter_encounter, ai_meter_encounter },
	{ _ai_meter_encounter_active, NULL },
	{ _ai_meter_actor, ai_meter_actor },
	{ _ai_meter_actor_active, NULL },
	{ _ai_meter_actor_non_dormant, NULL },
	{ _ai_meter_unit, ai_meter_unit },
	{ _ai_meter_unit_active, NULL },
	{ _ai_meter_unit_non_dormant, NULL },
	{ _ai_meter_prop, ai_meter_prop },
	{ _ai_meter_prop_acknowledged_body, NULL },
	{ _ai_meter_prop_orphaned_body, NULL },
	{ _ai_meter_prop_unacknowledged_body, NULL },
	{ _ai_meter_prop_acknowledged_enemy, NULL },
	{ _ai_meter_prop_orphaned_enemy, NULL },
	{ _ai_meter_prop_unacknowledged_enemy, NULL },
	{ _ai_meter_prop_acknowledged_friend, NULL },
	{ _ai_meter_prop_orphaned_friend, NULL },
	{ _ai_meter_prop_unacknowledged_friend, NULL },
	{ _ai_meter_swarm, ai_meter_swarm_actor },
	{ _ai_meter_swarm_actor, ai_meter_swarm_cache },
	{ _ai_meter_swarm_component, ai_meter_swarm_component },
	{ _ai_meter_collision_vector, NULL },
	{ _ai_meter_line_of_sight, NULL },
	{ _ai_meter_line_of_fire, NULL },
	{ _ai_meter_path_flood, NULL },
	{ _ai_meter_path_find, NULL },
	{ _ai_meter_action_change, NULL },
	{ _ai_meter_firing_point_eval, NULL }
};

/* ---------- public code */

void ai_profile_initialize(
	void)
{
	memset(&ai_profile, 0, sizeof(ai_profile));
	ai_profile.show = TRUE;

	return;
}

void ai_profile_dispose(
	void)
{
	return;
}

void ai_profile_initialize_for_new_map(
	void)
{
	memset(ai_profile.meters, 0, sizeof(ai_profile.meters));

	return;
}

void ai_profile_dispose_from_old_map(
	void)
{
	return;
}

void ai_profile_update(
	void)
{
	short index;

	for (index = 0; index < NUMBER_OF_AI_METERS; index++)
	{
		struct ai_meter_definition *definition = &global_ai_meter_definitions[index];
		struct ai_meter *meter = &ai_profile.meters[index];

		match_assert("c:\\halo\\SOURCE\\ai\\ai_profile.c", 140, definition->meter_id == index);

		if (definition->value_callback)
		{
			meter->current_count = definition->value_callback();
		}

		meter->last_count = meter->current_count;
		meter->current_count = 0;

		match_assert("c:\\halo\\SOURCE\\ai\\ai_profile.c", 151, (meter->history_next_index >= 0) && (meter->history_next_index < AI_METER_HISTORY_TICKS));

		if (meter->history_next_index < meter->history_max_index)
		{
			meter->average_total -= meter->history_count[meter->history_next_index];
		}

		meter->history_count[meter->history_next_index++] = meter->last_count;
		meter->history_max_index = MAX(meter->history_max_index, meter->history_next_index);
		meter->history_next_index %= AI_METER_HISTORY_TICKS;
		meter->average_count = (real)meter->average_total / (real)meter->history_max_index;
	}

	return;
}

void ai_profile_display(
	char *string)
{
	sprintf(
		&string[strlen(string)],
		"ai enc % 2d/% 3d, actor % 3d/% 3d/% 3d, unit % 3d/% 3d/% 3d, props % 3d/% 3d|n",
		ai_profile.meters[_ai_meter_encounter_active].last_count,
		ai_profile.meters[_ai_meter_encounter].last_count,
		ai_profile.meters[_ai_meter_actor_non_dormant].last_count,
		ai_profile.meters[_ai_meter_actor_active].last_count,
		ai_profile.meters[_ai_meter_actor].last_count,
		ai_profile.meters[_ai_meter_unit_non_dormant].last_count,
		ai_profile.meters[_ai_meter_unit_active].last_count,
		ai_profile.meters[_ai_meter_unit].last_count,
		ai_profile.meters[_ai_meter_prop].last_count,
		MAXIMUM_NUMBER_OF_PROPS);

	return;
}

static void ai_profile_string(
	char const *string,
	short tab_stop_count,
	short const *tab_stops,
	real_argb_color const *color)
{
	rectangle2d bounds;
	point2d cursor;

	bounds.x0 = 0;
	bounds.y0 = global_ai_profile_string_position;
	bounds.x1 = SHRT_MAX;
	bounds.y1 = SHRT_MAX;

	if (!color)
	{
		color = global_real_argb_white;
	}

	interface_set_bitmap_text_draw_mode(
		_interface_font_terminal,
		NONE,
		0,
		0,
		_interface_color_table_dialog,
		0);
	draw_string_set_color(color);
	draw_string_set_tab_stops(tab_stops, tab_stop_count);
	rasterizer_draw_string(&bounds, NULL, &cursor, 0, string);
	draw_string_set_tab_stops(NULL, 0);
	global_ai_profile_string_position -= cursor.y - bounds.y0;

	return;
}

short ai_profile_change_render_spray(
	void)
{
	static char const *spray_mode_names[] =
	{
		"none",
		"actions",
		"activation status"
	};

	match_assert("c:\\halo\\SOURCE\\ai\\ai_profile.c", 246, NUMBEROF(spray_mode_names) == NUMBER_OF_AI_PROFILE_RENDER_SPRAY_MODES);

	ai_profile.render_spray_mode = (ai_profile.render_spray_mode + 1) % NUMBER_OF_AI_PROFILE_RENDER_SPRAY_MODES;
	console_printf(
		FALSE,
		"AI line-spray: %s",
		spray_mode_names[ai_profile.render_spray_mode]);

	return ai_profile.render_spray_mode;
}

/* ---------- private code */

static short ai_meter_encounter(
	void)
{
	return encounter_data->actual_count;
}

static short ai_meter_actor(
	void)
{
	return actor_data->actual_count;
}

static short ai_meter_unit(
	void)
{
	struct actor_iterator iterator;
	struct actor_datum *actor;
	short count = 0;

	actor_iterator_new(&iterator, FALSE);

	while (actor = actor_iterator_next(&iterator))
	{
		count += actor->meta.swarm ? actor->meta.swarm_unit_count : 1;
	}

	return count;
}

static short ai_meter_prop(
	void)
{
	return prop_data->actual_count;
}

static short ai_meter_swarm_cache(
	void)
{
	return swarm_data->actual_count;
}

static short ai_meter_swarm_actor(
	void)
{
	struct actor_iterator iterator;
	struct actor_datum *actor;
	short count = 0;

	actor_iterator_new(&iterator, FALSE);

	while (actor = actor_iterator_next(&iterator))
	{
		count += actor->meta.swarm;
	}

	return count;
}

static short ai_meter_swarm_component(
	void)
{
	return swarm_component_data->actual_count;
}

static void ai_profile_show_stats(
	void)
{
	short tabs[] = { 150, 300 };

	sprintf(
		profilestring,
		"encounters %d/%d|tprops %d/%d",
		ai_profile.meters[_ai_meter_encounter_active].last_count,
		ai_profile.meters[_ai_meter_encounter].last_count,
		ai_profile.meters[_ai_meter_prop].last_count,
		MAXIMUM_NUMBER_OF_PROPS);
	ai_profile_string(profilestring, NUMBEROF(tabs), tabs, global_real_argb_white);

	return;
}

static void ai_profile_show_actors(
	void)
{
	short tabs[] = { 150, 300 };

	sprintf(
		profilestring,
		"actors %d/%d/%d|units %d/%d%d",
		ai_profile.meters[_ai_meter_actor_non_dormant].last_count,
		ai_profile.meters[_ai_meter_actor_active].last_count,
		ai_profile.meters[_ai_meter_actor].last_count,
		ai_profile.meters[_ai_meter_unit_non_dormant].last_count,
		ai_profile.meters[_ai_meter_unit_active].last_count,
		ai_profile.meters[_ai_meter_unit].last_count);
	ai_profile_string(profilestring, NUMBEROF(tabs), tabs, global_real_argb_white);

	return;
}

static void ai_profile_show_swarms(
	void)
{
	short tabs[] = { 150 };

	sprintf(
		profilestring,
		"swarms %d/%d/%d|tcomponents %d/%d",
		ai_profile.meters[_ai_meter_swarm_actor].last_count,
		ai_profile.meters[_ai_meter_swarm].last_count,
		MAXIMUM_NUMBER_OF_ACTIVE_SWARMS,
		ai_profile.meters[_ai_meter_swarm_component].last_count,
		MAXIMUM_NUMBER_OF_SWARM_COMPONENTS);
	ai_profile_string(profilestring, NUMBEROF(tabs), tabs, global_real_argb_white);

	return;
}

static void ai_profile_show_prop_types(
	void)
{
	short tabs[] = { 150, 300, 450 };

	sprintf(
		profilestring,
		"props a/o/u:|tenemy %d/%d/%d|tfriend %d/%d/%d|tdead %d/%d/%d",
		ai_profile.meters[_ai_meter_prop_acknowledged_enemy].last_count,
		ai_profile.meters[_ai_meter_prop_orphaned_enemy].last_count,
		ai_profile.meters[_ai_meter_prop_unacknowledged_enemy].last_count,
		ai_profile.meters[_ai_meter_prop_acknowledged_friend].last_count,
		ai_profile.meters[_ai_meter_prop_orphaned_friend].last_count,
		ai_profile.meters[_ai_meter_prop_unacknowledged_friend].last_count,
		ai_profile.meters[_ai_meter_prop_acknowledged_body].last_count,
		ai_profile.meters[_ai_meter_prop_orphaned_body].last_count,
		ai_profile.meters[_ai_meter_prop_unacknowledged_body].last_count);
	ai_profile_string(profilestring, NUMBEROF(tabs), tabs, global_real_argb_white);

	return;
}

static void ai_profile_show_line_of_sight(
	void)
{
	short tabs[] = { 150, 300, 450 };

	sprintf(
		profilestring,
		"collisions %d|tlineofsight %d|tlineoffire %d|tfiringpoint %d",
		ai_profile.meters[_ai_meter_collision_vector].last_count,
		ai_profile.meters[_ai_meter_line_of_sight].last_count,
		ai_profile.meters[_ai_meter_line_of_fire].last_count,
		ai_profile.meters[_ai_meter_firing_point_eval].last_count);
	ai_profile_string(profilestring, NUMBEROF(tabs), tabs, global_real_argb_white);

	return;
}

static void ai_profile_show_paths(
	void)
{
	short tabs[] = { 150, 300, 450 };

	sprintf(
		profilestring,
		"path_flood %d|tpath_find %d|taction_change %d",
		ai_profile.meters[_ai_meter_path_flood].last_count,
		ai_profile.meters[_ai_meter_path_find].last_count,
		ai_profile.meters[_ai_meter_action_change].last_count);
	ai_profile_string(profilestring, NUMBEROF(tabs), tabs, global_real_argb_white);

	return;
}

static void ai_profile_render_spray(
	void)
{
	struct observer_result const *camera = observer_get_camera(0);

	if (ai_profile.render_spray_mode > _ai_profile_render_spray_none && camera)
	{
		struct actor_iterator iterator;
		real_point3d from_point;
		boolean active_only = TRUE;

		switch (ai_profile.render_spray_mode)
		{
		case _ai_profile_render_spray_activation:
			active_only = FALSE;
			break;
		}

		point_from_line3d(&camera->position, &camera->forward, 0.05f, &from_point);
		actor_iterator_new(&iterator, active_only);

		while (actor_iterator_next(&iterator))
		{
			struct actor_datum *actor = actor_get(iterator.index);
			real_argb_color const *color = NULL;

			switch (ai_profile.render_spray_mode)
			{
			case _ai_profile_render_spray_actions:
				color = actor_action_debug_color(iterator.index);
				break;

			case _ai_profile_render_spray_activation:
				color = actor_activation_debug_color(iterator.index);
				break;
			}

			if (color)
			{
				if (actor->meta.swarm)
				{
					long unit_index = actor->meta.swarm_unit_index;

					while (unit_index != NONE)
					{
						real_point3d head_position;
						struct unit_datum *unit = unit_get(unit_index);

						unit_get_head_position(unit_index, &head_position);
						render_debug_line(TRUE, &from_point, &head_position, color);
						unit_index = unit->unit.swarm_next_unit_index;
					}
				}
				else
				{
					render_debug_line(TRUE, &from_point, &actor->input.position.head_position, color);
				}
			}
		}
	}

	return;
}

void ai_profile_render(
	void)
{
	global_ai_profile_string_position = rasterizer_globals.frame_bounds.y1 - 20;
	ai_profile_render_spray();

	if (ai_profile.show)
	{
		if (ai_profile.show_prop_types)
		{
			ai_profile_show_prop_types();
		}

		if (ai_profile.show_line_of_sight)
		{
			ai_profile_show_line_of_sight();
		}

		if (ai_profile.show_paths)
		{
			ai_profile_show_paths();
		}

		if (ai_profile.show_swarms)
		{
			ai_profile_show_swarms();
		}

		if (ai_profile.show_actors)
		{
			ai_profile_show_actors();
		}

		if (ai_profile.show_stats)
		{
			ai_profile_show_stats();
		}
	}

	return;
}
