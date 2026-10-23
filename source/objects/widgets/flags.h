/*
FLAGS.H

header included in hcex build.
*/

#ifndef __FLAGS_H
#define __FLAGS_H
#pragma once

/* ---------- headers */

#include "flag_definitions.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/FLAGS.C */

void flags_initialize(void);
void flags_initialize_for_new_map(void);
void flags_dispose_from_old_map(void);
void flags_dispose(void);
long flag_new(long flag_definition_index);
void flag_delete(long flag_index);
void flag_render(long object_index, long flag_index, struct render_lighting const *lighting, struct render_animation const *animation);
void flags_update(real dt);

/* ---------- globals */

/* ---------- public code */

#endif // __FLAGS_H
