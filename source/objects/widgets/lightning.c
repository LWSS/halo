/*
LIGHTNING.C
*/

/* ---------- headers */

#include "cseries.h"
#include "lightning.h"
#include "widgets.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "rasterizer.h"
#include "bitmaps_inlines.h"
#include "game_state.h"
#include "texture_cache.h"

/* ---------- constants */

enum
{
	MAXIMUM_LIGHTNING_PER_MAP = 256,
	MAXIMUM_POINTS_PER_LIGHTNING = 4097,
};

/* ---------- macros */

#define lightning_get(index) ((struct lightning_datum *)datum_get(lightning_globals.lightning_data, (index)))	/* fake name */
#define lightning_try_and_get(index) ((struct lightning_datum *)datum_try_and_get(lightning_globals.lightning_data, (index)))	/* fake name */

/* ---------- structures */

struct lightning_datum
{
	short identifier;
	word flags;
	long definition_index;
};

struct intermediate_lightning_point
{
	real_point3d position;
	real width;
	real_argb_color color;
	boolean valid;
};

/* ---------- prototypes */

static void lightning_offset_marker_position(real_point3d *position, real_matrix4x3 const *matrix, real_vector3d const *random_position_bounds);

/* ---------- globals */

static struct
{
	struct data_array *lightning_data;
} lightning_globals;

/* ---------- public code */

void lightnings_initialize(
	void)
{
	lightning_globals.lightning_data = game_state_data_new(
		"lightnings",
		MAXIMUM_LIGHTNING_PER_MAP,
		sizeof(struct lightning_datum));
	match_assert("c:\\halo\\SOURCE\\objects\\widgets\\lightning.c", 50, lightning_globals.lightning_data);

	return;
}

void lightnings_dispose(
	void)
{
	return;
}

void lightnings_initialize_for_new_map(
	void)
{
	if (lightning_globals.lightning_data)
	{
		data_make_valid(lightning_globals.lightning_data);
	}

	return;
}

void lightnings_dispose_from_old_map(
	void)
{
	if (lightning_globals.lightning_data)
	{
		data_make_invalid(lightning_globals.lightning_data);
	}

	return;
}

long lightning_new(
	long definition_index)
{
	long lightning_index = datum_new(lightning_globals.lightning_data);

	if (lightning_index != NONE)
	{
		struct lightning_datum *lightning = lightning_get(lightning_index);

		lightning->definition_index = definition_index;
	}

	return lightning_index;
}

void lightning_delete(
	long lightning_index)
{
	if (lightning_index != NONE)
	{
		datum_delete(lightning_globals.lightning_data, lightning_index);
	}

	return;
}

static void lightning_offset_marker_position(
	real_point3d *position,
	real_matrix4x3 const *matrix,
	real_vector3d const *random_position_bounds)
{
	real_vector3d offset;

	match_assert("c:\\halo\\SOURCE\\objects\\widgets\\lightning.c", 116, position);
	match_assert("c:\\halo\\SOURCE\\objects\\widgets\\lightning.c", 117, matrix);
	match_assert("c:\\halo\\SOURCE\\objects\\widgets\\lightning.c", 118, random_position_bounds);

	set_real_vector3d(
		&offset,
		(real_local_random() * 2.f - 1.f) * random_position_bounds->i,
		(real_local_random() * 2.f - 1.f) * random_position_bounds->j,
		(real_local_random() * 2.f - 1.f) * random_position_bounds->k);
	matrix4x3_transform_vector(matrix, &offset, &offset);
	position->x += offset.i;
	position->y += offset.j;
	position->z += offset.k;

	return;
}

void lightning_submit(
	long object_index,
	long widget_index,
	struct render_lighting const *lighting,
	struct render_animation const *animation)
{
	if (object_index != NONE && widget_index != NONE)
	{
		struct lightning_datum *lightning = lightning_get(widget_index);
		struct lightning_definition *definition = lightning_definition_get(lightning->definition_index);
		struct object_marker marker;

		if (definition->markers.count > 0 &&
			object_get_marker_by_name(
				object_index,
				TAG_BLOCK_GET_ELEMENT(&definition->markers, 0, struct lightning_marker_definition)->attachment_marker,
				&marker,
				1) > 0)
		{
			struct bitmap_data *bitmap = TAG_BLOCK_GET_ELEMENT(
				&bitmap_group_get(definition->map.index)->bitmaps,
				0,
				struct bitmap_data);

			if (_texture_cache_bitmap_get_hardware_format(bitmap, FALSE, TRUE))
			{
				short lightning_index;

				for (lightning_index = 0; lightning_index < definition->count; lightning_index++)
				{
					struct intermediate_lightning_point points[MAXIMUM_POINTS_PER_LIGHTNING];
					short point_count = 0;
					real jitter_scale = 1.f;
					boolean new_segment = TRUE;
					short marker_index;

					if (animation &&
						animation->values &&
						definition->jitter_scale_source >= _object_function_reference_a &&
						definition->jitter_scale_source <= _object_function_reference_d)
					{
						jitter_scale *= animation->values[definition->jitter_scale_source - _object_function_reference_a];
					}

					for (marker_index = 0; marker_index < definition->markers.count; marker_index++)
					{
						struct lightning_marker_definition *marker_definition = TAG_BLOCK_GET_ELEMENT(
							&definition->markers,
							marker_index,
							struct lightning_marker_definition);

						if (new_segment)
						{
							point_count = 0;
							memset(points, 0, sizeof(points));
							object_get_marker_by_name(object_index, marker_definition->attachment_marker, &marker, 1);
							points[0].position = marker.matrix.position;
							lightning_offset_marker_position(
								&points[0].position,
								&marker.matrix,
								&marker_definition->random_position_bounds);
							points[0].width = marker_definition->thickness;
							points[0].color = marker_definition->tint;
							points[0].valid = TRUE;
							new_segment = FALSE;
						}

						if (TEST_FLAG(marker_definition->flags, _lightning_marker_not_connected_to_next_marker_bit) ||
							marker_index == definition->markers.count - 1)
						{
							rasterizer_globals.current_lock_operation = _rasterizer_lock_lightning;

							if (point_count > 2)
							{
								long dynamic_vertex_buffer_index;

								point_count++;
								dynamic_vertex_buffer_index = rasterizer_dynamic_vertices_new(
									_rasterizer_vertex_type_dynamic_unlit,
									2 * point_count);

								if (dynamic_vertex_buffer_index != NONE)
								{
									struct dynamic_unlit_vertex *vertices = rasterizer_dynamic_vertices_lock(dynamic_vertex_buffer_index);
									real_rectangle3d bounds;
									real_point3d centroid;
									struct shader_effect *effect;
									real one_over_point_count = 1.f / point_count;
									real texture_offset = real_local_random();
									real thickness_scale = 1.f;
									real_rgb_color const *tint = global_real_rgb_white;
									real brightness_scale = 1.f;
									short vertex_index;

									match_assert("c:\\halo\\SOURCE\\objects\\widgets\\lightning.c", 239, vertices);

									if (animation &&
										animation->values &&
										definition->thickness_scale_source >= _object_function_reference_a &&
										definition->thickness_scale_source <= _object_function_reference_d)
									{
										thickness_scale = animation->values[definition->thickness_scale_source - _object_function_reference_a];
									}

									if (animation &&
										animation->colors &&
										definition->tint_modulation_source >= _object_function_reference_a &&
										definition->tint_modulation_source <= _object_function_reference_d)
									{
										tint = &animation->colors[definition->tint_modulation_source - _object_function_reference_a];
									}

									if (animation &&
										animation->values &&
										definition->brightness_scale_source >= _object_function_reference_a &&
										definition->brightness_scale_source <= _object_function_reference_d)
									{
										brightness_scale = animation->values[definition->brightness_scale_source - _object_function_reference_a];
									}

									for (vertex_index = 0; vertex_index < point_count; vertex_index++)
									{
										struct intermediate_lightning_point *point = &points[vertex_index];
										struct intermediate_lightning_point *previous_point = vertex_index > 0 ?
											&points[vertex_index - 1] :
											point;
										struct intermediate_lightning_point *next_point = vertex_index < point_count - 1 ?
											&points[vertex_index + 1] :
											point;
										real_vector3d up;
										real width = points[vertex_index].width * thickness_scale;
										real_argb_color adjusted_color;
										pixel32 pixel;
										real u = vertex_index * one_over_point_count;

										match_assert("c:\\halo\\SOURCE\\objects\\widgets\\lightning.c", 280, points[vertex_index].valid);
										vector_from_points3d(&previous_point->position, &next_point->position, &up);
										cross_product3d(&up, &render.camera.forward, &up);
										fast_normalize3d(&up);
										adjusted_color.alpha = points[vertex_index].color.alpha * brightness_scale;
										adjusted_color.red = points[vertex_index].color.red * tint->red;
										adjusted_color.green = points[vertex_index].color.green * tint->green;
										adjusted_color.blue = points[vertex_index].color.blue * tint->blue;
										pixel = real_argb_color_to_pixel32(&adjusted_color);
										point_from_line3d(&point->position, &up, width, &vertices->position);
										vertices->color = pixel;
										vertices->texcoord.x = u + texture_offset;
										vertices->texcoord.y = 0.f;
										vertices++;
										vector_from_line3d(&point->position, &up, -width, &vertices->position);
										vertices->color = pixel;
										vertices->texcoord.x = u + texture_offset;
										vertices->texcoord.y = 1.f;
										vertices++;

										if (vertex_index == 0)
										{
											bounds.x1 = point->position.x;
											bounds.x0 = bounds.x1;
											bounds.y1 = point->position.y;
											bounds.y0 = bounds.y1;
											bounds.z1 = point->position.z;
											bounds.z0 = bounds.z1;
										}
										else
										{
											bounds.x0 = MIN(point->position.x, bounds.x0);
											bounds.y0 = MIN(point->position.y, bounds.y0);
											bounds.z0 = MIN(point->position.z, bounds.z0);
											bounds.x1 = MAX(point->position.x, bounds.x1);
											bounds.y1 = MAX(point->position.y, bounds.y1);
											bounds.z1 = MAX(point->position.z, bounds.z1);
										}
									}

									set_real_point3d(
										&centroid,
										(bounds.x0 + bounds.x1) * 0.5f,
										(bounds.y0 + bounds.y1) * 0.5f,
										(bounds.z0 + bounds.z1) * 0.5f);
									effect = definition->shaders.count > 0 ?
										TAG_BLOCK_GET_ELEMENT(&definition->shaders, 0, struct shader_effect) :
										&global_shader_effect_additive;
									rasterizer_dynamic_vertices_unlock(dynamic_vertex_buffer_index);
									rasterizer_dynamic_unlit_geometry_draw(
										(struct shader const *)effect,
										bitmap,
										animation,
										-point_count * 2,
										dynamic_vertex_buffer_index,
										2 * point_count - 2,
										&centroid,
										0);
									rasterizer_dynamic_vertices_delete(dynamic_vertex_buffer_index);
								}

								new_segment = TRUE;
							}

							rasterizer_globals.current_lock_operation = _rasterizer_lock_none;
						}
						else
						{
							struct lightning_marker_definition *next_marker_definition = TAG_BLOCK_GET_ELEMENT(
								&definition->markers,
								marker_index + 1,
								struct lightning_marker_definition);
							short segment_point_count = 1 << marker_definition->octaves_to_next_marker;
							short octave_count = marker_definition->octaves_to_next_marker;
							short octave;
							real const jitter_falloff_scale = 0.5f;
							real jitter_falloff = 1.f;
							real_vector3d up;

							match_assert("c:\\halo\\SOURCE\\objects\\widgets\\lightning.c", 383, !points[point_count+segment_point_count].valid);
							object_get_marker_by_name(object_index, next_marker_definition->attachment_marker, &marker, 1);
							points[point_count + segment_point_count].position = marker.matrix.position;
							lightning_offset_marker_position(
								&points[point_count + segment_point_count].position,
								&marker.matrix,
								&next_marker_definition->random_position_bounds);
							points[point_count + segment_point_count].width = next_marker_definition->thickness;
							points[point_count + segment_point_count].color = next_marker_definition->tint;
							points[point_count + segment_point_count].valid = TRUE;
							vector_from_points3d(
								&points[point_count].position,
								&points[point_count + segment_point_count].position,
								&up);
							cross_product3d(&up, &render.camera.forward, &up);

							if (normalize3d(&up) == 0.f)
							{
								up = *(real_vector3d const *)global_z_axis3d;
							}

							for (octave = 1; octave <= octave_count; octave++)
							{
								short segment_point_start_index = 1 << (octave_count - octave);
								short segment_point_step = 2 * segment_point_start_index;
								short segment_point_index;

								match_assert("c:\\halo\\SOURCE\\objects\\widgets\\lightning.c", 408, segment_point_start_index>0);

								for (segment_point_index = segment_point_start_index;
									segment_point_index < segment_point_count;
									segment_point_index += segment_point_step)
								{
									struct intermediate_lightning_point *previous_point = &points[point_count + segment_point_index - segment_point_start_index];
									struct intermediate_lightning_point *next_point = &points[point_count + segment_point_index + segment_point_start_index];
									struct intermediate_lightning_point *point = &points[point_count + segment_point_index];
									real_vector3d displacement;
									real t = (real)segment_point_index / (real)segment_point_count;
									real jitter = ((next_marker_definition->random_jitter_offset - marker_definition->random_jitter_offset) * t +
										marker_definition->random_jitter_offset) *
										jitter_falloff *
										jitter_scale;

									match_assert("c:\\halo\\SOURCE\\objects\\widgets\\lightning.c", 426, !points[point_count+segment_point_index].valid);
									scale_vector3d(&up, real_local_random_range(-1.f, 1.f) * jitter, &displacement);
									set_real_point3d(
										&point->position,
										(previous_point->position.x + next_point->position.x) * 0.5f + displacement.i,
										(previous_point->position.y + next_point->position.y) * 0.5f + displacement.j,
										(previous_point->position.z + next_point->position.z) * 0.5f + displacement.k);
									point->width = (previous_point->width + next_point->width) * 0.5f;
									point->color.alpha = (previous_point->color.alpha + next_point->color.alpha) * 0.5f;
									point->color.red = (previous_point->color.red + next_point->color.red) * 0.5f;
									point->color.green = (previous_point->color.green + next_point->color.green) * 0.5f;
									point->color.blue = (previous_point->color.blue + next_point->color.blue) * 0.5f;
									point->valid = TRUE;
								}

								jitter_falloff *= jitter_falloff_scale;
							}

							point_count += 1 << marker_definition->octaves_to_next_marker;
						}
					}
				}
			}
		}
	}

	return;
}

void lightning_render(
	long object_index,
	long widget_index)
{
	return;
}

/* ---------- private code */
