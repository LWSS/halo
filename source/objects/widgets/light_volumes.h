/*
LIGHT_VOLUMES.H

header included in hcex build.
*/

#ifndef __LIGHT_VOLUMES_H
#define __LIGHT_VOLUMES_H
#pragma once

/* ---------- headers */

#include "light_volume_definitions.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/LIGHT_VOLUMES.C */

void light_volumes_initialize(void);
void light_volumes_dispose(void);
void light_volumes_initialize_for_new_map(void);
void light_volumes_dispose_from_old_map(void);
long light_volume_new(long definition_index);
void light_volume_delete(long light_volume_index);
void light_volume_submit(long object_index, long widget_index, struct render_lighting const *lighting, struct render_animation const *animation);
void light_volume_render(long object_index, long widget_index);

/* ---------- globals */

/* ---------- public code */

#endif // __LIGHT_VOLUMES_H
