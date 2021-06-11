#include "connection.h"
#include "communication.h"
#include "utils.h"
#pragma once


// Players information
typedef struct players_info {
  int* sockets;  // Array of player sockets
  int num_players;  // Amount of players
} PlayersInfo;

// Waiting lobby
void lobby(int server);
