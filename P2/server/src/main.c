#include "connection.h"
#include "game.h"


// Monster Hunter Server
int main(int argc, char* argv[]) {

    // Initialize server
    int server = init_server(argv[2], atoi(argv[4])); // Receives IP & PORT

    // Initialize game
    Game* game = init_game(4);

    // Run server
    await_requests(server, game);

    return 0;
}
