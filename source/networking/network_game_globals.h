/*
NETWORK_GAME_GLOBALS.H

header included in hcex build.
*/

#ifndef __NETWORK_GAME_GLOBALS_H
#define __NETWORK_GAME_GLOBALS_H
#pragma once

/* ---------- headers */

#include "bungie_net/common/message_header.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/NETWORK_GAME_GLOBALS.C */

boolean network_game_is_active(void);
struct network_game_client *global_network_game_client_get(void);

/* ---------- globals */

/* ---------- public code */

#endif // __NETWORK_GAME_GLOBALS_H
