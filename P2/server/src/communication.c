#include "communication.h"

//LINKS REFERENCIAS
//https://pubs.opengroup.org/onlinepubs/009695399/functions/recv.html
//https://pubs.opengroup.org/onlinepubs/009695399/functions/send.html


// Receive ID from message
int server_receive_id(int client_socket) {
  int id = 0;
  recv(client_socket, &id, 1, 0);
  return id;
}

// Receive payload from message
char* server_receive_payload(int client_socket) {
  int len = 0;
  recv(client_socket, &len, 1, 0);
  char* payload = malloc(len);
  int received = recv(client_socket, payload, len, 0);
  return payload;
}

// Send message to client
void server_send_message(int client_socket, int pkg_id, char* message) {
  int payloadSize = strlen(message) + 1;

  // Build package
  char msg[1 + 1 + payloadSize];
  msg[0] = pkg_id;
  msg[1] = payloadSize;
  memcpy(&msg[2], message, payloadSize);

  // Send package
  send(client_socket, msg, 2 + payloadSize, 0);
}
