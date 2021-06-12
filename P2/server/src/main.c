#include "connection.h"
#include "game.h"


// Monster Hunter Server
int main(int argc, char* argv[]) {

    // Initialize server
    int server = init_server(argv[2], atoi(argv[4])); // Receives IP & PORT

    // Initialize players info
    PlayersInfo* players = malloc(sizeof(PlayersInfo));
    players->max_players = 5; // Up to 5 players
    players->num_players = 0;
    players->sockets = malloc(players->max_players * sizeof(int));
    players->characters = malloc(players->max_players * sizeof(Character*));

    // Run server
    await_requests(server, players);

    // Guardaremos los sockets en un arreglo e iremos alternando a quién escuchar.
    /*
    int sockets_array[2] = {players_info->socket_c1, players_info->socket_c2};
    int my_attention = 0;
    while (1) {

    if (msg_code == 1) //El cliente me envió un mensaje a mi (servidor)
    {
        char * client_message = server_receive_payload(sockets_array[my_attention]);
        printf("El cliente %d dice: %s\n", my_attention+1, client_message);

        // Le enviamos la respuesta
        server_send_message(sockets_array[my_attention], 1, response);
    }
    else if (msg_code == 2){ //El cliente le envía un mensaje al otro cliente
        char * client_message = server_receive_payload(sockets_array[my_attention]);
        printf("Servidor traspasando desde %d a %d el mensaje: %s\n", my_attention+1, ((my_attention+1)%2)+1, client_message);

        // Mi atención cambia al otro socket
        my_attention = (my_attention + 1) % 2;

        server_send_message(sockets_array[my_attention], 2, client_message);
    }
    printf("------------------\n");
    }
    */
    return 0;
}
