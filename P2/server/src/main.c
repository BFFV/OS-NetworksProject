#include "connection.h"
#include "game.h"
#include <time.h>


// Monster Hunter Server
int main(int argc, char* argv[]) {

    // Set RNG seed
    srand(time(NULL));

    // Initialize server
    int server = init_server(argv[2], atoi(argv[4])); // Receives IP & PORT

    // Initialize game
    Game* game = init_game(4);

    // Run server
    await_requests(server, game);

    // Stop server
    close(server);
    printf("Server terminated normally!\n");

    return 0;
}
