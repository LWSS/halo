/*
PLAYLIST_PROFILE.C
*/

/* ---------- headers */

#include "cseries.h"
#include "playlist_profile.h"
#include "player_profile.h"
#include "saved_game_files.h"
#include "cheats.h"
#include "game.h"
#include "network_game_globals.h"
#include "game_engine.h"
#include "input.h"
#include "text_group.h"
#include "ui_widget.h"
#include "unicode.h"
#include "bungie_net/common/thread.h"

/* ---------- constants */

enum
{
	NUMBER_OF_DEFAULT_VARIANTS = 26
};

/* ---------- macros */

/* ---------- structures */

struct playlist_profile_internal
{
	struct game_variant variant;
	XCALCSIG_SIGNATURE checksum;
};

union playlist_profile_block
{
	byte data[SAVED_GAME_FILE_BLOCK_SIZE];
	struct playlist_profile_internal profile_block;
};

struct playlist_profile_write_internal_input
{
	long index;
	struct game_variant variant;
};

struct _playlist_profile_globals
{
	struct playlist_profile_write_internal_input thread_input;
	struct thread *thread;
	word number_of_default_profiles;
	boolean initialized;
};

/* ---------- prototypes */

static void playlist_profile_create_default_profiles_on_disk(void);
static boolean playlist_profile_read(long index, struct game_variant *variant);
static void playlist_profile_write(long index, struct game_variant *variant);
static unsigned long __stdcall playlist_profile_write_thread_proc(void *input);

/* ---------- globals */

static struct game_variant (*default_variant_building_functions[NUMBER_OF_DEFAULT_VARIANTS])(void) =
{
	build_game_variant_slayer,
	build_game_variant_slayer_pro,
	build_game_variant_elimination,
	build_game_variant_phantoms,
	build_game_variant_endurance,
	build_game_variant_rockets,
	build_game_variant_snipers,
	build_game_variant_oddball,
	build_game_variant_reverse_tag,
	build_game_variant_accumulation,
	build_game_variant_juggernaut,
	build_game_variant_stalker,
	build_game_variant_king,
	build_game_variant_king_pro,
	build_game_variant_crazy_king,
	build_game_variant_race,
	build_game_variant_rally,
	build_game_variant_ctf,
	build_game_variant_invasion,
	build_game_variant_iron_ctf,
	build_game_variant_ctf_pro,
	build_game_variant_team_race,
	build_game_variant_team_rally,
	build_game_variant_team_oddball,
	build_game_variant_team_king,
	build_game_variant_team_slayer
};

static struct _playlist_profile_globals playlist_profile_globals;

/* ---------- public code */

void playlist_profiles_initialize(
	void)
{
	memset(&playlist_profile_globals, 0, sizeof(playlist_profile_globals));
	playlist_profile_globals.initialized = TRUE;

	return;
}

void playlist_profiles_dispose(
	void)
{
	if (playlist_profile_globals.thread)
	{
		error(_error_silent, "waiting for asynchronous playlist profile writes to finish...");

		while (!thread_has_exited(playlist_profile_globals.thread))
		{
		}

		dispose_thread(playlist_profile_globals.thread);
		playlist_profile_globals.thread = NULL;
	}

	memset(&playlist_profile_globals, 0, sizeof(playlist_profile_globals));

	return;
}

void playlist_profiles_enumerate_available_to_local_player_index(
	short local_player_index,
	word *number_of_profiles,
	long *playlist_profile_indices)
{
	static boolean first_time = TRUE;

	if (first_time == TRUE)
	{
		playlist_profile_create_default_profiles_on_disk();
		first_time = FALSE;
	}

	saved_game_files_enumerate_available_to_local_player_index(
		local_player_index,
		_saved_game_file_type_play_list,
		number_of_profiles,
		playlist_profile_indices,
		TRUE);

	return;
}

long playlist_profile_new(
	short local_player_index,
	wchar_t *name)
{
	struct file_reference playlist_file;
	struct game_variant slayer;
	long playlist_profile_index = create_enumerated_saved_game_file(_saved_game_file_type_play_list, local_player_index, name);

	if (playlist_profile_index != NONE)
	{
		if (saved_game_file_open(&playlist_file, playlist_profile_index))
		{
			union playlist_profile_block block = {0};

			slayer = build_game_variant_slayer();
			memcpy(&block.profile_block.variant, &slayer, sizeof(slayer));
			SET_FLAG(block.profile_block.variant.flags, _variant_is_system_default_bit, FALSE);
			game_engine_variant_cleanup(&block.profile_block.variant);
			ustrncpy(
				block.profile_block.variant.human_readable_game_description,
				name,
				MAXIMUM_GAME_VARIANT_NAME_LENGTH - 1);
			block.profile_block.variant.human_readable_game_description[MAXIMUM_GAME_VARIANT_NAME_LENGTH - 1] = '\0';
			saved_game_file_generate_checksum(
				&block.profile_block.variant,
				sizeof(block.profile_block.variant),
				&block.profile_block.checksum);

			if (!file_set_position(&playlist_file, 0) ||
				!file_write(&playlist_file, sizeof(block), &block))
			{
				error(_error_silent, "failed to initialize newly created playlist profile");
				delete_enumerated_saved_game_file(playlist_profile_index);
				playlist_profile_index = NONE;
			}

			saved_game_file_close(&playlist_file, playlist_profile_index);
		}
		else
		{
			error(_error_silent, "failed to open newly created playlist profile");
			delete_enumerated_saved_game_file(playlist_profile_index);
			playlist_profile_index = NONE;
		}
	}

	return playlist_profile_index;
}

void playlist_profile_delete(
	long playlist_profile_index)
{
	if (playlist_profile_index != NONE &&
		!delete_enumerated_saved_game_file(playlist_profile_index))
	{
		error(_error_silent, "playlist_profile_delete() failed (profile index= #0x%lX)", playlist_profile_index);
	}

	return;
}

boolean playlist_profile_get(
	long playlist_profile_index,
	struct game_variant *variant)
{
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\saved games\\playlist_profile.c", 217, variant);

	if (playlist_profile_index == NONE)
	{
		game_engine_playlist_next(0, 0, 4);
	}
	else
	{
		success = playlist_profile_read(playlist_profile_index, variant);
	}

	return success;
}

boolean playlist_profile_get_from_path(
	char const *full_path,
	struct game_variant *variant)
{
	union playlist_profile_block block;
	struct file_reference profile_file;
	XCALCSIG_SIGNATURE checksum;
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\saved games\\playlist_profile.c", 243, full_path && variant);

	if (file_reference_create_from_path(&profile_file, full_path, FALSE) &&
		file_open(&profile_file, FLAG(_permission_read_bit)))
	{
		if (file_read(&profile_file, sizeof(block), &block))
		{
			saved_game_file_generate_checksum(
				&block.profile_block.variant,
				sizeof(block.profile_block.variant),
				&checksum);

			if (!memcmp(&checksum, &block.profile_block.checksum, sizeof(checksum)))
			{
				memcpy(variant, &block.profile_block.variant, sizeof(*variant));
				success = TRUE;
			}
			else
			{
				error(_error_silent, "checksum failed on playlist profile file");
			}
		}
		else
		{
			error(_error_silent, "failed to read playlist profile");
		}

		file_close(&profile_file);
	}
	else
	{
		error(_error_silent, "failed to open playlist profile file");
	}

	return success;
}

boolean playlist_profile_get_display_name(
	long playlist_profile_index,
	wchar_t *display_name)
{
	boolean success;
	wchar_t *saved_game_display_name = saved_game_file_get_display_name(playlist_profile_index);

	if (saved_game_display_name)
	{
		ustrncpy(display_name, saved_game_display_name, MAXIMUM_SAVED_GAME_NAME_LENGTH - 1);
		display_name[MAXIMUM_SAVED_GAME_NAME_LENGTH - 1] = '\0';
		success = TRUE;
	}
	else
	{
		success = FALSE;
	}

	return success;
}

void playlist_profile_save(
	long playlist_profile_index,
	struct game_variant *variant)
{
	match_assert("c:\\halo\\SOURCE\\saved games\\playlist_profile.c", 305, variant);

	if (playlist_profile_index != NONE)
	{
		game_engine_variant_cleanup(variant);
		playlist_profile_write(playlist_profile_index, variant);
	}

	return;
}

word playlist_profile_number_of_default_profiles_on_disk(
	void)
{
	return playlist_profile_globals.number_of_default_profiles;
}

/* ---------- private code */

static void playlist_profile_create_default_profiles_on_disk(
	void)
{
	char full_path[MAXIMUM_FILENAME_LENGTH+1];
	union playlist_profile_block block;
	struct game_variant game_variant;
	struct file_reference file;
	long string_list_index = tag_loaded(UNICODE_STRING_LISTS_GROUP_TAG, DEFAULT_MULTIPLAYER_GAME_SETTING_NAMES_TAG_NAME);

	if (string_list_index != NONE)
	{
		long variant_index;

		for (variant_index = 0; variant_index < NUMBER_OF_DEFAULT_VARIANTS; variant_index++)
		{
			wchar_t *display_name;
			boolean written = FALSE;

			game_variant = default_variant_building_functions[variant_index]();
			_snprintf(full_path, MAXIMUM_FILENAME_LENGTH, DEFAULT_PLAYLIST_DIRECTORY_FORMAT, variant_index);
			full_path[MAXIMUM_FILENAME_LENGTH] = '\0';
			directory_create_or_delete_contents(full_path);
			strncat(full_path, "\\" PLAYLIST_PROFILE_FILE_NAME, MAXIMUM_FILENAME_LENGTH);
			full_path[MAXIMUM_FILENAME_LENGTH] = '\0';
			display_name = unicode_string_list_get_string(string_list_index, variant_index);
			memcpy(&block.profile_block.variant, &game_variant, sizeof(game_variant));
			ustrncpy(
				block.profile_block.variant.human_readable_game_description,
				display_name,
				MAXIMUM_GAME_VARIANT_NAME_LENGTH - 1);
			block.profile_block.variant.human_readable_game_description[MAXIMUM_GAME_VARIANT_NAME_LENGTH - 1] = '\0';
			block.profile_block.variant.flags |= variant_index << CHAR_BITS;
			saved_game_file_generate_checksum(
				&block.profile_block.variant,
				sizeof(block.profile_block.variant),
				&block.profile_block.checksum);

			if (file_reference_create_from_path(&file, full_path, FALSE) &&
				file_create(&file) &&
				file_open(&file, FLAG(_permission_write_bit)) &&
				file_set_position(&file, 0))
			{
				written = file_write(&file, sizeof(block), &block);
				file_close(&file);
			}

			if (written == TRUE)
			{
				playlist_profile_globals.number_of_default_profiles++;
			}
			else
			{
				error(_error_silent, "failed to create default playlist profile file '%s' on disk", full_path);
			}
		}

		saved_game_files_notify_memory_units_changed();
	}
	else
	{
		error(
			_error_silent,
			"failed to load localized default variant names string list tag; no default game variants enumerated");
	}

	return;
}

static boolean playlist_profile_read(
	long index,
	struct game_variant *variant)
{
	struct file_reference playlist_file;
	union playlist_profile_block block;
	XCALCSIG_SIGNATURE checksum;
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\saved games\\playlist_profile.c", 396, variant);

	if (playlist_profile_globals.thread)
	{
		error(_error_silent, "waiting for asynchronous playlist profile io to finish...");

		while (!thread_has_exited(playlist_profile_globals.thread))
		{
		}

		dispose_thread(playlist_profile_globals.thread);
		playlist_profile_globals.thread = NULL;
	}

	if (TEST_FLAG(index, _profile_checksum_valid_bit31))
	{
		if (saved_game_files_take_mutex())
		{
			if (saved_game_file_open(&playlist_file, index))
			{
				if (file_read(&playlist_file, sizeof(block), &block))
				{
					saved_game_file_generate_checksum(
						&block.profile_block.variant,
						sizeof(block.profile_block.variant),
						&checksum);

					if (!memcmp(&checksum, &block.profile_block.checksum, sizeof(checksum)))
					{
						memcpy(variant, &block.profile_block.variant, sizeof(*variant));
					}
					else
					{
						struct game_variant sanitized_variant = build_game_variant_slayer();

						error(_error_silent, "checksum failed on playlist profile file, sanitizing memory resident version...");
						sanitized_variant.flags = 0;
						ustrncpy(
							sanitized_variant.human_readable_game_description,
							saved_game_file_get_display_name(index),
							MAXIMUM_GAME_VARIANT_NAME_LENGTH - 1);
						sanitized_variant.human_readable_game_description[MAXIMUM_GAME_VARIANT_NAME_LENGTH - 1] = '\0';
						memcpy(variant, &sanitized_variant, sizeof(*variant));
					}

					success = TRUE;
				}
				else
				{
					error(_error_silent, "failed to read playlist profile from file");
				}

				saved_game_file_close(&playlist_file, index);
			}
			else
			{
				error(_error_silent, "failed to open playlist profile file");
			}

			saved_game_files_release_mutex();
		}
		else
		{
			error(_error_silent, "failed to get saved game files mutex; perhaps another operation is in progress?");
		}
	}
	else
	{
		struct game_variant sanitized_variant = build_game_variant_slayer();

		error(_error_silent, "checksum failed on playlist profile file, sanitizing memory resident version...");
		sanitized_variant.flags = 0;
		ustrncpy(
			sanitized_variant.human_readable_game_description,
			saved_game_file_get_display_name(index),
			MAXIMUM_GAME_VARIANT_NAME_LENGTH - 1);
		sanitized_variant.human_readable_game_description[MAXIMUM_GAME_VARIANT_NAME_LENGTH - 1] = '\0';
		memcpy(variant, &sanitized_variant, sizeof(*variant));
		success = TRUE;
	}

	return success;
}

static void playlist_profile_write(
	long index,
	struct game_variant *variant)
{
	match_assert("c:\\halo\\SOURCE\\saved games\\playlist_profile.c", 490, variant);

	if (playlist_profile_globals.thread)
	{
		error(_error_silent, "waiting for asynchronous playlist profile io to finish...");

		while (!thread_has_exited(playlist_profile_globals.thread))
		{
		}

		dispose_thread(playlist_profile_globals.thread);
		playlist_profile_globals.thread = NULL;
	}

	playlist_profile_globals.thread_input.index = index;
	memcpy(&playlist_profile_globals.thread_input.variant, variant, sizeof(*variant));
	create_thread(
		0,
		playlist_profile_write_thread_proc,
		&playlist_profile_globals.thread_input,
		&playlist_profile_globals.thread);

	return;
}

static unsigned long __stdcall playlist_profile_write_thread_proc(
	void *input)
{
	struct file_reference playlist_file;
	union playlist_profile_block block;

	match_assert("c:\\halo\\SOURCE\\saved games\\playlist_profile.c", 514, input);

	error(_error_silent, "begin playlist profile write");

	if (saved_game_files_take_mutex())
	{
		boolean failed = FALSE;
		struct playlist_profile_write_internal_input *thread_input = (struct playlist_profile_write_internal_input *)input;
		long index = thread_input->index;
		struct game_variant *variant = &thread_input->variant;

		if (saved_game_file_open(&playlist_file, index))
		{
			memcpy(&block.profile_block.variant, variant, sizeof(*variant));
			saved_game_file_generate_checksum(
				&block.profile_block.variant,
				sizeof(block.profile_block.variant),
				&block.profile_block.checksum);

			if (!file_set_position(&playlist_file, 0) ||
				!file_write(&playlist_file, sizeof(block), &block))
			{
				error(_error_silent, "failed to write playlist profile to file");
				failed = TRUE;
			}

			if (saved_game_file_close(&playlist_file, index) &&
				!synchronize_metadata_display_name_with_profile_name(index, variant->human_readable_game_description))
			{
				error(_error_silent, "metadata name may not match game display name");
			}
		}
		else
		{
			error(_error_silent, "failed to open playlist profile file");
		}

		if (failed)
		{
			delete_enumerated_saved_game_file(index);
		}

		saved_game_files_release_mutex();
	}
	else
	{
		error(_error_silent, "failed to get saved game files mutex; perhaps another operation is in progress?");
	}

	error(_error_silent, "end playlist profile write");

	return 0;
}
