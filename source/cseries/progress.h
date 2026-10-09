/*
PROGRESS.H

header included in hcex build.
*/

#ifndef __PROGRESS_H
#define __PROGRESS_H
#pragma once

/* ---------- constants */

enum
{
	PROGRESS_STRING_LENGTH = 255,
	PROGRESS_UPDATE_FREQUENCY_IN_MILLISECONDS = 125,
};

/* ---------- macros */

/* ---------- structures */

struct progress_callbacks
{
	void (*update_proc)(void *user_data, char const *description, char const *status, long percent);
	void *user_data;
};

struct progress /* fake name */
{
	struct progress_callbacks callbacks; /* fake name */
	char description[PROGRESS_STRING_LENGTH + 1];
	long maximum; /* fake name */
	unsigned long last_milliseconds;
};

/* ---------- prototypes/PROGRESS.C */

void progress_new(struct progress *progress, struct progress_callbacks const *callbacks, char const *description, long maximum);
void progress_update(struct progress *progress, char const *status, long current, boolean force);

/* ---------- globals */

/* ---------- public code */

#endif // __PROGRESS_H
