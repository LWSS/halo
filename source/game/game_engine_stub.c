/*
GAME_ENGINE_STUB.C
*/

/* ---------- headers */

#include "cseries.h"
#include "game_engine.h"
#include "game_engine_list.h"

/* ---------- prototypes */

static void stub_engine_dispose(void);
static boolean stub_engine_initialize_for_new_map(void);
static void stub_engine_dispose_from_old_map(void);
static void stub_engine_player_added(long player_index);
static void stub_engine_game_ending(void);
static void stub_engine_game_starting(void);
static void stub_engine_statistics_append(struct game_statistics *permanent_statistics, struct game_statistics *game_statistics);
static void stub_engine_handle_client_message(long player_index, void *encoded_message, short encoded_message_size);
static void stub_engine_handle_server_message(void *encoded_message, short encoded_message_size);
static void stub_engine_pregame_post_rasterize(void);
static void stub_engine_post_rasterize(void);
static void stub_engine_update(void);
static boolean stub_engine_allow_pick_up(long unit_index, long item_index);
static void stub_engine_player_damaged_player(long killing_player_index, long dead_player_index, boolean friendly_fire);
static void stub_engine_player_killed_player(long killing_player_index, long killing_object_index, long dead_player_index, boolean friendly_fire);

/* ---------- globals */

struct game_engine stub_engine =
{
	"stub",
	_game_engine_stub,
	stub_engine_dispose,
	stub_engine_initialize_for_new_map,
	stub_engine_dispose_from_old_map,
	stub_engine_player_added,
	stub_engine_game_ending,
	stub_engine_game_starting,
	stub_engine_statistics_append,
	stub_engine_handle_client_message,
	stub_engine_handle_server_message,
	stub_engine_pregame_post_rasterize,
	stub_engine_post_rasterize,
	NULL,
	NULL,
	NULL,
	NULL,
	stub_engine_update,
	NULL,
	NULL,
	NULL,
	NULL,
	stub_engine_allow_pick_up,
	stub_engine_player_damaged_player,
	stub_engine_player_killed_player,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};

/* ---------- private code */

static void stub_engine_dispose(
	void)
{
	return;
}

static boolean stub_engine_initialize_for_new_map(
	void)
{
	return TRUE;
}

static void stub_engine_dispose_from_old_map(
	void)
{
	return;
}

static void stub_engine_player_added(
	long player_index)
{
	return;
}

static void stub_engine_game_ending(
	void)
{
	return;
}

static void stub_engine_game_starting(
	void)
{
	return;
}

static void stub_engine_statistics_append(
	struct game_statistics *permanent_statistics,
	struct game_statistics *game_statistics)
{
	return;
}

static void stub_engine_handle_client_message(
	long player_index,
	void *encoded_message,
	short encoded_message_size)
{
	return;
}

static void stub_engine_handle_server_message(
	void *encoded_message,
	short encoded_message_size)
{
	return;
}

static void stub_engine_pregame_post_rasterize(
	void)
{
	return;
}

static void stub_engine_post_rasterize(
	void)
{
	return;
}

static void stub_engine_update(
	void)
{
	return;
}

static boolean stub_engine_allow_pick_up(
	long unit_index,
	long item_index)
{
	return TRUE;
}

static void stub_engine_player_damaged_player(
	long killing_player_index,
	long dead_player_index,
	boolean friendly_fire)
{
	return;
}

static void stub_engine_player_killed_player(
	long killing_player_index,
	long killing_object_index,
	long dead_player_index,
	boolean friendly_fire)
{
	return;
}
