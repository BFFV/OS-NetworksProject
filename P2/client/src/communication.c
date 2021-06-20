#include "communication.h"


// Receive ID from message
int client_receive_id(int client_socket) {
    int id = 0;
    recv(client_socket, &id, 1, 0);
    return id;
}

// Receive payload from message
char* client_receive_payload(int client_socket) {
    int len = 0;
    recv(client_socket, &len, 1, 0);
    char* payload = calloc(sizeof(char), len + 1);
    int received = recv(client_socket, payload, len, 0);
    return payload;
}

// Send message to server
void client_send_message(int client_socket, int pkg_id, char* message) {
    int length = strlen(message);
    if (length) {
        length++;
    }
    int payloadSize = length;

    // Build package
    char msg[1 + 1 + payloadSize];
    msg[0] = pkg_id;
    msg[1] = payloadSize;
    memcpy(&msg[2], message, payloadSize);

    // Send package
    send(client_socket, msg, 2 + payloadSize, 0);
}

// Receive image
int* client_receive_image(int client_socket, char* data) {
    int n_packages = 0;
    recv(client_socket, &n_packages, 1, 0);
    int current_package = 0;
    recv(client_socket, &current_package, 1, 0);
    int payload_size = 0;
    recv(client_socket, &payload_size, 1, 0);
    int offset = (current_package - 1) * 255;
    int received = recv(client_socket, data + offset, payload_size, MSG_WAITALL);

    // Done + received
    int* status = malloc(2 * sizeof(int));
    status[0] = 1;
    status[1] = payload_size;

    // Last package
    if (current_package == n_packages) {
        status[0] = 0;
    }
    return status;
}
