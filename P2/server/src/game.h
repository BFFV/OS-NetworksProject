#include "connection.h"
#include "communication.h"
#include "characters.h"
#pragma once


// ----------- Structs/Enums ----------- //

// Game object
typedef struct game {
    int max_players;  // Maximum amount of players
    int num_players;  // Amount of players
    int* players;  // Array of player sockets
    char** usernames;  // Player usernames
    Character** characters;  // Player characters
    int current_player;  // Current player
    Character* monster;  // Game monster
    int rounds;  // Round counter
    bool playing;  // Lobby/Game
    int active_players;  // Active players
} Game;

// Request types
typedef enum request_types {
    SET_USERNAME,
    SELECT_CLASS,
    START_GAME,
    SELECT_MONSTER,
    SELECT_ACTION,
    SELECT_SKILL,
    SELECT_OBJECTIVE,
    CONTINUE,
    MESSAGE,
} Request;


// ----------- Game ----------- //

// Initialize game object
Game* init_game(int max_players);

// Check game state
void check_state(Game* game);

// Reset game
void reset_game(Game* game);

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


// ----------- Helper Functions ----------- //

void send_select_action_message(Game* game, int player);

void send_select_skill_message(Game* game, int player);

void send_select_objective_message(Game* game, int player);

void monster_turn(Game* game);

int get_next_player(Game* game, int index);

// Get active player index
int get_active_index(Game* game, int player);

int get_active_socket(Game* game, int random);

Character** get_active_characters(Game* game);

void reorder_players(Game* game);

// ----------- Handlers ----------- //

// Set username
void set_username(int client, int player, Game* game);

// Select class
void select_class(int client, int player, Game* game);

// Start game
void start_game(int client, int player, Game* game);

// Select monster
void select_monster(int client, int player, Game* game);

// Start turn
void start_turn(int client, int player, Game* game);

// Select action
void select_action(int client, int player, Game* game);

// Select skill
void select_skill(int client, int player, Game* game);

// Select objective
void select_objective(int client, int player, Game* game);

// Continue playing or exit server
void return_to_lobby(int client, int player, Game* game);

// Restart lobby
void remake_lobby(Game* game);
