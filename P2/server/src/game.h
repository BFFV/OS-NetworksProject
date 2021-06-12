#include "connection.h"
#include "communication.h"
#include "utils.h"
#include "characters.h"
#pragma once


// Players information
typedef struct players_info {
    int max_players;  // Maximum amount of players
    int num_players;  // Amount of players
    int* sockets;  // Array of player sockets
    Character** characters;  // Player characters
} PlayersInfo;

// Request types
typedef enum request_types {
    DISCONNECT,
    CREATE_PLAYER,
    INIT_GAME,
    SELECT_MONSTER,
    SELECT_SKILL,
    SURRENDER,
    CONTINUE
} Request;

// Listen to multiple clients for requests
void await_requests(int server_socket, PlayersInfo* players);

// Map pkg_id to request type
Request get_request_type(int pkg_id);

// Process request from client
void process_request(Request req_type, int client, int player, PlayersInfo *players);

// Disconnect player
void disconnect(int client, int player, PlayersInfo* players);

// Destroy players
void destroy_players(PlayersInfo* players_info);
