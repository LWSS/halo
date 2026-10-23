/*
WIDGET_TYPES.H

file has inline function assertions.
*/

#ifndef __WIDGET_TYPES_H
#define __WIDGET_TYPES_H
#pragma once

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct widget_type_definition
{
	unsigned long group_tag;
	boolean needs_lighting;
	void (*initialize)(void);
	void (*initialize_for_new_map)(void);
	void (*dispose_from_old_map)(void);
	void (*dispose)(void);
	long (*new_proc)(long definition_index);
	void (*delete_proc)(long type_datum_index);
	void (*update)(real dt);
	void (*render_proc)(long object_index, long type_datum_index, struct render_lighting const *lighting, struct render_animation const *animation);
};

/* ---------- globals */

static struct widget_type_definition widget_type_definitions[NUMBER_OF_WIDGET_TYPES] =
{
	{
		FLAG_DEFINITION_TAG,
		TRUE,
		flags_initialize,
		flags_initialize_for_new_map,
		flags_dispose_from_old_map,
		flags_dispose,
		flag_new,
		flag_delete,
		flags_update,
		flag_render
	},
	{
		ANTENNA_DEFINITION_TAG,
		FALSE,
		antennas_initialize,
		antennas_initialize_for_new_map,
		antennas_dispose_from_old_map,
		antennas_dispose,
		antenna_new,
		antenna_delete,
		antennas_update,
		antenna_render
	},
	{
		GLOW_DEFINITION_TAG,
		FALSE,
		glow_initialize,
		glow_initialize_for_new_map,
		glow_dispose_from_old_map,
		glow_dispose,
		glow_new,
		glow_delete,
		NULL,
		glow_submit
	},
	{
		LIGHT_VOLUME_DEFINITION_TAG,
		FALSE,
		light_volumes_initialize,
		light_volumes_initialize_for_new_map,
		light_volumes_dispose_from_old_map,
		light_volumes_dispose,
		light_volume_new,
		light_volume_delete,
		NULL,
		light_volume_submit
	},
	{
		LIGHTNING_DEFINITION_TAG,
		FALSE,
		lightnings_initialize,
		lightnings_initialize_for_new_map,
		lightnings_dispose_from_old_map,
		lightnings_dispose,
		lightning_new,
		lightning_delete,
		NULL,
		lightning_submit
	}
};

/* ---------- public code */

short tag_group_to_widget_type(
	tag group_tag)
{
	short result = NONE;
	short type;

	for (type = 0; type < NUMBER_OF_WIDGET_TYPES; type++)
	{
		if (widget_type_definitions[type].group_tag == group_tag)
		{
			result = type;
			break;
		}
	}

	return result;
}

static struct widget_type_definition *widget_type_definition_get(
	short type)
{
	match_assert("c:\\halo\\source\\objects\\widgets\\widget_types.h", 150, type>=0 && type<NUMBER_OF_WIDGET_TYPES);

	return &widget_type_definitions[type];
}

#endif // __WIDGET_TYPES_H
