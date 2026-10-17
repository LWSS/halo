/*
GLOW.H

header included in hcex build.
*/

#ifndef __GLOW_H
#define __GLOW_H
#pragma once

/* ---------- headers */

#include "glow_definitions.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/GLOW.C */

void glow_initialize(void);
void glow_dispose(void);
void glow_initialize_for_new_map(void);
void glow_dispose_from_old_map(void);
long glow_new(long glow_definition_index);
void glow_delete(long glow_index);
void glow_submit(long object_index, long widget_index, struct render_lighting const *lighting, struct render_animation const *animation);
void glow_render(long object_index, long widget_index);

/* ---------- globals */

/* ---------- public code */

#endif // __GLOW_H
