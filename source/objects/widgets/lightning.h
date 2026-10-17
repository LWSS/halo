/*
LIGHTNING.H

header included in hcex build.
*/

#ifndef __LIGHTNING_H
#define __LIGHTNING_H
#pragma once

/* ---------- headers */

#include "lightning_definitions.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/LIGHTNING.C */

void lightnings_initialize(void);
void lightnings_dispose(void);
void lightnings_initialize_for_new_map(void);
void lightnings_dispose_from_old_map(void);
long lightning_new(long definition_index);
void lightning_delete(long lightning_index);
void lightning_submit(long object_index, long widget_index, struct render_lighting const *lighting, struct render_animation const *animation);
void lightning_render(long object_index, long widget_index);

/* ---------- globals */

/* ---------- public code */

#endif // __LIGHTNING_H
