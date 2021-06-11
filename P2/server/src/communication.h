#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#pragma once


// Receive ID from message
int server_receive_id(int client_socket);

// Receive payload from message
char* server_receive_payload(int client_socket);

// Send message to client
void server_send_message(int client_socket, int pkg_id, char* message);
