/*
GLOW.C
*/

/* ---------- headers */

#include "cseries.h"
#include "glow.h"
#include "widgets.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "rasterizer.h"
#include "game_state.h"
#include "render_debug.h"
#include "rasterizer_widgets.h"
#include "game.h"
#include "shader_definitions.h"

/* ---------- constants */

enum
{
	MAXIMUM_GLOWS_PER_MAP = 8,
	MAXIMUM_MARKERS_PER_GLOW_EFFECT = 5,
	MAXIMUM_PARTICLES_PER_GLOW_EFFECT = 64,
	MAXIMUM_GLOW_PARTICLES_PER_MAP = 512,
};

#define MINIMUM_TRAILING_PARTICLE_GENERATION_FREQUENCY 0.01f	/* fake name */
#define NUMBER_OF_TRAVERSAL_POINTS 4							/* fake name */

enum
{
	_glow_particle_traversal_direction_negative_bit = 0,
	_glow_particle_is_trailing_particle_bit,
	NUMBER_OF_GLOW_PARTICLE_FLAGS,
};

/* ---------- macros */

#define glow_get(index) ((struct glow_datum *)datum_get(glow_globals.glow_data, (index))) /* fake name */
#define glow_try_and_get(index) ((struct glow_datum *)datum_try_and_get(glow_globals.glow_data, (index))) /* fake name */
#define glow_particle_get(index) ((struct glow_particle *)datum_get(glow_globals.glow_particle_data, (index))) /* fake name */
#define glow_particle_try_and_get(index) ((struct glow_particle *)datum_try_and_get(glow_globals.glow_particle_data, (index))) /* fake name */

/* ---------- structures */

struct glow_particle
{
	short identifier;
	short parent_marker_index;
	long index;
	real initial_angle;
	real_argb_color color;
	real distance_to_object;
	real initial_size;
	real present_size;
	real t;
	real_point3d position;
	real_vector3d initial_velocity;
	real_vector3d present_velocity;
	short ticks_in_existence;
	short lifetime;
	unsigned long flags;
	real fade;
	struct glow_particle *next;
	struct glow_particle *previous;
};

struct glow_datum
{
	short identifier;
	boolean initialized;
	short number_of_markers;
	struct object_marker markers[MAXIMUM_MARKERS_PER_GLOW_EFFECT];
	long definition_index;
	short bitmap_dimension;
	short marker_order[MAXIMUM_MARKERS_PER_GLOW_EFFECT];
	real total_time;
	real marker_time_index[MAXIMUM_MARKERS_PER_GLOW_EFFECT];
	short number_of_particles;
	struct glow_particle *head_particle;
	struct glow_particle *tail_particle;
	short accumulated_trailing_particle_generation_ticks;
};

/* ---------- prototypes */

static void glow_update(struct glow_datum *glow, long object_index);
static void glow_trailing_particle_update_color(struct glow_datum *glow, struct glow_particle *particle);
static void glow_trailing_particle_update_size(struct glow_datum *glow, struct glow_particle *particle);
static void glow_trailing_particle_update_velocity(struct glow_datum *glow, struct glow_particle *particle);
static void glow_trailing_particle_update_position(struct glow_datum *glow, struct glow_particle *particle, real elapsed_time);
static void glow_trailing_particle_age(struct glow_datum *glow, struct glow_particle *particle);
static void glow_normal_particle_update_color(long object_index, struct glow_datum *glow, struct glow_particle *particle);
static void glow_normal_particle_update_size(long object_index, struct glow_datum *glow, struct glow_particle *particle);
static void glow_normal_particle_update_position(long object_index, struct glow_datum *glow, struct glow_particle *particle, real elapsed_time, real rotation_per_unit_t);
static void glow_particles_initialize(struct glow_datum *glow);
static struct glow_particle *glow_normal_particle_new(struct glow_datum *glow, short index, short total);
static struct glow_particle *glow_trailing_particle_new(struct glow_datum *glow);
static struct glow_particle *glow_particle_new(struct glow_datum *glow);
static void get_particle_world_position(struct glow_datum *glow, struct glow_particle *particle, real rotation_per_unit_t);

/* ---------- globals */

struct
{
	struct data_array *glow_data;
	struct data_array *glow_particle_data;
} glow_globals;

/* ---------- public code */

void glow_initialize(
	void)
{
	boolean success = FALSE;

	if (!glow_globals.glow_data)
	{
		glow_globals.glow_data = game_state_data_new(
			"glow",
			MAXIMUM_GLOWS_PER_MAP,
			sizeof(struct glow_datum));

		if (glow_globals.glow_data)
		{
			if (!glow_globals.glow_particle_data)
			{
				glow_globals.glow_particle_data = game_state_data_new(
					"glow particles",
					MAXIMUM_GLOW_PARTICLES_PER_MAP,
					sizeof(struct glow_particle));

				if (glow_globals.glow_particle_data)
				{
					success = TRUE;
				}
				else
				{
					error(_error_silent, "could not allocate glow particle data array");
				}
			}
		}
		else
		{
			error(_error_silent, "could not allocate glow data array");
		}
	}

	if (!success)
	{
		glow_dispose();
	}

	return;
}

void glow_dispose(
	void)
{
	return;
}

void glow_initialize_for_new_map(
	void)
{
	if (glow_globals.glow_data)
	{
		data_make_valid(glow_globals.glow_data);
	}

	if (glow_globals.glow_particle_data)
	{
		data_make_valid(glow_globals.glow_particle_data);
	}

	return;
}

void glow_dispose_from_old_map(
	void)
{
	if (glow_globals.glow_data)
	{
		data_make_invalid(glow_globals.glow_data);
	}

	if (glow_globals.glow_particle_data)
	{
		data_make_invalid(glow_globals.glow_particle_data);
	}

	return;
}

long glow_new(
	long glow_definition_index)
{
	long glow_index = NONE;

	if (glow_definition_index != NONE)
	{
		glow_index = datum_new(glow_globals.glow_data);

		if (glow_index != NONE)
		{
			struct glow_datum *glow = glow_get(glow_index);
			struct glow_definition *definition = glow_definition_get(glow_definition_index);
			struct bitmap_group *bitmap_group = bitmap_group_get(definition->texture.index);

			if (bitmap_group->type == _bitmap_group_type_sprites)
			{
				struct bitmap_group_sprite *sprite = TAG_BLOCK_GET_ELEMENT(
					&TAG_BLOCK_GET_ELEMENT(&bitmap_group->sequences, 0, struct bitmap_group_sequence)->sprites,
					0,
					struct bitmap_group_sprite);
				struct bitmap_data *bitmap = bitmap_group_get_bitmap_from_sequence(
					definition->texture.index,
					0,
					sprite->bitmap_index);

				glow->definition_index = glow_definition_index;
				glow->number_of_particles = definition->number_of_particles;
				glow->bitmap_dimension = (short)((sprite->bounds.x1 - sprite->bounds.x0) * bitmap->width);
			}
		}
	}

	return glow_index;
}

void glow_delete(
	long glow_index)
{
	struct glow_datum *glow = glow_get(glow_index);
	struct glow_particle *particle = glow->head_particle;

	while (particle)
	{
		struct glow_particle *next = particle->next;

		datum_delete(glow_globals.glow_particle_data, particle->index);
		particle = next;
	}

	datum_delete(glow_globals.glow_data, glow_index);

	return;
}

void glow_submit(
	long object_index,
	long widget_index,
	struct render_lighting const *lighting,
	struct render_animation const *animation)
{
	if (object_index != NONE && widget_index != NONE)
	{
		struct glow_definition *definition = glow_definition_get(glow_get(widget_index)->definition_index);
		struct object_marker marker;
		short marker_count;

		glow_update(glow_get(widget_index), object_index);
		marker_count = object_get_marker_by_name(object_index, definition->attachment_marker, &marker, 1);
		glow_render(object_index, widget_index);
	}

	return;
}

static void glow_update(
	struct glow_datum *glow,
	long object_index)
{
	struct glow_definition *definition = glow_definition_get(glow->definition_index);

	if (definition)
	{
		glow->number_of_markers = object_get_marker_by_name(
			object_index,
			definition->attachment_marker,
			glow->markers,
			MAXIMUM_MARKERS_PER_GLOW_EFFECT);

		if (!glow->initialized && glow->number_of_markers > 1)
		{
			short marker_index;
			short other_marker_index;
			short next_marker[MAXIMUM_MARKERS_PER_GLOW_EFFECT];

			for (marker_index = 0; marker_index < glow->number_of_markers; marker_index++)
			{
				short best_marker_index = NONE;
				real best_dot_product = 0.f;

				for (other_marker_index = 0; other_marker_index < glow->number_of_markers; other_marker_index++)
				{
					if (marker_index != other_marker_index)
					{
						struct object_marker *marker = &glow->markers[marker_index];
						struct object_marker *other_marker = &glow->markers[other_marker_index];
						real_vector3d forward;
						real dot_product;

						vector_from_points3d(&marker->matrix.position, &other_marker->matrix.position, &forward);
						normalize3d(&forward);
						dot_product = dot_product3d(&forward, &marker->matrix.forward);

						if (dot_product > best_dot_product)
						{
							best_marker_index = other_marker_index;
							best_dot_product = dot_product;
						}
					}
				}

				next_marker[marker_index] = best_marker_index;
			}

			{
				short order_index = glow->number_of_markers;

				for (other_marker_index = NONE; --order_index >= 0; other_marker_index = marker_index)
				{
					marker_index = glow->number_of_markers;

					while (--marker_index >= 0)
					{
						if (next_marker[marker_index] == other_marker_index)
						{
							glow->marker_order[order_index] = marker_index;
							break;
						}
					}
				}
			}

			glow->total_time = 0.f;
			glow->marker_time_index[0] = 0.f;

			{
				short segment_index;

				for (segment_index = 0; segment_index < glow->number_of_markers - 1; segment_index++)
				{
					short marker0_index = glow->marker_order[segment_index];
					short marker1_index = glow->marker_order[segment_index + 1];
					struct object_marker *marker0 = &glow->markers[marker0_index];
					struct object_marker *marker1 = &glow->markers[marker1_index];
					real_point3d p0 = marker0->matrix.position;
					real_point3d p1 = marker1->matrix.position;
					real_vector3d segment;
					real segment_length;

					vector_from_points3d(&p0, &p1, &segment);
					segment_length = magnitude3d(&segment);
					glow->total_time += segment_length;
					glow->marker_time_index[segment_index + 1] = glow->total_time;
				}
			}

			glow_particles_initialize(glow);
			glow->accumulated_trailing_particle_generation_ticks = 0;
			glow->initialized = TRUE;
		}
		else
		{
			real effect_translational_velocity;
			real rotation_per_unit_t;

			if (glow->number_of_markers > 1)
			{
				real effect_rotational_velocity = definition->glow_effect_rotational_velocity;

				if (definition->effect_rotational_velocity_attachment_index != NONE)
				{
					real magnitude;
					real scale;

					if (!object_get_function_value(object_index, definition->effect_rotational_velocity_attachment_index, &magnitude))
					{
						magnitude = 0.f;
					}

					scale = (definition->glow_effect_rotational_velocity_scale_upper_bound - definition->glow_effect_rotational_velocity_scale_lower_bound) * magnitude +
						definition->glow_effect_rotational_velocity_scale_lower_bound;
					effect_rotational_velocity *= scale;
				}

				effect_translational_velocity = definition->glow_effect_translational_velocity;

				if (definition->effect_translational_velocity_attachment_index != NONE)
				{
					real magnitude;
					real scale;

					if (!object_get_function_value(object_index, definition->effect_translational_velocity_attachment_index, &magnitude))
					{
						magnitude = 0.f;
					}

					scale = (definition->glow_effect_translational_velocity_scale_upper_bound - definition->glow_effect_translational_velocity_scale_lower_bound) * magnitude +
						definition->glow_effect_translational_velocity_scale_lower_bound;
					effect_translational_velocity *= scale;
				}

				rotation_per_unit_t = effect_rotational_velocity / effect_translational_velocity;
			}

			glow->accumulated_trailing_particle_generation_ticks += game_time_get();

			{
				struct glow_particle *particle;

				if (glow->number_of_markers > 1)
				{
					for (particle = glow->head_particle; particle; particle = particle->next)
					{
						if (!TEST_FLAG(particle->flags, _glow_particle_is_trailing_particle_bit))
						{
							glow_normal_particle_update_position(
								object_index,
								glow,
								particle,
								render.time_delta_since_tick_sec * effect_translational_velocity,
								rotation_per_unit_t);
							glow_normal_particle_update_color(object_index, glow, particle);
							glow_normal_particle_update_size(object_index, glow, particle);
						}
					}
				}

				for (particle = glow->head_particle; particle; particle = particle->next)
				{
					if (TEST_FLAG(particle->flags, _glow_particle_is_trailing_particle_bit))
					{
						particle->ticks_in_existence += game_time_get();
						glow_trailing_particle_update_color(glow, particle);
						glow_trailing_particle_update_size(glow, particle);
						glow_trailing_particle_update_velocity(glow, particle);
						glow_trailing_particle_update_position(glow, particle, render.time_delta_since_tick_sec);
						glow_trailing_particle_age(glow, particle);
					}
				}
			}

			if (definition->trailing_particle_generation_frequency > MINIMUM_TRAILING_PARTICLE_GENERATION_FREQUENCY)
			{
				real ticks_per_particle = TICKS_PER_SECOND / definition->trailing_particle_generation_frequency;

				if (ticks_per_particle < 1.f)
				{
					ticks_per_particle = 1.f;
				}

				while (glow->accumulated_trailing_particle_generation_ticks > ticks_per_particle)
				{
					struct glow_particle *particle = glow_trailing_particle_new(glow);

					if (particle)
					{
						glow->number_of_particles++;

						if (glow->tail_particle)
						{
							glow->tail_particle->next = particle;
							particle->previous = glow->tail_particle;
							glow->tail_particle = particle;
						}
						else
						{
							glow->tail_particle = particle;
							glow->head_particle = particle;
						}

						glow->accumulated_trailing_particle_generation_ticks -= (short)ticks_per_particle;
					}
					else
					{
						match_vhalt("c:\\halo\\SOURCE\\objects\\widgets\\glow.c", 521, "the map limit for the number of active glow particles has been reached");
						break;
					}
				}
			}
		}
	}

	return;
}

static void glow_trailing_particle_update_color(
	struct glow_datum *glow,
	struct glow_particle *particle)
{
	struct glow_definition *definition = glow_definition_get(glow->definition_index);

	if (TEST_FLAG(definition->flags, _glow_trailing_particles_fade_over_time_bit))
	{
		particle->fade = 1.f - (real)particle->ticks_in_existence / particle->lifetime;
		particle->fade = PIN(particle->fade, 0.f, 1.f);
	}
	else
	{
		particle->fade = 1.f;
	}

	return;
}

static void glow_trailing_particle_update_size(
	struct glow_datum *glow,
	struct glow_particle *particle)
{
	struct glow_definition *definition = glow_definition_get(glow->definition_index);

	if (TEST_FLAG(definition->flags, _glow_trailing_particles_shrink_over_time_bit))
	{
		real scale = 1.f - (real)particle->ticks_in_existence / particle->lifetime;

		scale = MAX(0.f, scale);
		particle->present_size = scale * particle->initial_size;
	}

	return;
}

static void glow_trailing_particle_update_velocity(
	struct glow_datum *glow,
	struct glow_particle *particle)
{
	struct glow_definition *definition = glow_definition_get(glow->definition_index);

	if (TEST_FLAG(definition->flags, _glow_trailing_particles_slow_over_time_bit))
	{
		real scale = 1.f - (real)particle->ticks_in_existence / particle->lifetime;

		scale = MAX(0.f, scale);
		particle->present_velocity.i = scale * particle->initial_velocity.i;
		particle->present_velocity.j = scale * particle->initial_velocity.j;
		particle->present_velocity.k = scale * particle->initial_velocity.k;
	}
	else
	{
		particle->present_velocity = particle->initial_velocity;
	}

	return;
}

static void glow_trailing_particle_update_position(
	struct glow_datum *glow,
	struct glow_particle *particle,
	real elapsed_time)
{
	struct glow_definition *definition = glow_definition_get(glow->definition_index);

	particle->position.x += elapsed_time * particle->present_velocity.i;
	particle->position.y += elapsed_time * particle->present_velocity.j;
	particle->position.z += elapsed_time * particle->present_velocity.k;

	return;
}

static void glow_trailing_particle_age(
	struct glow_datum *glow,
	struct glow_particle *particle)
{
	struct glow_definition *definition = glow_definition_get(glow->definition_index);

	if (particle->ticks_in_existence > particle->lifetime)
	{
		struct glow_particle *previous = particle->previous;
		struct glow_particle *next = particle->next;

		if (previous)
		{
			previous->next = next;
		}
		else
		{
			glow->head_particle = next;
		}

		if (next)
		{
			next->previous = previous;
		}
		else
		{
			glow->tail_particle = previous;
		}

		datum_delete(glow_globals.glow_particle_data, particle->index);
		glow->number_of_particles--;
	}

	return;
}

static void glow_normal_particle_update_color(
	long object_index,
	struct glow_datum *glow,
	struct glow_particle *particle)
{
	struct glow_definition *definition = glow_definition_get(glow->definition_index);

	if (definition->color_attachment_index != NONE)
	{
		real magnitude;

		if (!object_get_function_value(object_index, definition->color_attachment_index, &magnitude))
		{
			magnitude = 0.f;
		}

		particle->color.red = (definition->color_bound_1.red - definition->color_bound_0.red) * magnitude + definition->color_bound_0.red;
		particle->color.green = (definition->color_bound_1.green - definition->color_bound_0.green) * magnitude + definition->color_bound_0.green;
		particle->color.blue = (definition->color_bound_1.blue - definition->color_bound_0.blue) * magnitude + definition->color_bound_0.blue;
		particle->color.alpha = 1.f;
	}

	if (TEST_FLAG(definition->flags, _glow_interpolate_color_per_particle_bit))
	{
		particle->color.red = particle->t * definition->color_rate_of_change * (definition->color_bound_1.red - definition->color_bound_0.red) + definition->color_bound_0.red;
		particle->color.green = particle->t * definition->color_rate_of_change * (definition->color_bound_1.green - definition->color_bound_0.green) + definition->color_bound_0.green;
		particle->color.blue = particle->t * definition->color_rate_of_change * (definition->color_bound_1.blue - definition->color_bound_0.blue) + definition->color_bound_0.blue;
		particle->color.alpha = 1.f;
	}

	{
		real normalized_t = particle->t / glow->total_time;
		real edge_fade = definition->percentage_edge_fade * 0.5f;

		if (normalized_t < edge_fade)
		{
			particle->fade = normalized_t / edge_fade;
		}
		else if (normalized_t > 1.f - edge_fade)
		{
			particle->fade = (1.f - normalized_t) / edge_fade;
		}
		else
		{
			particle->fade = 1.f;
		}

		particle->fade = PIN(particle->fade, 0.f, 1.f);
	}

	return;
}

static void glow_normal_particle_update_size(
	long object_index,
	struct glow_datum *glow,
	struct glow_particle *particle)
{
	particle->present_size = particle->initial_size;

	return;
}

static void glow_normal_particle_update_position(
	long object_index,
	struct glow_datum *glow,
	struct glow_particle *particle,
	real elapsed_time,
	real rotation_per_unit_t)
{
	struct glow_definition *definition = glow_definition_get(glow->definition_index);
	real magnitude;

	if (definition->distance_to_object_attachment_index != NONE)
	{
		real scale;

		if (!object_get_function_value(object_index, definition->distance_to_object_attachment_index, &magnitude))
		{
			magnitude = 0.f;
		}

		scale = (definition->distance_to_object_scale_upper_bound - definition->distance_to_object_scale_lower_bound) * magnitude +
			definition->distance_to_object_scale_lower_bound;
		particle->distance_to_object = (definition->maximum_distance_glow_particle_to_object - definition->minimum_distance_glow_particle_to_object) * scale +
			definition->minimum_distance_glow_particle_to_object;
	}

	if (TEST_FLAG(particle->flags, _glow_particle_traversal_direction_negative_bit))
	{
		particle->t -= elapsed_time;

		switch (definition->boundary_effect)
		{
		case _glow_effect_boundary_type_bounce:
			if (particle->t < 0.f)
			{
				while (particle->t < 0.f)
				{
					particle->t += glow->total_time;
				}

				particle->t = glow->total_time - particle->t;
				SET_FLAG(particle->flags, _glow_particle_traversal_direction_negative_bit, FALSE);
			}
			break;
		case _glow_effect_boundary_type_wrap:
			while (particle->t < 0.f)
			{
				particle->t += glow->total_time;
			}
			break;
		default:
			match_vhalt("c:\\halo\\SOURCE\\objects\\widgets\\glow.c", 800, "glow effect received illegal boundary effect");
		}
	}
	else
	{
		particle->t += elapsed_time;

		switch (definition->boundary_effect)
		{
		case _glow_effect_boundary_type_bounce:
			if (particle->t > glow->total_time)
			{
				while (particle->t > glow->total_time)
				{
					particle->t -= glow->total_time;
				}

				particle->t = glow->total_time - particle->t;
				SET_FLAG(particle->flags, _glow_particle_traversal_direction_negative_bit, TRUE);
			}
			break;
		case _glow_effect_boundary_type_wrap:
			while (particle->t > glow->total_time)
			{
				particle->t -= glow->total_time;
			}
			break;
		default:
			match_vhalt("c:\\halo\\SOURCE\\objects\\widgets\\glow.c", 829, "glow effect received illegal boundary effect");
		}
	}

	get_particle_world_position(glow, particle, rotation_per_unit_t);

	return;
}

static void glow_particles_initialize(
	struct glow_datum *glow)
{
	struct glow_definition *definition = glow_definition_get(glow->definition_index);
	short particle_index;
	boolean direction = TRUE;
	struct glow_particle *particle;
	struct glow_particle *previous_particle = NULL;

	for (particle_index = 0; particle_index < glow->number_of_particles; particle_index++)
	{
		particle = glow_normal_particle_new(glow, particle_index, glow->number_of_particles);

		if (!particle)
		{
			break;
		}

		if (TEST_FLAG(definition->flags, _glow_particles_initial_direction_negative_bit))
		{
			SET_FLAG(particle->flags, _glow_particle_traversal_direction_negative_bit, TRUE);
		}

		if (TEST_FLAG(definition->flags, _glow_particles_move_in_both_directions_bit))
		{
			SET_FLAG(particle->flags, _glow_particle_traversal_direction_negative_bit, !direction);
			direction = !direction;
		}

		if (!glow->head_particle)
		{
			glow->head_particle = particle;
		}

		if (previous_particle)
		{
			previous_particle->next = particle;
		}

		particle->previous = previous_particle;
		glow->tail_particle = particle;
		previous_particle = particle;
	}

	return;
}

static struct glow_particle *glow_normal_particle_new(
	struct glow_datum *glow,
	short index,
	short total)
{
	struct glow_definition *definition = glow_definition_get(glow->definition_index);
	struct glow_particle *particle = NULL;

	particle = glow_particle_new(glow);

	if (particle)
	{
		if (definition->distance_to_object_attachment_index == NONE)
		{
			particle->distance_to_object = real_local_random_range(
				definition->minimum_distance_glow_particle_to_object,
				definition->maximum_distance_glow_particle_to_object);
		}

		if (definition->particle_size_attachment_index == NONE)
		{
			particle->initial_size = real_local_random_range(
				definition->particle_size_lower_bound,
				definition->particle_size_upper_bound);
			particle->initial_size = particle->initial_size / glow->bitmap_dimension;
		}

		if (definition->color_attachment_index == NONE && !TEST_FLAG(definition->flags, _glow_interpolate_color_per_particle_bit))
		{
			real color_random = real_local_random_range(0.f, 1.f);

			particle->color.alpha = 1.f;
			particle->color.red = (definition->color_bound_1.red - definition->color_bound_0.red) * color_random + definition->color_bound_0.red;
			particle->color.green = (definition->color_bound_1.green - definition->color_bound_0.green) * color_random + definition->color_bound_0.green;
			particle->color.blue = (definition->color_bound_1.blue - definition->color_bound_0.blue) * color_random + definition->color_bound_0.blue;
		}

		switch (definition->normal_particle_distribution)
		{
		case _glow_particle_distribution_type_random:
			particle->t = real_local_random_range(0.f, glow->total_time);
			particle->initial_angle = real_local_random_range(0.f, 2.f * _pi);
			break;
		case _glow_particle_distribution_type_uniform:
		{
			real normalized_t = (real)index / total;

			particle->t = normalized_t * glow->total_time;
			particle->initial_angle = real_local_random_range(0.f, 2.f * _pi);
			break;
		}
		default:
			match_halt("c:\\halo\\SOURCE\\objects\\widgets\\glow.c", 945);
		}
	}

	return particle;
}

static struct glow_particle *glow_trailing_particle_new(
	struct glow_datum *glow)
{
	struct glow_particle *particle = NULL;
	struct glow_definition *definition = glow_definition_get(glow->definition_index);

	particle = glow_particle_new(glow);

	if (particle)
	{
		real scale;

		if (glow->number_of_markers > 1)
		{
			particle->t = real_local_random_range(
				definition->trailing_particle_minimum_normalized_t * glow->total_time,
				definition->trailing_particle_maximum_normalized_t * glow->total_time);
			get_particle_world_position(glow, particle, 0.f);
		}
		else
		{
			particle->position = glow->markers[0].matrix.position;
		}

		switch (definition->trailing_particle_distribution)
		{
		case _glow_trailing_particle_emission_vertical:
			particle->initial_velocity.i = 0.f;
			particle->initial_velocity.j = 0.f;
			particle->initial_velocity.k = 1.f;
			break;
		case _glow_trailing_particle_emission_normal:
			particle->initial_velocity = glow->markers[particle->parent_marker_index].matrix.up;
			break;
		case _glow_trailing_particle_emission_random:
			particle->initial_velocity.i = real_local_random_range(-1.f, 1.f);
			particle->initial_velocity.j = real_local_random_range(-1.f, 1.f);
			particle->initial_velocity.k = real_local_random_range(-1.f, 1.f);
			normalize3d(&particle->initial_velocity);
			break;
		default:
			match_vhalt("c:\\halo\\SOURCE\\objects\\widgets\\glow.c", 996, "unknown trailing particle distribution?");
		}

		scale = definition->trailing_particle_velocity / TICKS_PER_SECOND;
		particle->initial_velocity.i *= scale;
		particle->initial_velocity.j *= scale;
		particle->initial_velocity.k *= scale;
		particle->initial_size = real_local_random_range(
			definition->particle_size_lower_bound,
			definition->particle_size_upper_bound);
		particle->initial_size = particle->initial_size / glow->bitmap_dimension;
		particle->lifetime = (short)(definition->trailing_particle_lifetime * TICKS_PER_SECOND);
		scale = real_local_random_range(0.f, 1.f);
		particle->color.alpha = 1.f;
		particle->color.red = (definition->color_bound_1.red - definition->color_bound_0.red) * scale + definition->color_bound_0.red;
		particle->color.green = (definition->color_bound_1.green - definition->color_bound_0.green) * scale + definition->color_bound_0.green;
		particle->color.blue = (definition->color_bound_1.blue - definition->color_bound_0.blue) * scale + definition->color_bound_0.blue;
		SET_FLAG(particle->flags, _glow_particle_is_trailing_particle_bit, TRUE);
	}

	return particle;
}

static struct glow_particle *glow_particle_new(
	struct glow_datum *glow)
{
	struct glow_particle *particle = NULL;
	long particle_index = datum_new(glow_globals.glow_particle_data);

	if (particle_index != NONE)
	{
		particle = glow_particle_get(particle_index);
		particle->index = particle_index;
	}

	return particle;
}

__inline void point_from_parametric_line(
	real_point3d *p0,
	real_vector3d *forward,
	real t,
	real_point3d *result)
{
	result->x = t * forward->i + p0->x;
	result->y = t * forward->j + p0->y;
	result->z = t * forward->k + p0->y;

	return;
}

static void get_particle_world_position(
	struct glow_datum *glow,
	struct glow_particle *particle,
	real rotation_per_unit_t)
{
	real_point3d traversal_points[NUMBER_OF_TRAVERSAL_POINTS];
	real_vector3d traversal_up_vectors[4];
	real_vector3d traversal_left_vectors[4];
	real traversal_time_indices[4];
	real_vector3d forward;
	real_vector3d u;
	real_vector3d v;
	short marker_index;

	for (marker_index = 0; marker_index < glow->number_of_markers - 1; marker_index++)
	{
		if (glow->marker_time_index[marker_index] <= particle->t && glow->marker_time_index[marker_index + 1] > particle->t)
		{
			break;
		}
	}

	match_assert("c:\\halo\\SOURCE\\objects\\widgets\\glow.c", 1079, marker_index<glow->number_of_markers-1);
	particle->parent_marker_index = PIN(marker_index, 0, glow->number_of_markers - 1);
	match_assert("c:\\halo\\SOURCE\\objects\\widgets\\glow.c", 1083, glow->number_of_markers > 1);

	switch (glow->number_of_markers)
	{
	case 2:
		traversal_points[0] = glow->markers[0].matrix.position;
		traversal_points[3] = glow->markers[1].matrix.position;
		traversal_up_vectors[0] = glow->markers[0].matrix.up;
		traversal_up_vectors[3] = glow->markers[1].matrix.up;
		traversal_time_indices[0] = glow->marker_time_index[0];
		traversal_time_indices[3] = glow->marker_time_index[1];
		vector_from_points3d(&traversal_points[0], &traversal_points[3], &forward);
		point_from_parametric_line(&traversal_points[0], &forward, 0.25f, &traversal_points[1]);
		point_from_parametric_line(&traversal_points[0], &forward, 0.75f, &traversal_points[2]);
		vector_from_points3d(
			(real_point3d *)&traversal_up_vectors[0],
			(real_point3d *)&traversal_up_vectors[3],
			&forward);
		point_from_parametric_line(
			(real_point3d *)&traversal_up_vectors[0],
			&forward,
			0.25f,
			(real_point3d *)&traversal_up_vectors[1]);
		point_from_parametric_line(
			(real_point3d *)&traversal_up_vectors[0],
			&forward,
			0.75f,
			(real_point3d *)&traversal_up_vectors[2]);
		traversal_time_indices[1] = (traversal_time_indices[3] - traversal_time_indices[0]) * 0.25f + traversal_time_indices[0];
		traversal_time_indices[2] = (traversal_time_indices[3] - traversal_time_indices[0]) * 0.75f + traversal_time_indices[0];
		break;
	case 3:
		traversal_points[0] = glow->markers[0].matrix.position;
		traversal_points[3] = glow->markers[2].matrix.position;
		traversal_up_vectors[0] = glow->markers[0].matrix.up;
		traversal_up_vectors[3] = glow->markers[2].matrix.up;
		traversal_time_indices[0] = glow->marker_time_index[0];
		traversal_time_indices[3] = glow->marker_time_index[2];

		switch (particle->parent_marker_index)
		{
		case 0:
			traversal_points[1] = glow->markers[1].matrix.position;
			traversal_up_vectors[1] = glow->markers[1].matrix.up;
			traversal_time_indices[1] = glow->marker_time_index[1];
			vector_from_points3d(&traversal_points[1], &traversal_points[3], &forward);
			point_from_parametric_line(&traversal_points[1], &forward, 0.5f, &traversal_points[2]);
			vector_from_points3d(
				(real_point3d *)&traversal_up_vectors[1],
				(real_point3d *)&traversal_up_vectors[3],
				&forward);
			point_from_parametric_line(
				(real_point3d *)&traversal_up_vectors[1],
				&forward,
				0.5f,
				(real_point3d *)&traversal_up_vectors[2]);
			traversal_time_indices[2] = (traversal_time_indices[3] - traversal_time_indices[1]) * 0.5f + traversal_time_indices[1];
			break;
		case 1:
			traversal_points[2] = glow->markers[1].matrix.position;
			traversal_up_vectors[2] = glow->markers[1].matrix.up;
			traversal_time_indices[2] = glow->marker_time_index[1];
			vector_from_points3d(&traversal_points[0], &traversal_points[2], &forward);
			point_from_parametric_line(&traversal_points[0], &forward, 0.5f, &traversal_points[1]);
			vector_from_points3d(
				(real_point3d *)&traversal_up_vectors[0],
				(real_point3d *)&traversal_up_vectors[2],
				&forward);
			point_from_parametric_line(
				(real_point3d *)&traversal_up_vectors[0],
				&forward,
				0.5f,
				(real_point3d *)&traversal_up_vectors[1]);
			traversal_time_indices[2] = (traversal_time_indices[2] - traversal_time_indices[0]) * 0.5f + traversal_time_indices[0];
			break;
		}
		break;
	default:
		{
			short first_marker_index;
			short last_marker_index;
			short traversal_index;

			for (marker_index = 0; marker_index < glow->number_of_markers - 1; marker_index++)
			{
				if (glow->marker_time_index[marker_index] <= particle->t && particle->t <= glow->marker_time_index[marker_index + 1])
				{
					break;
				}
			}

			match_assert("c:\\halo\\SOURCE\\objects\\widgets\\glow.c", 1180, marker_index<glow->number_of_markers-1);
			marker_index = PIN(marker_index, 0, glow->number_of_markers - 1);
			first_marker_index = marker_index;
			last_marker_index = marker_index + 1;

			while (last_marker_index - first_marker_index + 1 < NUMBER_OF_TRAVERSAL_POINTS)
			{
				if (first_marker_index > 0)
				{
					first_marker_index--;
				}

				if (last_marker_index < glow->number_of_markers - 1)
				{
					last_marker_index++;
				}
			}

			for (traversal_index = 0; traversal_index < NUMBER_OF_TRAVERSAL_POINTS; traversal_index++)
			{
				short traversal_marker_index = glow->marker_order[traversal_index + first_marker_index];

				traversal_points[traversal_index] = glow->markers[traversal_marker_index].matrix.position;
				traversal_up_vectors[traversal_index] = glow->markers[traversal_marker_index].matrix.up;
				cross_product3d(
					&glow->markers[traversal_marker_index].matrix.up,
					&glow->markers[traversal_marker_index].matrix.forward,
					&traversal_left_vectors[traversal_index]);
				traversal_time_indices[traversal_index] = glow->marker_time_index[traversal_index + first_marker_index];
			}
		}
	}

	nonuniform_cubic_spline_vector3d(
		(real_vector3d *)&particle->position,
		(real_vector3d *)&traversal_points[0],
		(real_vector3d *)&traversal_points[1],
		(real_vector3d *)&traversal_points[2],
		(real_vector3d *)&traversal_points[3],
		traversal_time_indices[0],
		traversal_time_indices[1],
		traversal_time_indices[2],
		traversal_time_indices[3],
		particle->t);
	nonuniform_cubic_spline_vector3d(
		&u,
		&traversal_up_vectors[0],
		&traversal_up_vectors[1],
		&traversal_up_vectors[2],
		&traversal_up_vectors[3],
		traversal_time_indices[0],
		traversal_time_indices[1],
		traversal_time_indices[2],
		traversal_time_indices[3],
		particle->t);
	nonuniform_cubic_spline_vector3d(
		&v,
		&traversal_left_vectors[0],
		&traversal_left_vectors[1],
		&traversal_left_vectors[2],
		&traversal_left_vectors[3],
		traversal_time_indices[0],
		traversal_time_indices[1],
		traversal_time_indices[2],
		traversal_time_indices[3],
		particle->t);

	{
		real angle = particle->t * rotation_per_unit_t + particle->initial_angle;
		real sine_angle = sine(angle);
		real cosine_angle = cosine(angle);

		particle->position.x += (u.i * sine_angle + v.i * cosine_angle) * particle->distance_to_object;
		particle->position.y += (u.j * sine_angle + v.j * cosine_angle) * particle->distance_to_object;
		particle->position.z += (u.k * sine_angle + v.k * cosine_angle) * particle->distance_to_object;
	}

	return;
}

void glow_render(
	long object_index,
	long widget_index)
{
	struct glow_datum *glow = glow_get(widget_index);
	struct glow_definition *definition = glow_definition_get(glow->definition_index);
	struct build_sprite_data data;
	struct glow_particle *particle;

	build_sprites_begin(
		&data,
		glow->number_of_particles,
		definition->texture.index,
		&global_shader_effect_additive,
		0);

	for (particle = glow->head_particle; particle; particle = particle->next)
	{
		struct object_marker *marker = &glow->markers[particle->parent_marker_index];

		build_sprite(
			&data,
			_build_sprite_normal,
			0,
			0,
			&particle->position,
			&marker->matrix.forward,
			0.f,
			particle->present_size,
			&particle->color,
			particle->fade,
			0);
	}

	build_sprites_end(&data);

	return;
}
