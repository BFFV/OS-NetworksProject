#include "game.h"


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

// Listen to multiple clients for requests
void await_requests(int server_socket, PlayersInfo* players) {

    // Define vars
    int new_player;  // Player ID
    int activity;  // Used for select()
    int sd;  // Socket ID
    int max_sd;  // Maximum socket descriptor
    char buffer[1025];  // Data buffer of 1K

    // Set of socket descriptors
    fd_set readfds;

    // Initialize all client sockets to 0
    for (int p = 0; p < players->max_players; p++) {
        players->sockets[p] = 0;
    }

    // Accept incoming connections
    struct sockaddr_in client_addr;
    socklen_t addr_size = sizeof(client_addr);
    while (true) {

        // Clear the socket set
        FD_ZERO(&readfds);

        // Add server socket to set
        FD_SET(server_socket, &readfds);
        max_sd = server_socket;

        // Add child sockets to set
        for (int d = 0 ; d < players->max_players; d++) {

            // Socket descriptor
            sd = players->sockets[d];

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
        if (FD_ISSET(server_socket, &readfds) && (players->num_players < players->max_players)) {

            // Accept new player
            new_player = accept(server_socket, (struct sockaddr*) &client_addr, &addr_size);
            players->sockets[players->num_players] = new_player;
            players->num_players++;

            // Inform user of socket number - used in send and receive commands
            printf("New connection , socket fd is %d , ip is : %s , port : %d\n",
                   new_player, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            // Send new connection greeting message
            char* num = itoa(players->num_players);
            char* welcome = "Bienvenido Jugador ";
            char* end = "!";
            char* msg[3];
            msg[0] = welcome;
            msg[1] = num;
            msg[2] = end;
            char* welcome_msg = concatenate(msg, 3);
            server_send_message(new_player, 1, welcome_msg);
            free(num);
            free(welcome_msg);
        }

        // Request from client
        for (int p = 0; p < players->max_players; p++) {

            // Current client
            sd = players->sockets[p];
            if (FD_ISSET(sd, &readfds)) {

                // Package ID
                int pkg_id = server_receive_id(sd);

                // Process request
                process_request(get_request_type(pkg_id), sd, p, players);
            }
        }
    }
}

// Process request from client
void process_request(Request req_type, int client, int player, PlayersInfo* players) {
    char msg[2000];
    switch (req_type) {
        case DISCONNECT:
            disconnect(client, player, players);
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

// Disconnect player
void disconnect(int client, int player, PlayersInfo* players) {
    close(client);
    players->sockets[player] = 0;
    players->num_players--;
}

// Destroy players
void destroy_players(PlayersInfo* players_info) {
    free(players_info->sockets);
    for (int p = 0; p < players_info->max_players; p++) {
        destroy_character(players_info->characters[p]);
    }
    free(players_info->characters);
}
