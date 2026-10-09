/*
PROGRESS.C
*/

/* ---------- headers */

#include "cseries.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

void progress_new(
	struct progress *progress,
	struct progress_callbacks const *callbacks,
	char const *description,
	long maximum)
{
	memset(progress, 0, sizeof(*progress));

	if (callbacks && callbacks->update_proc)
	{
		progress->callbacks.update_proc = callbacks->update_proc;
		progress->callbacks.user_data = callbacks->user_data;

		if (description)
		{
			strncpy(progress->description, description, PROGRESS_STRING_LENGTH);
		}

		progress->maximum = maximum;
	}

	return;
}

void progress_update(
	struct progress *progress,
	char const *status,
	long current,
	boolean force)
{
	match_assert("c:\\halo\\SOURCE\\cseries\\progress.c", 35, progress);

	if (progress->callbacks.update_proc && progress->maximum)
	{
		unsigned long milliseconds = system_milliseconds();
		unsigned long elapsed = milliseconds - progress->last_milliseconds;

		if (elapsed > PROGRESS_UPDATE_FREQUENCY_IN_MILLISECONDS || force)
		{
			progress->callbacks.update_proc(progress->callbacks.user_data, progress->description, status, (current * 100) / progress->maximum);
			progress->last_milliseconds = milliseconds;
		}
	}

	return;
}

/* ---------- private code */
