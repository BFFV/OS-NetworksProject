#include "connection.h"
#include "communication.h"
#include "utils.h"
#include "characters.h"
#pragma once


// ----------- Structs/Enums ----------- //

// Game object
typedef struct game {
    int max_players;  // Maximum amount of players
    int num_players;  // Amount of players
    int* players;  // Array of player sockets
    Character** characters;  // Player characters
    int rounds;  // Round counter
    bool playing;  // Lobby/Game
} Game;

// Request types
typedef enum request_types {

    // Server side actions
    DISCONNECT,
    CREATE_PLAYER,
    INIT_GAME,
    SELECT_MONSTER,
    SELECT_SKILL,
    SURRENDER,
    CONTINUE,

    // Client side actions
    MESSAGE

} Request;


// ----------- Game ----------- //

// Initialize game object
Game* init_game(int max_players);

// Destroy game
void destroy_game(Game* game);


// ----------- Networking ----------- //

// Listen to multiple clients for requests
void await_requests(int server_socket, Game* game);

// Map pkg_id to request type
Request get_request_type(int pkg_id);

// Map request type to pkg_id
int get_pkg_id(Request request_type);

// Process request from client
void process_request(Request req_type, int client, int player, Game* game);


// ----------- Handlers ----------- //

// Disconnect player
void disconnect(int client, int player, Game* game);
