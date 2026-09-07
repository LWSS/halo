/*
HUD.H

header included in hcex build.
*/

#ifndef __HUD_H
#define __HUD_H
#pragma once

#include "integer_math.h"
#include "real_math.h"

/* ---------- constants */

enum
{
	HUD_STACK_BUFFER_LONG_COUNT = 128,
	HUD_STACK_BUFFER_BYTE = 0x62,
};

/* ---------- macros */

/* ---------- structures */

struct hud_scripted_globals_definition
{
	boolean show_hud;
	boolean show_hud_help_text;
	byte pad[2];
};

/* ---------- prototypes/HUD_UNIT.C */

struct player_datum;

void unit_hud_shield_meter_mapper_init(void);
void hud_initialize_unit_interface(void);
void hud_initialize_unit_interface_for_new_map(void);
void hud_dispose_unit_interface_from_old_map(void);
void hud_dispose_unit_interface(void);
void scripted_hud_show_health(boolean show);
void scripted_hud_blink_health(boolean blink);
void scripted_hud_show_shield(boolean show);
void scripted_hud_blink_shield(boolean blink);
void scripted_hud_show_motion_sensor(boolean show);
void scripted_hud_blink_motion_sensor(boolean blink);
void hud_play_unit_sounds(struct player_datum *player, boolean show_hud);
void hud_fix_unit_data(short old_local_player_index, short new_local_player_index);
void hud_render_damage_indicators(short local_player_index);
void hud_tick_shield(long player_index, real amount);
void hud_update_unit(void);
void hud_render_unit_interface(struct player_datum *player);

/* ---------- prototypes/HUD_SOUNDS.C */

struct tag_block;
void hud_play_sound(short local_player_index, long type_flags, struct tag_block *sounds, long *sound_handles, unsigned short *sound_flags);

/* ---------- prototypes/HUD_DRAW.C */

struct bitmap_data;
struct hud_absolute_placement_definition;
struct hud_placement_definition;
struct static_hud_element_definition;
struct meter_hud_element_definition;
struct hud_color_definition;

long get_return_eip(void);
real hud_globals_get_scale(boolean in_multiplayer);
void hud_retrieve_bitmap_and_bounding_rect(long bitmap_group_index, short sequence_index, short frame_index, struct bitmap_data const **bitmap, real_rectangle2d const **clip);
void hud_draw_bitmap_direct(struct bitmap_data const *bitmap, short placement, point2d const *point, real_rectangle2d const *clip, real scale, real theta, unsigned long color, boolean is_interface_bitmap);
void hud_draw_static_element(short local_player_index, struct hud_absolute_placement_definition const *placement, struct static_hud_element_definition const *static_element, short draw_flags, long flash_reference_time);
void hud_draw_meter(short local_player_index, struct hud_absolute_placement_definition const *placement, struct meter_hud_element_definition const *meter, byte min_value, byte max_value, short draw_flags, real reference_time, real reference_value);
void hud_calculate_point(short local_player_index, struct hud_absolute_placement_definition const *absolute_placement, struct hud_placement_definition const *placement, struct bitmap_data const *bitmap, boolean in_multiplayer, real override_scale, point2d *result);
long get_flash_duration(struct hud_color_definition const *color);
unsigned long real_rgb_color_to_pixel32(real_rgb_color const *color);

/* ---------- globals */

extern struct hud_scripted_globals_definition *hud_scripted_globals;
extern struct hud_globals_definition *hud_globals;

/* ---------- public code */

__inline short check_stack_buffer(long const *buffer)
{
	short index;

	for (index = HUD_STACK_BUFFER_LONG_COUNT - 1; index >= 0; index--)
	{
		if (buffer[index] != 0x62626262)
			return index;
	}

	return NONE;
}

#endif // __HUD_H
