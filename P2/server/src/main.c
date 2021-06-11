#include "connection.h"
#include "game.h"


// Monster Hunter Server
int main(int argc, char* argv[]) {

  // Initialize server
  int server = init_server(argv[2], atoi(argv[4])); // Receives IP & PORT

  // Start waiting lobby
  lobby(server);

  // Guardaremos los sockets en un arreglo e iremos alternando a quién escuchar.
  /*
  int sockets_array[2] = {players_info->socket_c1, players_info->socket_c2};
  int my_attention = 0;
  while (1) {
    // Se obtiene el paquete del cliente 1
    int msg_code = server_receive_id(sockets_array[my_attention]);

    if (msg_code == 1) //El cliente me envió un mensaje a mi (servidor)
    {
      char * client_message = server_receive_payload(sockets_array[my_attention]);
      printf("El cliente %d dice: %s\n", my_attention+1, client_message);

      // Le enviaremos el mismo mensaje invertido jeje
      char * response = revert(client_message);

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
