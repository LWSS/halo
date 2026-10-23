/*
LIGHT_VOLUMES.C
*/

/* ---------- headers */

#include "cseries.h"
#include "light_volumes.h"
#include "widgets.h"
#include "objects.h"
#include "collision_bsp.h"
#include "render.h"
#include "rasterizer.h"
#include "bitmaps_inlines.h"
#include "game_state.h"
#include "rasterizer_widgets.h"

/* ---------- constants */

enum
{
	MAXIMUM_LIGHT_VOLUMES_PER_MAP = 256,
};

/* ---------- macros */

#define light_volume_get(index) ((struct light_volume_datum *)datum_get(light_volume_globals.light_volume_data, (index))) /* fake name */
#define light_volume_try_and_get(index) ((struct light_volume_datum *)datum_try_and_get(light_volume_globals.light_volume_data, (index))) /* fake name */

/* ---------- structures */

struct light_volume_datum
{
	short identifier;
	word flags;
	long definition_index;
};

/* ---------- prototypes */

static struct light_volume_frame *light_volume_interpolate_frames(struct light_volume_definition const *definition, long object_index);
static real pow1(real z, real e);

/* ---------- globals */

static struct
{
	struct data_array *light_volume_data;
} light_volume_globals;

/* ---------- public code */

void light_volumes_initialize(
	void)
{
	light_volume_globals.light_volume_data = game_state_data_new("light volumes", MAXIMUM_LIGHT_VOLUMES_PER_MAP, sizeof(struct light_volume_datum));
	match_assert("c:\\halo\\SOURCE\\objects\\widgets\\light_volumes.c", 44, light_volume_globals.light_volume_data);

	return;
}

void light_volumes_dispose(
	void)
{
	return;
}

void light_volumes_initialize_for_new_map(
	void)
{
	if (light_volume_globals.light_volume_data)
	{
		data_make_valid(light_volume_globals.light_volume_data);
	}

	return;
}

void light_volumes_dispose_from_old_map(
	void)
{
	if (light_volume_globals.light_volume_data)
	{
		data_make_invalid(light_volume_globals.light_volume_data);
	}

	return;
}

long light_volume_new(
	long definition_index)
{
	long light_volume_index = datum_new(light_volume_globals.light_volume_data);

	if (light_volume_index != NONE)
	{
		struct light_volume_datum *light_volume = light_volume_get(light_volume_index);

		light_volume->definition_index = definition_index;
	}

	return light_volume_index;
}

void light_volume_delete(
	long light_volume_index)
{
	if (light_volume_index != NONE)
	{
		datum_delete(light_volume_globals.light_volume_data, light_volume_index);
	}

	return;
}

static struct light_volume_frame *light_volume_interpolate_frames(
	struct light_volume_definition const *definition,
	long object_index)
{
	static struct light_volume_frame frame_storage;
	struct light_volume_frame *frame = NULL;

	match_assert("c:\\halo\\SOURCE\\objects\\widgets\\light_volumes.c", 110, definition);

	frame = TAG_BLOCK_GET_ELEMENT(&definition->frames, 0, struct light_volume_frame);

	if (definition->frames.count > 1)
	{
		struct light_volume_frame *frame0 = TAG_BLOCK_GET_ELEMENT(&definition->frames, 0, struct light_volume_frame);
		struct light_volume_frame *frame1 = TAG_BLOCK_GET_ELEMENT(&definition->frames, 0, struct light_volume_frame);
		real q;

		if (object_get_function_value(object_index, definition->frame_animation_source - 1, &q))
		{
			frame = &frame_storage;
			frame->offset_from_marker = (1.f - q) * frame0->offset_from_marker + q * frame1->offset_from_marker;
			frame->offset_exponent = (1.f - q) * frame0->offset_exponent + q * frame1->offset_exponent;
			frame->length = (1.f - q) * frame0->length + q * frame1->length;
			frame->radius_hither = (1.f - q) * frame0->radius_hither + q * frame1->radius_hither;
			frame->radius_yon = (1.f - q) * frame0->radius_yon + q * frame1->radius_yon;
			frame->radius_exponent = (1.f - q) * frame0->radius_exponent + q * frame1->radius_exponent;
			frame->color_hither.alpha = (1.f - q) * frame0->color_hither.alpha + q * frame1->color_hither.alpha;
			frame->color_hither.red = (1.f - q) * frame0->color_hither.red + q * frame1->color_hither.red;
			frame->color_hither.green = (1.f - q) * frame0->color_hither.green + q * frame1->color_hither.green;
			frame->color_hither.blue = (1.f - q) * frame0->color_hither.blue + q * frame1->color_hither.blue;
			frame->color_yon.alpha = (1.f - q) * frame0->color_yon.alpha + q * frame1->color_yon.alpha;
			frame->color_yon.red = (1.f - q) * frame0->color_yon.red + q * frame1->color_yon.red;
			frame->color_yon.green = (1.f - q) * frame0->color_yon.green + q * frame1->color_yon.green;
			frame->color_yon.blue = (1.f - q) * frame0->color_yon.blue + q * frame1->color_yon.blue;
			frame->color_exponent = (1.f - q) * frame0->color_exponent + q * frame1->color_exponent;
			frame->brightness_exponent = (1.f - q) * frame0->brightness_exponent + q * frame1->brightness_exponent;
		}
	}
	else
	{
		frame = TAG_BLOCK_GET_ELEMENT(&definition->frames, 0, struct light_volume_frame);
	}

	return frame;
}

void light_volume_submit(
	long object_index,
	long widget_index,
	struct render_lighting const *lighting,
	struct render_animation const *animation)
{
	if (object_index != NONE && widget_index != NONE)
	{
		struct light_volume_datum *light_volume = light_volume_get(widget_index);
		struct light_volume_definition *definition = light_volume_definition_get(light_volume->definition_index);

		if (definition->count > 0 && definition->frames.count > 0 &&
			(!definition->brightness_scale_source || !animation || animation->values[definition->brightness_scale_source - 1] > 0.f))
		{
			struct object_marker marker;
			short marker_count;
			real_vector3d eye_to_marker_vector;

			marker_count = object_get_marker_by_name(object_index, definition->attachment_marker, &marker, 1);
			vector_from_points3d(&render.camera.position, &marker.matrix.position, &eye_to_marker_vector);

			if (definition->far_fade_distance == 0.f || dot_product3d(&render.camera.forward, &eye_to_marker_vector) < definition->far_fade_distance)
			{
				rasterizer_widget_submit(object_index, widget_index, &marker.matrix.position, light_volume_render);
			}
		}
	}

	return;
}

static real pow1(
	real z,
	real e)
{
	return e == 1.f ? z : pow(z, e);
}

void light_volume_render(
	long object_index,
	long widget_index)
{
	if (object_index != NONE && widget_index != NONE)
	{
		struct light_volume_datum *light_volume = light_volume_get(widget_index);
		struct light_volume_definition *definition = light_volume_definition_get(light_volume->definition_index);

		if (definition->count > 0 && definition->frames.count > 0)
		{
			struct light_volume_frame *frame = light_volume_interpolate_frames(definition, object_index);
			struct object_marker marker;
			short marker_count = object_get_marker_by_name(
				object_index,
				definition->attachment_marker,
				&marker,
				1);
			real_vector3d eye_to_marker_vector;
			real brightness = 1.f;

			vector_from_points3d(&render.camera.position, &marker.matrix.position, &eye_to_marker_vector);

			{
				real cosine_angle = ABS(dot_product3d(&render.camera.forward, &marker.matrix.forward));
				real external_scale = 1.f;

				if (definition->far_fade_distance > 0.f)
				{
					brightness *= PIN(
						(dot_product3d(&render.camera.forward, &eye_to_marker_vector) - definition->far_fade_distance) / (definition->near_fade_distance - definition->far_fade_distance),
						0.f,
						1.f);
				}

				brightness *= PIN(
					(1.f - cosine_angle) * definition->perpendicular_brightness_scale + cosine_angle * definition->parallel_brightness_scale,
					0.f,
					1.f);

				if (object_get_function_value(object_index, definition->brightness_scale_source - 1, &external_scale))
				{
					brightness *= external_scale;
				}
			}

			if (brightness > 0.f &&
				(frame->color_hither.alpha > 0.f || frame->color_yon.alpha > 0.f) &&
				(frame->radius_hither > 0.f || frame->radius_yon > 0.f))
			{
				short count;
				short index;

				rasterizer_widget_begin(_widget_type_internal_sprite, FLAG(_widget_internal_zbuffer_bit));
				rasterizer_widget_set_texture(0, definition->map.index, definition->sequence_index);
				count = definition->count;

				for (index = 0; index < count; index++)
				{
					real t = pow1((real)index / (count - 1), frame->offset_exponent);
					real_point3d position;
					real radius_t = pow1(t, frame->radius_exponent);
					real radius = (1.f - radius_t) * frame->radius_hither + frame->radius_yon * radius_t;
					real color_t = pow1(t, frame->color_exponent);
					real brightness_t = pow1(t, frame->brightness_exponent);
					real_argb_color color;

					point_from_line3d(
						&marker.matrix.position,
						&marker.matrix.forward,
						t * frame->length + frame->offset_from_marker,
						&position);
					rgb_colors_interpolate(
						&color.rgb,
						definition->flags & (FLAG(_light_volume_color_interpolate_in_hsv_bit) | FLAG(_light_volume_color_interpolate_along_farthest_hue_path_bit)),
						&frame->color_hither.rgb,
						&frame->color_yon.rgb,
						color_t);
					color.alpha = (1.f - brightness_t) * frame->color_hither.alpha + frame->color_yon.alpha * brightness_t;
					color.alpha *= brightness;
					rasterizer_widget_draw_sprite3d(
						&position,
						radius,
						NULL,
						0.f,
						real_argb_color_to_pixel32(&color));
				}

				rasterizer_widget_end();
			}
		}
	}

	return;
}
