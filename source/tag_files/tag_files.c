/*
TAG_FILES.C
*/

/* ---------- headers */

#include "cseries.h"

/* ---------- public code */

// NOTE: 1300 lines of tool-specific code lives here

char const *tag_name_strip_path(
	char const *name)
{
	char const *stripped_name;

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
