#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#pragma once


// Receive ID from message
int client_receive_id(int client_socket);

// Receive payload from message
char* client_receive_payload(int client_socket);

// Send message to server
void client_send_message(int client_socket, int pkg_id, char* message);

// Receive image
int* client_receive_image(int client_socket, char* data);
