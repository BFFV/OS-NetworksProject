#include "communication.h"


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
    if (!len) {
        return NULL;
    }
    char* payload = malloc(len);
    int received = recv(client_socket, payload, len, 0);
    return payload;
}

// Send message to client
void server_send_message(int client_socket, int pkg_id, char* message) {
    int length = strlen(message);
    if (length) {
        length++;
    }
    int payloadSize = length;

    // Build packages
    int sent = 0;
    int n_packages = payloadSize / 255;
    if (n_packages * 255 < payloadSize) {
        n_packages++;
    }
    for (int p = 0; p < n_packages; p++) {
        int to_send = payloadSize - sent;
        if (to_send > 255) {
            to_send = 255;
        }
        char msg[1 + 1 + to_send];
        msg[0] = 9;  // Messages for fragments
        if (p == n_packages - 1) {
            msg[0] = pkg_id;
        }
        msg[1] = to_send;
        memcpy(&msg[2], message + sent, to_send);

        // Send package
        send(client_socket, msg, 2 + to_send, 0);
        sent += to_send;
    }
}

// Send message to all connected users
void notify_users(int* clients, int n_clients, int pkg_id, char* message, int exception) {
    for (int u = 0; u < n_clients; u++) {
        if (u != exception) {
            server_send_message(clients[u], pkg_id, message);
        }
    }
}

// Send image to client
void server_send_image(int client_socket, int pkg_id, char* message, int size) {
    // Build packages
    int sent = 0;
    int n_packages = size / 255;
    if (n_packages * 255 < size) {
        n_packages++;
    }
    for (int p = 0; p < n_packages; p++) {
        int to_send = size - sent;
        if (to_send > 255) {
            to_send = 255;
        }
        char* msg = calloc(4 + to_send, sizeof(char));
        msg[0] = pkg_id;
        msg[1] = n_packages;
        msg[2] = p + 1;
        msg[3] = to_send;
        memcpy(&msg[4], message + sent, to_send);

        // Send package
        send(client_socket, msg, 4 + to_send, 0);
        free(msg);
        sent += to_send;
    }
}
