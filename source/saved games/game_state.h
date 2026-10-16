/*
GAME_STATE.H

header included in hcex build.
*/

#ifndef __GAME_STATE_H
#define __GAME_STATE_H
#pragma once

/* ---------- headers */

#include "lruv_cache.h"
#include "memory_pool.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

struct game_state_header
{
	unsigned long allocation_size_checksum;
	char map_name[MAXIMUM_FILENAME_LENGTH+1];
	char build_number[TAG_STRING_LENGTH+1];
	short player_count;
	short difficulty;
	unsigned long cache_file_checksum;
	long unused[7];
	unsigned long checksum;
};

/* ---------- prototypes/GAME_STATE.C */

void dummy(void);
void game_state_initialize(void);
void game_state_dispose(void);
void game_state_initialize_for_new_map(void);
void game_state_dispose_from_old_map(void);
void *game_state_malloc(char const *name, char const *type, long size);
void *game_state_gpu_malloc(char const *name, char const *type, long size);
struct data_array *game_state_data_new(char const *name, short maximum_count, short size);
struct memory_pool *game_state_memory_pool_new(char const *name, long size);
struct lruv_cache *game_state_lruv_cache_new(char const *name, long page_count, long page_size_bits, long maximum_block_count, void (*delete_block_proc)(long), boolean (*locked_block_proc)(long));
void game_state_save(void);
void game_state_revert(void);
void game_state_try_and_load_from_persistent_storage(void);
void game_state_save_to_persistent_storage(void);
boolean game_state_test_persistent_storage(char *map_name, short *difficulty, boolean *corrupted);
void game_state_save_core(char const *name);
void game_state_load_core(char const *name);
boolean game_state_reverted(void);

/* ---------- prototypes/GAME_STATE_XBOX.C */

void *game_state_allocate_buffer(unsigned long address, unsigned long cpu_size, unsigned long gpu_size);
void game_state_free_buffer(void);
void game_state_create_or_open_file(void);
void game_state_close_file(void);
boolean game_state_write_to_file(void);
boolean game_state_read_from_file(void);
boolean game_state_write_core(char const *name, void *buffer, long buffer_size);
boolean game_state_read_core_header(char const *name, void *header, long header_size);
void game_state_read_core(char const *name, void *buffer, long buffer_size);
boolean game_state_read_header_from_persistent_storage(void *header, unsigned long *header_checksum, long header_size, long buffer_size, boolean *corrupted);
void game_state_write_to_persistent_storage(void *buffer, unsigned long *header_checksum, long header_size, long buffer_size);
void game_state_read_from_persistent_storage(void *buffer, long buffer_size);
char const *game_state_get_persistent_storage_filename(void);
void game_state_create_persistent_storage(char const *path);

/* ---------- globals */

extern boolean recover_saved_games_hack;

/* ---------- public code */

#endif // __GAME_STATE_H
