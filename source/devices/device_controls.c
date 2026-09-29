/*
DEVICE_CONTROLS.C

*/

/* ---------- headers */

#include "cseries.h"
#include "devices.h"
#include "device_definitions.h"
#include "units.h"
#include "effects.h"
#include "object_lights.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static void control_toggle(long control_index);

/* ---------- globals */

/* ---------- public code */

void controls_initialize(
	void)
{
	return;
}

void controls_dispose(
	void)
{
	return;
}

void controls_initialize_for_new_map(
	void)
{
	return;
}

void controls_dispose_from_old_map(
	void)
{
	return;
}

void control_place(
	long control_index,
	struct scenario_control_datum *scenario_control)
{
	struct control_datum *control = control_get(control_index);
	struct control_definition *definition = control_definition_get(control->definition_index);

	device_add_scenario_information(control_index, &scenario_control->device);

	if (TEST_FLAG(scenario_control->flags, _scenario_control_usable_from_both_sides_bit))
	{
		SET_FLAG(control->control.flags, _control_usable_from_both_sides_bit, TRUE);
	}

	if (TEST_FLAG(scenario_control->flags, _scenario_device_not_usable_bit))
	{
		SET_FLAG(control->control.flags, _device_not_usable_bit, TRUE);
	}

	control->control.hud_override_index = scenario_control->hud_override_string_list_index - 1;

	return;
}

boolean control_new(
	long control_index)
{
	struct control_datum *control = control_get(control_index);
	struct control_definition *definition = control_definition_get(control->definition_index);

	return TRUE;
}

void control_delete(
	long control_index)
{
	return;
}

boolean control_update(
	long control_index)
{
	struct control_datum *control = control_get(control_index);
	struct control_definition *definition = control_definition_get(control->definition_index);

	return TRUE;
}

void control_touched(
	long control_index,
	long unit_index)
{
	struct control_datum *control = control_get(control_index);
	struct control_definition *definition = control_definition_get(control->definition_index);

	if (definition->control.trigger == _control_trigger_player)
	{
		control_toggle(control_index);
	}

	return;
}

void control_destroyed(
	long control_index)
{
	struct control_datum *control = control_get(control_index);
	struct control_definition *definition = control_definition_get(control->definition_index);

	if (definition->control.trigger == _control_trigger_destruction)
	{
		control_toggle(control_index);
	}

	return;
}

/* ---------- private code */

static void control_toggle(
	long control_index)
{
	struct control_datum *control = control_get(control_index);
	struct control_definition *definition = control_definition_get(control->definition_index);

	if (control->device.position_group_index != NONE)
	{
		struct device_group_datum *device_group = device_group_get(control->device.position_group_index);
		real desired_value;

		switch (definition->control.type)
		{
		case _control_toggle_switch:
			desired_value = device_group->desired_value > 0.5f ? 0.f : 1.f;
			break;
		case _control_on_button:
			desired_value = 1.f;
			break;
		case _control_off_button:
			desired_value = 0.f;
			break;
		case _control_call_button:
			desired_value = definition->control.call_value;
			break;
		default:
			match_vassert("c:\\halo\\SOURCE\\devices\\device_controls.c", 138, FALSE, NULL);
		}

		if (device_group_set_desired_value(control->device.position_group_index, desired_value))
		{
			device_effect_new(control_index, desired_value > 0.5f ? definition->control.on_effect.index : definition->control.off_effect.index);
		}
		else
		{
			device_effect_new(control_index, definition->control.deny_effect.index);
		}
	}

	return;
}
