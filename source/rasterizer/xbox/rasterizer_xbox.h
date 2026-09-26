/*
RASTERIZER_XBOX.H
*/

#ifndef __RASTERIZER_XBOX_H
#define __RASTERIZER_XBOX_H
#pragma once

/* ---------- constants */

/* ---------- macros */

// original name unknown
#define D3DCALL(success, call) { HRESULT result = (call); (success) = (success) && SUCCEEDED(result); if (!(success)) rasterizer_error(result, #call); }

/* ---------- structures */

/* ---------- prototypes/RASTERIZER_XBOX.C */

void rasterizer_preinitialize__fill_you_up_with_the_devils_cock(void);


/* ---------- prototypes/RASTERIZER_XBOX_ERRORS.C */

void rasterizer_error(HRESULT hr, char const *format, ...);

/* ---------- prototypes/RASTERIZER_SWIZZLE.C */

void rasterizer_xbox_bitmap_swizzle2d_byte(void *destination, void const *source, short width, short height);
void rasterizer_xbox_bitmap_swizzle2d_word(void *destination, void const *source, short width, short height);
void rasterizer_xbox_bitmap_swizzle2d_long(void *destination, void const *source, short width, short height);
void rasterizer_xbox_bitmap_swizzle3d_byte(void *destination, void const *source, short width, short height, short depth);
void rasterizer_xbox_bitmap_swizzle3d_word(void *destination, void const *source, short width, short height, short depth);
void rasterizer_xbox_bitmap_swizzle3d_long(void *destination, void const *source, short width, short height, short depth);
short rasterizer_xbox_bitmap_get_max_mipmap_count(struct bitmap_data const *bitmap);

/* ---------- prototypes/RASTERIZER_XBOX_PROFILE.C */

void rasterizer_profile_begin(short profile);

void rasterizer_profile_end(short profile);

/* ---------- globals */

extern IDirect3DDevice8 *global_d3d_device;

/* ---------- public code */

#endif // __RASTERIZER_XBOX_H
