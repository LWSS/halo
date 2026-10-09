/*
PROJECTILES.C
*/

/* ---------- headers */

#include "cseries.h"
#include "projectiles.h"
#include "item_definitions.h"
#include "items.h"
#include "projectile_definitions.h"
#include "network_game_globals.h"
#include "players.h"
#include "console.h"
#include "bungie_net/network/transport.h"
#include "collisions.h"
#include "sound_manager.h"
#include "render_debug.h"
#include "director.h"
#include "game_sound.h"
#include "sound_definitions.h"
#include "bipeds.h"
#include "physics_constants.h"
#include "effects.h"
#include "network_server_message_handler.h"
#include "object_types.h"
#include "physics.h"
#include "contrails.h"
#include "contrail_definitions.h"
#include "ai.h"
#include "ai_globals.h"
#include "breakable_surfaces.h"
#include "damage.h"
#include "game_engine.h"
#include "game_globals.h"
#include "periodic_functions.h"
#include "scenario.h"
#include "units.h"

/* ---------- constants */

enum
{
	MAXIMUM_PROJECTILE_COLLISIONS_PER_UPDATE = 10, /* fake name */
	SUPER_DETONATION_PROJECTILE_COUNT = 6, /* fake name */
};

enum
{
	_effect_vector_normal = 0,
	_effect_vector_incident,
	_effect_vector_negative_incident,
	_effect_vector_reflected,
	_effect_vector_gravity,
	NUMBER_OF_EFFECT_MARKERS,
};

/* ---------- prototypes */

static void projectile_set_action(long projectile_index, short action);
static void projectile_collision(long projectile_index, struct collision_result *collision, real_point3d *new_position, real_vector3d *new_velocity, real time_left);
static void projectile_effect_new(long projectile_index, long definition_index, struct collision_result const *collision, real_point3d const *points, real_vector3d const *vectors, real scale_a, real scale_b);
static void projectile_adjust_for_angular_velocity_change(long object_index);
static void projectile_calculate_deceleration(long projectile_index);
static real projectile_calculate_deceleration_from_distances(struct projectile_definition const *projectile_definition, real minimum_distance, real maximum_distance);
static boolean projectile_collision_test_line(long projectile_index, real_point3d const *new_position, struct collision_result *collision_result);
static void projectile_detonate(long projectile_index, boolean first_collision, real time_left);

/* ---------- globals */

static struct profile_section projectile_update_section =
{
	"projectile_update",
	NONE,
	TRUE
};

static char const *effect_marker_names[NUMBER_OF_EFFECT_MARKERS] =
{
	"normal",
	"incident",
	"negative incident",
	"reflection",
	"gravity"
};

static real const seconds_per_tick = SECONDS_PER_TICK;

/* ---------- public code */

void projectiles_initialize(
	void)
{
	return;
}

void projectiles_initialize_for_new_map(
	void)
{
	return;
}

void projectiles_dispose_from_old_map(
	void)
{
	return;
}

void projectiles_dispose(
	void)
{
	return;
}

void projectile_kill_tracer(
	long projectile_index)
{
	struct projectile_datum *projectile = projectile_get(projectile_index);

	SET_FLAG(projectile->projectile.flags, _projectile_tracer_bit, FALSE);

	return;
}

void projectiles_delete_all(
	void)
{
	struct object_iterator iterator;

	object_iterator_new(&iterator, _object_mask_projectile, 0);

	while (object_iterator_next(&iterator))
	{
		object_delete(iterator.index);
	}

	return;
}

boolean projectile_new(
	long projectile_index)
{
	real detonation_time;
	short attachment_index;
	struct projectile_datum *projectile = projectile_get(projectile_index);
	struct projectile_definition *definition = projectile_definition_get(projectile->definition_index);

	SET_FLAG(projectile->object.flags, _object_dynamic_lighting_recompute_bit, TRUE);
	projectile->projectile.flags = FLAG(_projectile_tracer_bit);
	projectile->projectile.target_object_index = NONE;
	projectile->projectile.action = _projectile_action_none;
	projectile->projectile.hit_material_type = NONE;
	projectile->projectile.ignore_object_index = object_get_ultimate_parent(projectile->object.owner_object_index);

	if (TEST_FLAG(definition->projectile.flags, _projectile_detonation_max_time_if_attached_bit))
	{
		detonation_time = definition->projectile.detonation_minimum_time;
	}
	else
	{
		detonation_time = real_random_range(
			definition->projectile.detonation_minimum_time,
			definition->projectile.detonation_maximum_time);
	}

	if (detonation_time * TICKS_PER_SECOND >= 1.f)
	{
		projectile->projectile.detonation_timer_delta = 1.f / (detonation_time * TICKS_PER_SECOND);
	}

	if (definition->projectile.arming_time * TICKS_PER_SECOND >= 1.f)
	{
		projectile->projectile.arming_time_delta = 1.f / (definition->projectile.arming_time * TICKS_PER_SECOND);
	}

	projectile->projectile.tracer_attachment_index_index = NONE;

	for (attachment_index = 0; attachment_index < definition->object.attachments.count; attachment_index++)
	{
		struct object_attachment_definition *attachment = TAG_BLOCK_GET_ELEMENT(
			&definition->object.attachments,
			attachment_index,
			struct object_attachment_definition);

		if (attachment->type.group_tag == CONTRAIL_DEFINITION_TAG)
		{
			projectile->projectile.tracer_attachment_index_index = attachment_index;
			break;
		}
	}

	vector_from_line3d(
		&projectile->object.translational_velocity,
		&projectile->object.forward,
		definition->projectile.initial_velocity,
		&projectile->object.translational_velocity);
	SET_FLAG(projectile->object.flags, _object_wholly_under_media_bit, scenario_location_underwater(&projectile->object.location, &projectile->object.bounding_sphere_center, NULL));
	projectile_adjust_for_angular_velocity_change(projectile_index);
	projectile_export_function_values(projectile_index);
	projectile_calculate_deceleration(projectile_index);
	SET_FLAG(projectile->object.flags, _object_shadowless_bit, TRUE);
	SET_FLAG(projectile->object.flags, _object_deleted_when_deactivated_bit, TRUE);

	return TRUE;
}

void projectile_delete(
	long projectile_index)
{
	return;
}

void projectile_set_target_object_index(
	long projectile_index,
	long target_object_index)
{
	struct projectile_datum *projectile = projectile_get(projectile_index);

	projectile->projectile.target_object_index = target_object_index;

	return;
}

void projectile_make_tracer(
	long projectile_index)
{
	struct projectile_datum *projectile = projectile_get(projectile_index);

	SET_FLAG(projectile->projectile.flags, _projectile_tracer_bit, TRUE);

	return;
}

boolean projectile_update(
	long projectile_index)
{
	boolean start_timer;
	struct projectile_datum *projectile = projectile_get(projectile_index);
	struct projectile_definition *definition = projectile_definition_get(projectile->definition_index);
	real time_left = 1.f;
	short collision_count = 0;
	boolean flyby_played = FALSE;

	profile_enter(projectile_update_section);

	if (!TEST_FLAG(projectile->projectile.flags, _projectile_tracer_bit) &&
		projectile->projectile.tracer_attachment_index_index != NONE)
	{
		long contrail_index = projectile->object.attachment_indices[projectile->projectile.tracer_attachment_index_index];

		if (contrail_index != NONE)
		{
			contrail_delete(contrail_index);
		}

		projectile->object.attachment_indices[projectile->projectile.tracer_attachment_index_index] = NONE;
		projectile->projectile.tracer_attachment_index_index = NONE;
	}

	projectile->projectile.arming_time += projectile->projectile.arming_time_delta;
	projectile->projectile.deceleration_timer += projectile->projectile.deceleration_timer_delta;

	switch (definition->projectile.detonation_timer_mode)
	{
	case _detonation_timer_once_bounced:
		start_timer = TEST_FLAG(projectile->projectile.flags, _projectile_collided_once_bit);
	case _detonation_timer_when_at_rest:
		start_timer = TEST_FLAG(projectile->projectile.flags, _projectile_stopped_after_collision_bit);
		break;
	default:
		start_timer = TRUE;
		break;
	}

	if (TEST_FLAG(projectile->projectile.flags, _projectile_counting_down_bit) ||
		TEST_FLAG(projectile->projectile.flags, _projectile_attached_bit))
	{
		start_timer = TRUE;
	}

	if (start_timer)
	{
		if (!TEST_FLAG(projectile->projectile.flags, _projectile_counting_down_bit))
		{
			SET_FLAG(projectile->projectile.flags, _projectile_counting_down_bit, TRUE);
		}

		if ((projectile->projectile.detonation_timer += projectile->projectile.detonation_timer_delta) >= 1.f)
		{
			projectile_set_action(projectile_index, _projectile_action_detonate);
		}
	}

	projectile_export_function_values(projectile_index);

	while (time_left > 0.f &&
		(projectile->projectile.action == _projectile_action_none ||
		(projectile->projectile.action == _projectile_action_detonate && projectile->projectile.arming_time_delta != 0.f && projectile->projectile.arming_time < 1.f)) &&
		!TEST_FLAG(projectile->projectile.flags, _projectile_attached_bit) &&
		!TEST_FLAG(projectile->object.flags, _object_at_rest_bit) &&
		projectile->object.parent_object_index == NONE)
	{
		struct collision_result collision;
		real_point3d new_position;
		real gravity;
		real travel_fraction;
		real speed = magnitude3d(&projectile->object.translational_velocity);
		real_vector3d new_velocity = projectile->object.translational_velocity;
		boolean moved = FALSE;
		real new_speed = speed;
		real average_speed = speed;
		real_vector3d average_velocity = projectile->object.translational_velocity;
		long ignore_object_index = projectile->projectile.ignore_object_index;

		match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\items\\projectiles.c", 326, &projectile->object.translational_velocity);

		if (projectile->projectile.target_object_index != NONE && definition->projectile.guided_angular_velocity > 0.f)
		{
			real_point3d target_point;
			real_vector3d projectile_to_target;
			real_vector3d axis;
			real_euler_angles2d offset_angles;
			real_vector3d offset_vector;
			real distance;
			real offset_scale;
			struct object_datum *target = object_get(projectile->projectile.target_object_index);
			real maximum_angle = definition->projectile.guided_angular_velocity * SECONDS_PER_TICK;

			if (TEST_FLAG(_object_mask_unit, target->object.type) &&
				unit_get(projectile->projectile.target_object_index)->unit.player_index != NONE)
			{
				maximum_angle *= game_difficulty_get_value(_game_difficulty_projectile_guidance_vs_player_scale);
			}

			distance = distance3d(&target->object.bounding_sphere_center, &projectile->object.bounding_sphere_center);

			if (distance > 10.f)
			{
				offset_scale = 1.f;
			}
			else if (distance > 2.f)
			{
				offset_scale = (distance - 2.f) / (10.f - 2.f);
				offset_scale = PIN(offset_scale, 0.f, 1.f);
			}
			else
			{
				offset_scale = 0.f;
			}

			unit_get_center_of_mass(projectile->projectile.target_object_index, &target_point);
			set_real_euler_angles2d(
				&offset_angles,
				_pi - periodic_function_evaluate(
					_periodic_function_wander,
					((game_time_get() + 3 * DATUM_INDEX_TO_IDENTIFIER(projectile_index)) & UNSIGNED_SHORT_MAX) / (3.f * TICKS_PER_SECOND)) * _half_pi,
				periodic_function_evaluate(
					_periodic_function_wander,
					((game_time_get() + 7 * DATUM_INDEX_TO_IDENTIFIER(projectile_index)) & UNSIGNED_SHORT_MAX) / (3.f * TICKS_PER_SECOND)) * (2.f * _pi));
			vector3d_from_euler_angles2d(&offset_vector, &offset_angles);
			point_from_line3d(&target_point, &offset_vector, offset_scale, &target_point);
			vector_from_points3d(&projectile->object.position, &target_point, &projectile_to_target);
			cross_product3d(&projectile->object.translational_velocity, &projectile_to_target, &axis);

			if (dot_product3d(&projectile_to_target, &projectile->object.translational_velocity) > 0.f && normalize3d(&axis) > 0.f)
			{
				rotate_vector_about_axis(
					&new_velocity,
					&axis,
					sine(maximum_angle),
					cosine(maximum_angle));
			}
		}

		match_vassert("c:\\halo\\SOURCE\\items\\projectiles.c", 413, valid_real_vector3d(&new_velocity), "projectile velocity is bad after steering.");

		if (projectile->projectile.deceleration_timer >= 1.f)
		{
			if (speed > definition->projectile.final_velocity && projectile->projectile.deceleration != 0.f)
			{
				new_speed = speed - projectile->projectile.deceleration * time_left;

				if (new_speed <= definition->projectile.final_velocity)
				{
					real fraction = (speed - definition->projectile.final_velocity) / (projectile->projectile.deceleration * time_left);

					new_speed = definition->projectile.final_velocity * 0.99f;
					average_speed = (new_speed + speed) * fraction * 0.5f + (1.f - fraction) * definition->projectile.final_velocity;
					scale_vector3d(&new_velocity, new_speed / speed, &new_velocity);
					average_velocity.i = (new_velocity.i + projectile->object.translational_velocity.i) * fraction * 0.5f + (1.f - fraction) * new_velocity.i;
					average_velocity.j = (new_velocity.j + projectile->object.translational_velocity.j) * fraction * 0.5f + (1.f - fraction) * new_velocity.j;
					average_velocity.k = (new_velocity.k + projectile->object.translational_velocity.k) * fraction * 0.5f + (1.f - fraction) * new_velocity.k;
				}
				else
				{
					average_speed = speed - projectile->projectile.deceleration * time_left * 0.5f;
					scale_vector3d(&new_velocity, new_speed / speed, &new_velocity);
					average_velocity.i = (new_velocity.i + projectile->object.translational_velocity.i) * 0.5f;
					average_velocity.j = (new_velocity.j + projectile->object.translational_velocity.j) * 0.5f;
					average_velocity.k = (new_velocity.k + projectile->object.translational_velocity.k) * 0.5f;
				}
			}
			else if (definition->projectile.detonation_maximum_range == 0.f &&
				definition->projectile.detonation_maximum_time == 0.f &&
				definition->projectile.detonation_minimum_velocity <= definition->projectile.final_velocity &&
				(projectile->projectile.deceleration != 0.f || projectile->projectile.odometer >= projectile->projectile.maximum_damage_distance))
			{
				projectile_set_action(projectile_index, _projectile_action_disappear);
			}
			else if (speed < definition->projectile.final_velocity && speed > 0.f)
			{
				scale_vector3d(
					&new_velocity,
					definition->projectile.final_velocity / speed * 0.99f,
					&new_velocity);
			}
		}

		match_vassert("c:\\halo\\SOURCE\\items\\projectiles.c", 461, valid_real_vector3d(&new_velocity), "projectile velocity is bad after deceleration.");

		if (TEST_FLAG(projectile->object.flags, _object_wholly_under_media_bit))
		{
			gravity = global_gravity * definition->projectile.water_gravity_scale;
		}
		else
		{
			gravity = global_gravity * definition->projectile.air_gravity_scale;
		}

		new_velocity.k -= gravity * time_left;
		average_velocity.k -= gravity * time_left * 0.5f;
		match_vassert("c:\\halo\\SOURCE\\items\\projectiles.c", 476, valid_real_vector3d(&new_velocity), "projectile velocity is bad after gravity.");

		if (definition->projectile.detonation_maximum_range != 0.f &&
			average_speed * time_left + projectile->projectile.odometer > definition->projectile.detonation_maximum_range)
		{
			if (average_speed != 0.f)
			{
				travel_fraction = (definition->projectile.detonation_maximum_range - projectile->projectile.odometer) / average_speed * time_left;
			}
			else
			{
				travel_fraction = 0.f;
			}

			projectile_set_action(projectile_index, _projectile_action_detonate);
		}
		else
		{
			travel_fraction = 1.f;
		}

		point_from_line3d(
			&projectile->object.position,
			&average_velocity,
			travel_fraction * time_left,
			&new_position);
		match_assert_valid_real_point3d("c:\\halo\\SOURCE\\items\\projectiles.c", 505, &new_position);
		match_collision_log_begin_user("c:\\halo\\SOURCE\\items\\projectiles.c", 507, _collision_user_projectiles);

		if (collision_count != MAXIMUM_PROJECTILE_COLLISIONS_PER_UPDATE &&
			projectile->projectile.action != _projectile_action_disappear &&
			(moved = TRUE) &&
			projectile_collision_test_line(projectile_index, &new_position, &collision))
		{
			time_left = 1.f - collision.t;
			new_velocity.k += gravity * time_left;

			if (new_speed != 0.f)
			{
				scale_vector3d(
					&new_velocity,
					CEILING(time_left * projectile->projectile.deceleration + new_speed, speed) / new_speed,
					&new_velocity);
			}

			if (collision.plane.n.k > 0.3f)
			{
				SET_FLAG(projectile->projectile.flags, _projectile_collided_once_bit, TRUE);
			}

			projectile->projectile.ignore_object_index = NONE;
			projectile_collision(
				projectile_index,
				&collision,
				&new_position,
				&new_velocity,
				time_left);
			collision_count++;
			ai_handle_spatial_effect(
				projectile_index,
				&collision.point,
				_ai_spatial_effect_weapon_impact,
				definition->projectile.impact_noise,
				1);

			if (TEST_FLAG(projectile->projectile.flags, _projectile_attached_bit))
			{
				moved = FALSE;
			}
		}
		else
		{
			if (collision_count == MAXIMUM_PROJECTILE_COLLISIONS_PER_UPDATE)
			{
				projectile_set_action(projectile_index, _projectile_action_detonate);
			}

			time_left = 0.f;
		}

		match_collision_log_end_user("c:\\halo\\SOURCE\\items\\projectiles.c", 560);

		if (moved)
		{
			real_vector3d travel;

			vector_from_points3d(&projectile->object.position, &new_position, &travel);
			projectile->projectile.odometer += magnitude3d(&travel);

			if (!flyby_played && definition->projectile.flyby_sound.index != NONE)
			{
				short local_player_index;

				for (local_player_index = 0; local_player_index < MAXIMUM_NUMBER_OF_LOCAL_PLAYERS; local_player_index++)
				{
					long unit_index = local_player_get_player_index(local_player_index) == NONE ? NONE : player_get(local_player_get_player_index(local_player_index))->unit_index;

					if (unit_index != NONE && unit_index != ignore_object_index)
					{
						real_vector3d ps;
						real_vector3d perp;
						real_vector3d parallel;
						real distance_along_travel;
						real_point3d *center = &object_get(unit_index)->object.bounding_sphere_center;
						real maximum_distance = sound_definition_get_maximum_distance(definition->projectile.flyby_sound.index);

						vector_from_points3d(&projectile->object.position, center, &ps);
						component_vectors_from_direction3d(&ps, &travel, &parallel, &perp);
						distance_along_travel = dot_product3d(&parallel, &travel);

						if (distance_along_travel >= 0.f &&
							distance_along_travel < magnitude_squared3d(&travel) &&
							magnitude_squared3d(&perp) < maximum_distance * maximum_distance)
						{
							struct sound_location sound_location;

							point_from_line3d(center, &perp, -1.f, &sound_location.position);
							sound_location.forward = travel;
							normalize3d(&sound_location.forward);
							sound_location.translational_velocity = *global_zero_vector3d;
							sound_location.game_location = collision.location;
							unattached_impulse_sound_new(
								definition->projectile.flyby_sound.index,
								&sound_location,
								1.f);
							flyby_played = TRUE;
						}
					}
				}
			}

			if (TEST_FLAG(definition->projectile.flags, _projectile_oriented_along_velocity_bit) &&
				(projectile->object.translational_velocity.i != 0.f || projectile->object.translational_velocity.j != 0.f || projectile->object.translational_velocity.k != 0.f))
			{
				real_vector3d forward = projectile->object.translational_velocity;

				if (normalize3d(&forward) > 0.f)
				{
					real_vector3d left;

					projectile->object.forward = forward;
					cross_product3d(&projectile->object.up, &projectile->object.forward, &left);

					if (normalize3d(cross_product3d(&projectile->object.forward, &left, &projectile->object.up)) == 0.f)
					{
						normalize3d(perpendicular3d(&projectile->object.forward, &projectile->object.up));
					}
				}

				rotate_vector_about_axis(
					&projectile->object.up,
					&projectile->object.forward,
					projectile->projectile.rotation_sine,
					projectile->projectile.rotation_cosine);
			}
			else if (TEST_FLAG(projectile->projectile.flags, _projectile_has_nonzero_angular_velocity_bit))
			{
				real_vector3d left;

				rotate_vector_about_axis(
					&projectile->object.forward,
					&projectile->projectile.rotation_axis,
					projectile->projectile.rotation_sine,
					projectile->projectile.rotation_cosine);
				rotate_vector_about_axis(
					&projectile->object.up,
					&projectile->projectile.rotation_axis,
					projectile->projectile.rotation_sine,
					projectile->projectile.rotation_cosine);
				normalize3d(&projectile->object.forward);
				cross_product3d(&projectile->object.up, &projectile->object.forward, &left);
				cross_product3d(&projectile->object.forward, &left, &projectile->object.up);
				normalize3d(&projectile->object.up);
			}

			object_translate(projectile_index, &new_position, &collision.location);
			projectile->object.translational_velocity = new_velocity;

			if (time_left != 0.f &&
				collision_count != 0 &&
				projectile->projectile.tracer_attachment_index_index != NONE &&
				projectile->object.attachment_indices[projectile->projectile.tracer_attachment_index_index] != NONE)
			{
				object_compute_node_matrices(projectile_index);
				contrail_owner_collision(
					projectile->object.attachment_indices[projectile->projectile.tracer_attachment_index_index],
					FALSE,
					(1.f - time_left) * seconds_per_tick);
			}
		}

		match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\items\\projectiles.c", 672, &projectile->object.translational_velocity);
	}

	switch (projectile->projectile.action)
	{
	case _projectile_action_detonate:
		if (projectile->projectile.arming_time_delta != 0.f && projectile->projectile.arming_time < 1.f)
		{
			break;
		}

		projectile_detonate(projectile_index, collision_count == 0, time_left);
	case _projectile_action_disappear:
		object_delete(projectile_index);
		break;
	}

	match_assert_valid_real_vector3d_axes2("c:\\halo\\SOURCE\\items\\projectiles.c", 688, &projectile->object.forward, &projectile->object.up);
	profile_exit(projectile_update_section);

	return TRUE;
}

real projectile_get_ballistic_acceleration(
	struct projectile_definition const *projectile_definition)
{
	return -(global_gravity * projectile_definition->projectile.air_gravity_scale);
}

boolean projectile_aim_ballistic(
	real base_velocity,
	real gravity_scale,
	real_point3d const *origin,
	real_point3d const *target_point,
	real *target_velocity_min,
	real *target_ballistic_fraction_min,
	real *forced_velocity,
	boolean lob,
	real_vector3d *result_aim_vector,
	real *result_velocity,
	real *result_ticks,
	real *result_distance,
	real *result_vertical_velocity,
	real *result_horizontal_velocity)
{
	real_vector3d aim_vector;
	real_vector3d target_vector;
	real vertical_distance;
	real horizontal_distance_squared;
	real gravity;
	real calculated_ticks;
	real calculated_distance;
	real vertical_velocity;
	real horizontal_velocity;
	real calculated_velocity = base_velocity;
	boolean aiming_successful = TRUE;

	vector_from_points3d(origin, target_point, &target_vector);
	vertical_distance = target_vector.k;
	horizontal_distance_squared = magnitude_squared2d((real_vector2d *)&target_vector);
	gravity = MAX(0.f, global_gravity * gravity_scale);

	{
		real t_max;
		real v_min;
		real c = gravity * gravity / 4.f;
		real a = vertical_distance * vertical_distance + horizontal_distance_squared;
		boolean found_solution = FALSE;

		{
			real b_max;
			real t_squared_max;
			real v_squared_min;

			match_assert("c:\\halo\\SOURCE\\items\\projectiles.c", 760, 4.0f * a * c > 0.0f);
			b_max = -square_root(4.0f * a * c);
			t_squared_max = -b_max / (2.f * c);
			match_assert("c:\\halo\\SOURCE\\items\\projectiles.c", 764, t_squared_max >= 0.0f);
			t_max = square_root(t_squared_max);
			v_squared_min = gravity * vertical_distance - b_max;
			v_min = (v_squared_min < 0.f) ? 0.f : square_root(v_squared_min);
		}

		if (forced_velocity)
		{
			calculated_velocity = *forced_velocity;
		}
		else
		{
			calculated_velocity = base_velocity;

			if (target_ballistic_fraction_min && *target_ballistic_fraction_min > 0.f)
			{
				real b_desired;
				real v_desired_sq;
				real v_desired;
				real t_desired = *target_ballistic_fraction_min * t_max;
				real t_desired_sq;

				PIN(t_desired, 0.001f, t_max);
				t_desired_sq = t_desired * t_desired;
				b_desired = -(c * t_desired_sq + a / t_desired_sq);
				v_desired_sq = gravity * vertical_distance - b_desired;
				match_assert("c:\\halo\\SOURCE\\items\\projectiles.c", 806, v_desired_sq > 0.0f);
				v_desired = square_root(v_desired_sq);
				calculated_velocity = MIN(calculated_velocity, v_desired);
			}
		}

		if (calculated_velocity >= v_min)
		{
			real b = gravity * vertical_distance - calculated_velocity * calculated_velocity;
			real disc = b * b - 4.0f * c * a;

			if (b < 0.f && disc >= 0.f)
			{
				real t_squared = (-b + (lob ? 1 : -1) * square_root(disc)) / (2.f * c);

				if (t_squared > 0.f)
				{
					calculated_ticks = square_root(t_squared);
					found_solution = TRUE;
				}
			}
		}

		if (!found_solution)
		{
			aiming_successful = FALSE;
			calculated_ticks = t_max;
			calculated_velocity = v_min;
		}
	}

	scale_vector3d(&target_vector, 1.f / calculated_ticks, &aim_vector);
	aim_vector.k += calculated_ticks * gravity * 0.5f;
	horizontal_velocity = magnitude2d((real_vector2d *)&aim_vector);
	vertical_velocity = aim_vector.k;
	calculated_distance = calculated_ticks * calculated_velocity;

	if (normalize3d(&aim_vector) == 0.f)
	{
		aiming_successful = FALSE;
		aim_vector = target_vector;

		if (normalize3d(&aim_vector) == 0.f)
		{
			aim_vector = *global_up3d;
		}
	}

	match_assert("c:\\halo\\SOURCE\\items\\projectiles.c", 867, result_aim_vector);
	*result_aim_vector = aim_vector;

	if (result_distance)
	{
		*result_distance = calculated_distance;
	}

	if (result_velocity)
	{
		*result_velocity = calculated_velocity;
	}

	if (result_vertical_velocity)
	{
		*result_vertical_velocity = vertical_velocity;
	}

	if (result_horizontal_velocity)
	{
		*result_horizontal_velocity = horizontal_velocity;
	}

	if (result_ticks)
	{
		*result_ticks = calculated_ticks;
	}

	return aiming_successful;
}

boolean projectile_aim_linear(
	real base_velocity,
	real_point3d const *origin,
	real_point3d const *target_point,
	real_vector3d *result_aim_vector,
	real *result_velocity,
	real *result_ticks,
	real *result_distance)
{
	real_vector3d aim_vector;
	real calculated_distance;
	real calculated_ticks;
	real calculated_velocity = base_velocity;

	vector_from_points3d(origin, target_point, &aim_vector);
	calculated_distance = normalize3d(&aim_vector);

	if (calculated_velocity > 0.f)
	{
		calculated_ticks = calculated_distance / calculated_velocity;
	}
	else
	{
		calculated_ticks = 0.f;
	}

	match_assert("c:\\halo\\SOURCE\\items\\projectiles.c", 921, result_aim_vector);
	*result_aim_vector = aim_vector;

	if (result_distance)
	{
		*result_distance = calculated_distance;
	}

	if (result_velocity)
	{
		*result_velocity = calculated_velocity;
	}

	if (result_ticks)
	{
		*result_ticks = calculated_ticks;
	}

	return TRUE;
}

boolean projectile_aim(
	struct projectile_definition const *projectile_definition,
	real_point3d const *origin,
	real_point3d const *target_point,
	real *override_velocity_max,
	real *target_velocity_min,
	real *target_ballistic_fraction_min,
	real *forced_velocity,
	boolean lob,
	real_vector3d *result_aim_vector,
	real *result_velocity,
	real *result_ticks,
	real *result_distance,
	boolean *result_linear)
{
	boolean aiming_successful;
	real base_velocity;

	if (!override_velocity_max)
	{
		base_velocity = projectile_definition->projectile.initial_velocity;
	}
	else
	{
		base_velocity = *override_velocity_max;
	}

	if (TEST_FLAG(projectile_definition->projectile.flags, _projectile_aim_ballistic_bit) && projectile_definition->projectile.air_gravity_scale > 0.f)
	{
		aiming_successful = projectile_aim_ballistic(
			base_velocity,
			projectile_definition->projectile.air_gravity_scale,
			origin,
			target_point,
			target_velocity_min,
			target_ballistic_fraction_min,
			forced_velocity,
			lob,
			result_aim_vector,
			result_velocity,
			result_ticks,
			result_distance,
			NULL,
			NULL);

		if (result_linear)
		{
			*result_linear = FALSE;
		}
	}
	else
	{
		aiming_successful = projectile_aim_linear(
			base_velocity,
			origin,
			target_point,
			result_aim_vector,
			result_velocity,
			result_ticks,
			result_distance);

		if (result_linear)
		{
			*result_linear = TRUE;
		}
	}

	return aiming_successful;
}

real projectile_estimate_time_to_target(
	struct projectile_definition const *projectile_definition,
	real target_distance)
{
	real time = 0.f;

	if (projectile_definition->projectile.initial_velocity > 0.f)
	{
		time = target_distance / projectile_definition->projectile.initial_velocity;
	}

	return time;
}

void projectile_accelerate(
	long projectile_index,
	real_vector3d const *acceleration)
{
	struct projectile_datum *projectile = projectile_get(projectile_index);
	struct projectile_definition *definition = projectile_definition_get(projectile->definition_index);

	match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\items\\projectiles.c", 1007, acceleration);

	if (projectile->object.parent_object_index != NONE)
	{
		return;
	}

	match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\items\\projectiles.c", 1011, &projectile->object.translational_velocity);
	add_vectors3d(
		&projectile->object.translational_velocity,
		acceleration,
		&projectile->object.translational_velocity);

	{
		real_vector3d random_angular_velocity;

		random_direction3d(&random_angular_velocity);
		scale_vector3d(
			&random_angular_velocity,
			real_random() * magnitude3d(acceleration) * _half_pi,
			&random_angular_velocity);
		add_vectors3d(
			&projectile->object.angular_velocity,
			&random_angular_velocity,
			&projectile->object.angular_velocity);
	}

	projectile_adjust_for_angular_velocity_change(projectile_index);
	SET_FLAG(projectile->object.flags, _object_at_rest_bit, FALSE);
	match_assert_valid_real_vector3d("c:\\halo\\SOURCE\\items\\projectiles.c", 1029, &projectile->object.translational_velocity);

	return;
}

boolean dangerous_projectiles_near_player(
	void)
{
	struct object_iterator iterator;
	struct projectile_datum *projectile;

	object_iterator_new(&iterator, _object_mask_projectile, 0);
	projectile = object_iterator_next(&iterator);

	if (projectile)
	{
		struct projectile_definition *definition = projectile_definition_get(projectile->definition_index);

		return TRUE;
	}

	return FALSE;
}

void projectile_handle_deleted_object(
	long projectile_index,
	long deleted_object_index)
{
	struct projectile_datum *projectile = projectile_get(projectile_index);

	if (projectile->projectile.target_object_index == deleted_object_index)
	{
		projectile->projectile.target_object_index = NONE;
	}

	return;
}

static void projectile_set_action(
	long projectile_index,
	short action)
{
	struct projectile_datum *projectile = projectile_get(projectile_index);

	if (action > projectile->projectile.action)
	{
		projectile->projectile.action = action;
	}

	return;
}

static void projectile_collision(
	long projectile_index,
	struct collision_result *collision,
	real_point3d *new_position,
	real_vector3d *new_velocity,
	real time_left)
{
	real_vector3d original_direction;
	real magnitude;
	real velocity_fraction;
	real velocity_into_surface;
	real angle_of_incidence;
	real velocity_squared;
	short response;
	long effect_definition_index;
	struct projectile_material_response_definition *material_response;
	struct projectile_datum *projectile = projectile_get(projectile_index);
	struct projectile_definition *definition = projectile_definition_get(projectile->definition_index);
	short material_type = collision->material_type;
	real effect_scale = 1.f;
	real material_effect_scale = 0.f;

	original_direction = *new_velocity;
	magnitude = normalize3d(&original_direction);

	if (magnitude == 0.f)
	{
		original_direction = *global_up3d;
	}

	if (definition->projectile.final_velocity != definition->projectile.initial_velocity)
	{
		velocity_fraction = (magnitude - definition->projectile.final_velocity) / (definition->projectile.initial_velocity - definition->projectile.final_velocity);
		velocity_fraction = PIN(velocity_fraction, 0.f, 1.f);
	}
	else
	{
		velocity_fraction = 1.f;
	}

	if (collision->type == _collision_result_object && definition->projectile.impact_damage.index != NONE)
	{
		struct damage_data damage_data;

		match_assert("c:\\halo\\SOURCE\\items\\projectiles.c", 1148, collision->object_index!=NONE);
		damage_data_new(&damage_data, definition->projectile.impact_damage.index);
		SET_FLAG(damage_data.flags, _damage_from_weapon_bit, TRUE);
		damage_data.scale = velocity_fraction;
		damage_data.owner_player_index = projectile->object.owner_player_index;
		damage_data.owner_object_index = projectile->object.owner_object_index;
		damage_data.owner_team_index = projectile->object.owner_team_index;
		damage_data.epicenter = collision->point;
		damage_data.origin = collision->point;
		damage_data.direction = *new_velocity;
		normalize3d(&damage_data.direction);
		object_cause_damage(
			&damage_data,
			collision->object_index,
			collision->node_index,
			collision->region_index,
			collision->material_index,
			&collision->plane.n);

		if (damage_data.material_type != NONE)
		{
			material_type = damage_data.material_type;
		}

		material_effect_scale = damage_data.material_effect_scale;
	}

	projectile->projectile.hit_material_type = material_type;

	if (material_type >= 0 && material_type < definition->projectile.material_responses.count)
	{
		material_response = TAG_BLOCK_GET_ELEMENT(
			&definition->projectile.material_responses,
			material_type,
			struct projectile_material_response_definition);
	}
	else
	{
		material_response = &default_projectile_material_response;
	}

	velocity_into_surface = -dot_product3d(&collision->plane.n, new_velocity) + real_random_range(-material_response->velocity_noise, 0.f);
	angle_of_incidence = angle_between_vectors3d(new_velocity, &collision->plane.n) - _half_pi + real_random_range(-material_response->angle_noise, material_response->angle_noise);

	if (material_response->possible_response != _projectile_response_disappear &&
		(material_response->possible_response_maximum_angle == 0.f || (angle_of_incidence >= material_response->possible_response_minimum_angle && angle_of_incidence <= material_response->possible_response_maximum_angle)) &&
		(material_response->possible_response_maximum_velocity == 0.f || (velocity_into_surface >= material_response->possible_response_minimum_velocity && velocity_into_surface <= material_response->possible_response_maximum_velocity)) &&
		(!TEST_FLAG(material_response->possible_response_flags, _projectile_possible_response_only_against_units_bit) ||
		(collision->type == _collision_result_object && object_try_and_get_and_verify_type(collision->object_index, _object_mask_unit))) &&
		real_random() >= material_response->possible_response_skip_fraction)
	{
		response = material_response->possible_response;
		effect_definition_index = material_response->possible_response_effect.index;
	}
	else
	{
		response = material_response->default_response;
		effect_definition_index = material_response->default_effect.index;
	}

	if (collision->type == _collision_result_structure && TEST_FLAG(collision->flags, _collision_surface_breakable_bit))
	{
		struct damage_data damage_data;

		damage_data_new(&damage_data, definition->projectile.impact_damage.index);
		SET_FLAG(damage_data.flags, _damage_from_weapon_bit, TRUE);
		damage_data.epicenter = collision->point;
		damage_data.origin = collision->point;
		damage_data.direction = *new_velocity;
		normalize3d(&damage_data.direction);
		damage_data.material_type = collision->material_type;

		if (damage_data.material_type >= 0 && damage_data.material_type < definition->projectile.material_responses.count)
		{
			damage_data.material_response = TAG_BLOCK_GET_ELEMENT(
				&definition->projectile.material_responses,
				damage_data.material_type,
				struct projectile_material_response_definition);
		}
		else
		{
			damage_data.material_response = &default_projectile_material_response;
		}

		damage_data.location = collision->location;
		breakable_surface_damage(
			collision->breakable_surface_index,
			&damage_data,
			collision->surface_index);
	}

	*new_position = collision->point;

	if (response == _projectile_response_penetrate)
	{
		if (collision->type == _collision_result_media)
		{
			SET_FLAG(projectile->object.flags, _object_wholly_under_media_bit, !TEST_FLAG(projectile->object.flags, _object_wholly_under_media_bit));
			projectile_calculate_deceleration(projectile_index);
			point_from_line3d(new_position, &collision->plane.n, -0.001f, new_position);
		}
		else if (collision->type == _collision_result_object)
		{
			scale_vector3d(new_velocity, 1.f - material_response->penetration_initial_friction, new_velocity);
			projectile->projectile.ignore_object_index = collision->object_index;
		}
		else if (definition->projectile.detonation_maximum_time != 0.f)
		{
			SET_FLAG(projectile->projectile.flags, _projectile_collided_once_bit, TRUE);
			SET_FLAG(projectile->projectile.flags, _projectile_stopped_after_collision_bit, TRUE);
			response = _projectile_response_attach;
		}
		else
		{
			response = _projectile_response_detonate;
		}
	}

	if (response == _projectile_response_reflect)
	{
		real_vector3d perpendicular_velocity;
		real_vector3d parallel_velocity;

		component_vectors_from_direction3d(
			new_velocity,
			&collision->plane.n,
			&parallel_velocity,
			&perpendicular_velocity);
		new_velocity->i = (1.f - material_response->reflection_perpendicular_friction) * perpendicular_velocity.i - (1.f - material_response->reflection_parallel_friction) * parallel_velocity.i;
		new_velocity->j = (1.f - material_response->reflection_perpendicular_friction) * perpendicular_velocity.j - (1.f - material_response->reflection_parallel_friction) * parallel_velocity.j;
		new_velocity->k = (1.f - material_response->reflection_perpendicular_friction) * perpendicular_velocity.k - (1.f - material_response->reflection_parallel_friction) * parallel_velocity.k;
	}
	else if (response != _projectile_response_penetrate)
	{
		*new_velocity = *global_zero_vector3d;
	}

	if (material_response->angle_noise != 0.f)
	{
		random_vector_in_cone3d(new_velocity, 0.f, material_response->angle_noise, new_velocity);
	}

	if (material_response->velocity_noise != 0.f)
	{
		real new_velocity_speed = normalize3d(new_velocity);

		if (new_velocity_speed != 0.f)
		{
			scale_vector3d(
				new_velocity,
				real_random_range(-material_response->velocity_noise, material_response->velocity_noise) + new_velocity_speed,
				new_velocity);
		}
	}

	velocity_squared = magnitude_squared3d(new_velocity);

	if (response != _projectile_response_attach &&
		magnitude_squared3d(new_velocity) < definition->projectile.detonation_minimum_velocity * definition->projectile.detonation_minimum_velocity)
	{
		projectile_set_action(projectile_index, _projectile_action_detonate);
	}

	if (velocity_squared < _real_epsilon)
	{
		SET_FLAG(projectile->projectile.flags, _projectile_stopped_after_collision_bit, TRUE);

		if (collision->plane.n.k > 0.3f)
		{
			SET_FLAG(projectile->object.flags, _object_at_rest_bit, TRUE);

			if (definition->projectile.detonation_maximum_time == 0.f)
			{
				projectile_set_action(projectile_index, _projectile_action_detonate);
			}
		}
	}

	switch (material_response->scale_effects_by)
	{
	case _projectile_material_response_scale_effects_by_damage:
		effect_scale = velocity_fraction;
		break;
	case _projectile_material_response_scale_effects_by_angle:
		effect_scale = angle_of_incidence * (1.f / _half_pi);
		break;
	}

	effect_scale = PIN(effect_scale, 0.f, 1.f);
	material_effect_scale = PIN(material_effect_scale, 0.f, 1.f);

	{
		real_vector3d effect_vectors[NUMBER_OF_EFFECT_MARKERS];
		real_point3d effect_points[NUMBER_OF_EFFECT_MARKERS];
		short marker_index;

		scale_vector3d(&original_direction, -1.f, &effect_vectors[_effect_vector_incident]);
		effect_vectors[_effect_vector_negative_incident] = original_direction;
		effect_vectors[_effect_vector_gravity] = *global_down3d;
		effect_vectors[_effect_vector_normal] = collision->plane.n;
		reflect_vector3d(
			&original_direction,
			&collision->plane.n,
			&effect_vectors[_effect_vector_reflected]);

		for (marker_index = 0; marker_index < NUMBER_OF_EFFECT_MARKERS; marker_index++)
		{
			effect_points[marker_index] = collision->point;
		}

		if (velocity_into_surface > SECONDS_PER_TICK / 4.f)
		{
			projectile_effect_new(
				projectile_index,
				effect_definition_index,
				collision,
				effect_points,
				effect_vectors,
				effect_scale,
				material_effect_scale);
		}

		if (!TEST_FLAG(projectile->projectile.flags, _projectile_counting_down_bit) &&
			(TEST_FLAG(projectile->projectile.flags, _projectile_stopped_after_collision_bit) || response == _projectile_response_attach))
		{
			projectile_effect_new(
				projectile_index,
				definition->projectile.detonation_timer_started.index,
				collision,
				effect_points,
				effect_vectors,
				effect_scale,
				material_effect_scale);
		}
	}

	switch (response)
	{
	case _projectile_response_disappear:
		projectile_set_action(projectile_index, _projectile_action_disappear);
		break;
	case _projectile_response_detonate:
		projectile_set_action(projectile_index, _projectile_action_detonate);
		break;
	case _projectile_response_reflect:
	case _projectile_response_penetrate:
		break;
	case _projectile_response_attach:
		if (collision->type == _collision_result_object && TEST_FLAG(definition->projectile.flags, _projectile_super_combining_explosion_bit))
		{
			long child_index = object_get(collision->object_index)->object.first_child_object_index;
			short attached_count = 0;

			while (child_index != NONE)
			{
				struct object_datum *child = object_get(child_index);

				if (child->definition_index == projectile->definition_index &&
					!TEST_FLAG(projectile_get(child_index)->projectile.flags, _projectile_already_super_exploded_bit))
				{
					struct projectile_datum *child_projectile = projectile_get(child_index);

					child_projectile->projectile.arming_time = 0.f;
					child_projectile->projectile.detonation_timer = 0.f;
					attached_count++;
				}

				if (attached_count >= SUPER_DETONATION_PROJECTILE_COUNT)
				{
					SET_FLAG(projectile->projectile.flags, _projectile_will_super_explode_bit, TRUE);
					break;
				}

				child_index = child->object.next_object_index;
			}
		}

		projectile->object.translational_velocity = *global_zero_vector3d;
		projectile->object.angular_velocity = *global_zero_vector3d;
		SET_FLAG(projectile->projectile.flags, _projectile_attached_bit, TRUE);
		SET_FLAG(projectile->object.flags, _object_at_rest_bit, TRUE);
		object_translate(projectile_index, new_position, &collision->location);

		if (collision->type == _collision_result_object)
		{
			object_attach_to_node(collision->object_index, projectile_index, collision->node_index);
		}

		if (TEST_FLAG(definition->projectile.flags, _projectile_detonation_max_time_if_attached_bit) &&
			definition->projectile.detonation_maximum_time * TICKS_PER_SECOND >= 1.f)
		{
			projectile->projectile.detonation_timer_delta = 1.f / (definition->projectile.detonation_maximum_time * TICKS_PER_SECOND);
		}

		break;
	default:
		match_halt("c:\\halo\\SOURCE\\items\\projectiles.c", 1487);
	}

	return;
}

static void projectile_effect_new(
	long projectile_index,
	long definition_index,
	struct collision_result const *collision,
	real_point3d const *points,
	real_vector3d const *vectors,
	real scale_a,
	real scale_b)
{
	if (collision->type == _collision_result_object)
	{
		effect_new_attached_from_markers(
			definition_index,
			projectile_index,
			collision->object_index,
			collision->node_index,
			NUMBER_OF_EFFECT_MARKERS,
			effect_marker_names,
			points,
			vectors,
			scale_a,
			scale_b,
			NULL,
			NULL);
	}
	else
	{
		effect_new_unattached_from_markers(
			definition_index,
			projectile_index,
			NULL,
			NUMBER_OF_EFFECT_MARKERS,
			effect_marker_names,
			points,
			vectors,
			scale_a,
			scale_b,
			NULL,
			NULL,
			TRUE);
	}

	return;
}

static void projectile_adjust_for_angular_velocity_change(
	long object_index)
{
	struct projectile_datum *projectile = projectile_get(object_index);
	real angular_velocity_magnitude = magnitude3d(&projectile->object.angular_velocity);

	if (angular_velocity_magnitude != 0.f)
	{
		SET_FLAG(projectile->projectile.flags, _projectile_has_nonzero_angular_velocity_bit, TRUE);
		scale_vector3d(
			&projectile->object.angular_velocity,
			1.f / angular_velocity_magnitude,
			&projectile->projectile.rotation_axis);
		projectile->projectile.rotation_sine = sine(angular_velocity_magnitude);
		projectile->projectile.rotation_cosine = cosine(angular_velocity_magnitude);
	}
	else
	{
		SET_FLAG(projectile->projectile.flags, _projectile_has_nonzero_angular_velocity_bit, FALSE);
		projectile->projectile.rotation_sine = 0.f;
		projectile->projectile.rotation_cosine = 1.f;
	}

	return;
}

void projectile_export_function_values(
	long projectile_index)
{
	short function_index;
	struct projectile_datum *projectile = projectile_get(projectile_index);
	struct projectile_definition *definition = projectile_definition_get(projectile->definition_index);

	for (function_index = 0; function_index < NUMBER_OF_INCOMING_OBJECT_FUNCTIONS; function_index++)
	{
		if (definition->projectile.function_modes[function_index] != _projectile_function_none)
		{
			real value;

			switch (definition->projectile.function_modes[function_index])
			{
			case _projectile_function_range_remaining:
				value = (definition->projectile.detonation_maximum_range != 0.f) ? projectile->projectile.odometer / definition->projectile.detonation_maximum_range : 0.f;
				break;
			case _projectile_function_time_remaining:
				value = projectile->projectile.detonation_timer;
				break;
			case _projectile_function_tracer:
				value = TEST_FLAG(projectile->projectile.flags, _projectile_tracer_bit) ? 1.f : 0.f;
				break;
			default:
				match_halt("c:\\halo\\SOURCE\\items\\projectiles.c", 1570);
			}

			projectile->object.incoming_function_values[function_index] = value;
		}
	}

	return;
}

static void projectile_calculate_deceleration(
	long projectile_index)
{
	struct projectile_datum *projectile = projectile_get(projectile_index);
	struct projectile_definition *definition = projectile_definition_get(projectile->definition_index);

	if (TEST_FLAG(projectile->object.flags, _object_wholly_under_media_bit))
	{
		projectile->projectile.deceleration = projectile_calculate_deceleration_from_distances(
			definition,
			definition->projectile.water_minimum_damage_distance,
			definition->projectile.water_maximum_damage_distance);
		projectile->projectile.maximum_damage_distance = definition->projectile.water_maximum_damage_distance;

		if (definition->projectile.water_minimum_damage_distance > 0.f)
		{
			projectile->projectile.deceleration_timer_delta = definition->projectile.water_minimum_damage_distance / definition->projectile.initial_velocity;
		}
		else
		{
			projectile->projectile.deceleration_timer = 1.f;
			projectile->projectile.deceleration_timer_delta = 0.f;
		}
	}
	else
	{
		projectile->projectile.deceleration = projectile_calculate_deceleration_from_distances(
			definition,
			definition->projectile.air_minimum_damage_distance,
			definition->projectile.air_maximum_damage_distance);
		projectile->projectile.maximum_damage_distance = definition->projectile.water_maximum_damage_distance;

		if (definition->projectile.air_minimum_damage_distance > 0.f)
		{
			projectile->projectile.deceleration_timer_delta = definition->projectile.air_minimum_damage_distance / definition->projectile.initial_velocity;
		}
		else
		{
			projectile->projectile.deceleration_timer = 1.f;
			projectile->projectile.deceleration_timer_delta = 0.f;
		}
	}

	return;
}

static real projectile_calculate_deceleration_from_distances(
	struct projectile_definition const *projectile_definition,
	real minimum_distance,
	real maximum_distance)
{
	real distance = maximum_distance - minimum_distance;
	real deceleration = 0.f;

	if (projectile_definition->projectile.initial_velocity != projectile_definition->projectile.final_velocity && distance != 0.f)
	{
		deceleration = (projectile_definition->projectile.initial_velocity * projectile_definition->projectile.initial_velocity -
			projectile_definition->projectile.final_velocity * projectile_definition->projectile.final_velocity) / (2.f * distance);
	}

	return deceleration;
}

static boolean projectile_collision_test_line(
	long projectile_index,
	real_point3d const *new_position,
	struct collision_result *collision_result)
{
	boolean result;
	struct projectile_datum *projectile = projectile_get(projectile_index);
	struct projectile_definition *definition = projectile_definition_get(projectile->definition_index);

	if (collision_test_line(
		_collision_test_for_projectiles_flags,
		&projectile->object.position,
		new_position,
		projectile->projectile.ignore_object_index,
		collision_result))
	{
		result = TRUE;
	}
	else if (definition->projectile.collision_radius < _real_epsilon)
	{
		result = FALSE;
	}
	else
	{
		real_point3d p0_left;
		real_point3d p1_left;
		real_point3d p0_right;
		real_point3d p1_right;
		real_vector3d left;
		real_vector3d forward;

		vector_from_points3d(&projectile->object.position, new_position, &forward);
		cross_product3d(global_up3d, &forward, &left);

		if (normalize3d(&left) == 0.f)
		{
			left = *global_left3d;
		}

		point_from_line3d(
			&projectile->object.position,
			&left,
			definition->projectile.collision_radius,
			&p0_left);
		point_from_line3d(
			new_position,
			&left,
			definition->projectile.collision_radius,
			&p1_left);
		point_from_line3d(
			&projectile->object.position,
			&left,
			-definition->projectile.collision_radius,
			&p0_right);
		point_from_line3d(
			new_position,
			&left,
			-definition->projectile.collision_radius,
			&p1_right);
		result =
			collision_test_line(
				_collision_test_for_projectiles_fat_flags,
				&p0_left,
				&p1_left,
				projectile->projectile.ignore_object_index,
				collision_result) ||
			collision_test_line(
				_collision_test_for_projectiles_fat_flags,
				&p0_right,
				&p1_right,
				projectile->projectile.ignore_object_index,
				collision_result);
	}

	return result;
}

static void projectile_detonate(
	long projectile_index,
	boolean first_collision,
	real time_left)
{
	real_point3d effect_points[2];
	real_vector3d effect_vectors[2];
	real_vector3d up;
	struct projectile_datum *projectile = projectile_get(projectile_index);
	struct projectile_definition *definition = projectile_definition_get(projectile->definition_index);
	char *effect_marker_names[2] = {"", "gravity"};
	long effect_definition_index = definition->projectile.detonation_effect.index;

	if (TEST_FLAG(definition->projectile.flags, _projectile_super_combining_explosion_bit) &&
		!TEST_FLAG(projectile->projectile.flags, _projectile_already_super_exploded_bit) &&
		projectile->object.parent_object_index != NONE)
	{
		struct object_datum *parent = object_get(projectile->object.parent_object_index);
		long child_index = parent->object.first_child_object_index;
		short attached_count = 0;

		while (child_index != NONE)
		{
			struct object_datum *child = object_get(child_index);

			if (child->definition_index == projectile->definition_index &&
				!TEST_FLAG(projectile_get(child_index)->projectile.flags, _projectile_already_super_exploded_bit))
			{
				attached_count++;
			}

			child_index = child->object.next_object_index;
		}

		if (parent->object.type == _object_type_biped &&
			(biped_get(projectile->object.parent_object_index)->unit.player_index == NONE || game_engine_running()) &&
			attached_count > SUPER_DETONATION_PROJECTILE_COUNT)
		{
			real_point3d parents_feet;
			real_point3d known_good_point;

			child_index = parent->object.first_child_object_index;

			while (child_index != NONE)
			{
				struct object_datum *child = object_get(child_index);

				if (child->definition_index == projectile->definition_index &&
					!TEST_FLAG(projectile_get(child_index)->projectile.flags, _projectile_already_super_exploded_bit))
				{
					struct projectile_datum *child_projectile = projectile_get(child_index);

					if (attached_count <= SUPER_DETONATION_PROJECTILE_COUNT)
					{
						SET_FLAG(child_projectile->projectile.flags, _projectile_already_super_exploded_bit, TRUE);
						child_projectile->projectile.detonation_timer *= real_random();
						child_projectile->projectile.arming_time *= real_random();
					}
					else
					{
						child_projectile->projectile.detonation_timer = 0.f;
						child_projectile->projectile.arming_time = 0.f;
					}

					attached_count--;
				}

				child_index = child->object.next_object_index;
			}

			effect_definition_index = definition->projectile.super_detonation_effect.index;
			object_get_origin(projectile->object.parent_object_index, &parents_feet);
			object_detach(projectile_index);
			known_good_point = projectile->object.position;
			object_translate(projectile_index, &parents_feet, NULL);
			object_force_inside_bsp(projectile_index, &known_good_point);
			object_compute_node_matrices_recursive(projectile_index);
		}
	}

	if (first_collision &&
		projectile->projectile.tracer_attachment_index_index != NONE &&
		projectile->object.attachment_indices[projectile->projectile.tracer_attachment_index_index] != NONE)
	{
		object_compute_node_matrices(projectile_index);
		contrail_owner_collision(
			projectile->object.attachment_indices[projectile->projectile.tracer_attachment_index_index],
			FALSE,
			(1.f - time_left) * seconds_per_tick);
	}

	object_get_origin(projectile_index, &effect_points[0]);
	object_get_orientation(projectile_index, &effect_vectors[0], &up);
	effect_points[1] = effect_points[0];
	effect_vectors[1] = *global_down3d;
	effect_new_unattached_from_markers(
		effect_definition_index,
		projectile->object.owner_object_index,
		NULL,
		NUMBEROF(effect_marker_names),
		effect_marker_names,
		effect_points,
		effect_vectors,
		0.f,
		0.f,
		NULL,
		NULL,
		TRUE);

	if (projectile->object.parent_object_index != NONE && definition->projectile.detonation_damage.index != NONE)
	{
		struct damage_data damage_data;

		damage_data_new(&damage_data, definition->projectile.detonation_damage.index);
		SET_FLAG(damage_data.flags, _damage_from_weapon_bit, TRUE);
		object_get_orientation(projectile_index, &damage_data.direction, NULL);
		object_get_origin(projectile_index, &damage_data.origin);
		damage_data.epicenter = damage_data.origin;
		damage_data.owner_object_index = projectile->object.owner_object_index;
		damage_data.owner_player_index = projectile->object.owner_player_index;
		damage_data.owner_team_index = projectile->object.owner_team_index;
		object_cause_damage(
			&damage_data,
			projectile->object.parent_object_index,
			NONE,
			NONE,
			NONE,
			NULL);
	}

	if (projectile->projectile.hit_material_type != NONE)
	{
		struct projectile_material_response_definition *material_response;

		if (projectile->projectile.hit_material_type >= 0 && projectile->projectile.hit_material_type < definition->projectile.material_responses.count)
		{
			material_response = TAG_BLOCK_GET_ELEMENT(
				&definition->projectile.material_responses,
				projectile->projectile.hit_material_type,
				struct projectile_material_response_definition);
		}
		else
		{
			material_response = &default_projectile_material_response;
		}

		effect_new_unattached_from_markers(
			material_response->detonation_effect.index,
			projectile->object.owner_object_index,
			NULL,
			NUMBEROF(effect_marker_names),
			effect_marker_names,
			effect_points,
			effect_vectors,
			0.f,
			0.f,
			NULL,
			NULL,
			TRUE);
	}

	ai_handle_spatial_effect(
		projectile_index,
		&effect_points[0],
		_ai_spatial_effect_weapon_detonation,
		definition->projectile.detonation_noise,
		1);

	return;
}

boolean projectile_handle_parent_destroyed(
	long projectile_index)
{
	struct projectile_datum *projectile = projectile_get(projectile_index);

	match_assert("c:\\halo\\SOURCE\\items\\projectiles.c", 1845, projectile->object.parent_object_index != NONE);
	projectile->projectile.arming_time = 1.f;
	projectile->projectile.detonation_timer = 1.f;
	SET_FLAG(projectile->projectile.flags, _projectile_attached_bit, FALSE);
	object_detach(projectile_index);

	return TRUE;
}

/* ---------- private code */
