/*
HUD_DEFINITIONS.H

header included in hcex build.
*/

#ifndef __HUD_DEFINITIONS_H
#define __HUD_DEFINITIONS_H
#pragma once

/* ---------- headers */

#include "real_math.h"
#include "tag_files.h"
#include "tag_groups.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct sound_hud_element_definition
{
	struct tag_reference sound;
	long type_flags;
	real scale;
	long unused[3];
	byte pad0;
	byte pad[3];
	long unused2[4];
};

/* ---------- prototypes/EXAMPLE.C */

/* ---------- globals */

/* ---------- public code */

#endif // __HUD_DEFINITIONS_H
