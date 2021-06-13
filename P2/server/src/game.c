#include "game.h"


// ----------- Game ----------- //

// Initialize game object
Game* init_game(int max_players) {
    Game* game = malloc(sizeof(Game));
    game->max_players = max_players;
    game->num_players = 0;
    game->players = malloc(max_players * sizeof(int));
    game->characters = malloc(max_players * sizeof(Character*));
    game->rounds = 0;
    game->playing = false;
    return game;
}

// Destroy game
void destroy_game(Game* game) {
    free(game->players);
    for (int p = 0; p < game->num_players; p++) {
        destroy_character(game->characters[p]);
    }
    free(game->characters);
    free(game);
}


// ----------- Networking ----------- //

// Map pkg_id to request type
Request get_request_type(int pkg_id) {
    switch (pkg_id) {
        case 0:
            return DISCONNECT;
        case 1:
            return CREATE_PLAYER;
        case 2:
            return INIT_GAME;
        case 3:
            return SELECT_MONSTER;
        case 4:
            return SELECT_SKILL;
        case 5:
            return SURRENDER;
        case 6:
            return CONTINUE;
        default:
            return CONTINUE;
    }
}

// Map request type to pkg_id
int get_pkg_id(Request request_type) {
    switch (request_type) {
        case DISCONNECT:
            return 0;
        case CREATE_PLAYER:
            return 1;
        case INIT_GAME:
            return 2;
        case SELECT_MONSTER:
            return 3;
        case SELECT_SKILL:
            return 4;
        case SURRENDER:
            return 5;
        case CONTINUE:
            return 6;
        default:
            return 6;
    }
}

// Listen to multiple clients for requests
void await_requests(int server_socket, Game* game) {

    // Define vars
    int new_player;  // Player ID
    int activity;  // Used for select()
    int sd;  // Socket ID
    int max_sd;  // Maximum socket descriptor

    // Set of socket descriptors
    fd_set readfds;

    // Initialize all client sockets to 0
    for (int p = 0; p < game->max_players; p++) {
        game->players[p] = 0;
    }

    // Accept incoming requests/connections
    struct sockaddr_in client_addr;
    socklen_t addr_size = sizeof(client_addr);
    while (true) {

        // Clear the socket set
        FD_ZERO(&readfds);

        // Add server socket to set
        FD_SET(server_socket, &readfds);
        max_sd = server_socket;

        // Add child sockets to set
        for (int d = 0 ; d < game->max_players; d++) {

            // Socket descriptor
            sd = game->players[d];

            // If valid socket descriptor then add to read list
            if (sd > 0) {
                FD_SET(sd, &readfds);
            }

            // Highest file descriptor number, needed for select
            if (sd > max_sd) {
                max_sd = sd;
            }
        }

        // Wait indefinitely for an activity on one of the sockets
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        // New client connection
        if (FD_ISSET(server_socket, &readfds) && (game->num_players < game->max_players)) {

            // Accept new player
            new_player = accept(server_socket, (struct sockaddr*) &client_addr, &addr_size);
            game->players[game->num_players] = new_player;
            game->num_players++;

            // Send new connection welcome message
            char* num = itoa(game->num_players);
            char* welcome = "Bienvenido Jugador ";
            char* end = "!";
            char* msg[3];
            msg[0] = welcome;
            msg[1] = num;
            msg[2] = end;
            char* welcome_msg = concatenate(msg, 3);
            server_send_message(new_player, get_pkg_id(MESSAGE), welcome_msg);
            free(num);
            free(welcome_msg);
        }

        // Request from client
        for (int p = 0; p < game->max_players; p++) {

            // Current client
            sd = game->players[p];
            if (FD_ISSET(sd, &readfds)) {

                // Package ID
                int pkg_id = server_receive_id(sd);

                // Process request
                process_request(get_request_type(pkg_id), sd, p, game);
            }
        }
    }
}

// Process request from client
void process_request(Request req_type, int client, int player, Game* game) {
    char msg[2000];
    switch (req_type) {
        case DISCONNECT:
            disconnect(client, player, game);
            break;
        case CREATE_PLAYER:
            strcpy(msg, server_receive_payload(client));
            printf("%s\n", msg);
            break;
        case INIT_GAME:
            // disconnect(client, player, players);
            break;
        case SELECT_MONSTER:
            // disconnect(client, player, players);
            break;
        case SELECT_SKILL:
            // disconnect(client, player, players);
            break;
        case SURRENDER:
            // disconnect(client, player, players);
            break;
        case CONTINUE:
            // disconnect(client, player, players);
            break;
        default:
            break;
    }
}


// ----------- Handlers ----------- //

// Disconnect player
void disconnect(int client, int player, Game* game) {
    close(client);
    game->players[player] = 0;
    game->num_players--;
}
