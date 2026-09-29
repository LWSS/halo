/*
DEVICE_LIGHT_FIXTURES.C

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

/* ---------- globals */

/* ---------- public code */

void light_fixtures_initialize(
	void)
{
	return;
}

void light_fixtures_dispose(
	void)
{
	return;
}

void light_fixtures_initialize_for_new_map(
	void)
{
	return;
}

void light_fixtures_dispose_from_old_map(
	void)
{
	return;
}

void light_fixture_place(
	long light_fixture_index,
	struct scenario_light_fixture_datum *scenario_light_fixture)
{
	struct light_fixture_datum *light_fixture = light_fixture_get(light_fixture_index);
	struct light_fixture_definition *definition = light_fixture_definition_get(light_fixture->definition_index);

	device_add_scenario_information(light_fixture_index, &scenario_light_fixture->device);

	light_fixture->light_fixture.color = scenario_light_fixture->color;
	light_fixture->light_fixture.intensity = scenario_light_fixture->intensity;
	light_fixture->light_fixture.falloff_angle = scenario_light_fixture->falloff_angle;
	light_fixture->light_fixture.cutoff_angle = scenario_light_fixture->cutoff_angle;

	return;
}

boolean light_fixture_new(
	long light_fixture_index)
{
	struct light_fixture_datum *light_fixture = light_fixture_get(light_fixture_index);
	struct light_fixture_definition *definition = light_fixture_definition_get(light_fixture->definition_index);

	return TRUE;
}

void light_fixture_delete(
	long light_fixture_index)
{
	return;
}

boolean light_fixture_update(
	long light_fixture_index)
{
	struct light_fixture_datum *light_fixture = light_fixture_get(light_fixture_index);
	struct light_fixture_definition *definition = light_fixture_definition_get(light_fixture->definition_index);

	return TRUE;
}

/* ---------- private code */
