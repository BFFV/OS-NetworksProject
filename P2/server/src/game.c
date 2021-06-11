#include "game.h"


// Waiting lobby
void lobby(int server) {

    // Players
    PlayersInfo* players = malloc(sizeof(PlayersInfo));
    players->sockets = malloc(5 * sizeof(int));  // Up to 5 players
    players->num_players = 0;

    // Waiting for the game to start
    bool waiting = true;
    while (waiting) {

        // New client socket
        struct sockaddr_in client_addr;
        socklen_t addr_size = sizeof(client_addr);

        // Accept new player
        int new_player = accept(server, (struct sockaddr*) &client_addr, &addr_size);
        players->sockets[players->num_players] = new_player;
        players->num_players++;
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

        // Game starts
        //waiting = false;
    }
}
