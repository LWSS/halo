/*
TAG_FILES.C
*/

/* ---------- headers */

#include "cseries.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

// [NOTE: 1300 lines of tool-specific code lives here]

const char *tag_name_strip_path(
	const char *name)
{
	const char *stripped_name;

	match_assert("c:\\halo\\SOURCE\\tag_files\\tag_files.c", 1374, name);
	stripped_name = strrchr(name, '\\');

	if (stripped_name)
	{
		stripped_name++;
	}
	else
	{
		stripped_name = name;
	}

	return stripped_name;
}

/* ---------- private code */
