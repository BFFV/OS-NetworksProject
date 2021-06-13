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
    char** usernames; // Player usernames
    Character** characters;  // Player characters
    int current_player;  // Current player

    Character* monster; // Game monster

    int rounds;  // Round counter
    bool playing;  // Lobby/Game
} Game;

// Request types
typedef enum request_types {
    DISCONNECT,
    SET_USERNAME,
    SELECT_CLASS,
    START_GAME,
    SELECT_MONSTER,
    SELECT_SKILL,  // SERVER only
    SELECT_OBJECTIVE,
    SURRENDER,  // SERVER only
    CONTINUE,
    MESSAGE,  // CLIENT only
    TURN,  // CLIENT only: Includes skill selection, surrender, disconnect
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


// ----------- Game Functions ----------- //

void start_turn(Game* game, int player);


// ----------- Handlers ----------- //

// Disconnect player
void disconnect(int client, int player, Game* game);

// Set Username for given player
void set_username(int client, int player, Game* game);

// Select Class
void select_class(int client, int player, Game* game);

// Start Game
void start_game(int client, int player, Game* game);

// Select monster
void select_monster(int client, int player, Game* game);

// Select skill
void select_skill(int client, int player, Game* game);

// Select objective
void select_objective(int client, int player, Game* game);
