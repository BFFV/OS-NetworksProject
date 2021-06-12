#include <stdbool.h>
#include "connection.h"


// Initialize server
int init_server(char* IP, int port) {

    // Server socket address
    struct sockaddr_in server_addr;

    // Create socket for server
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // Socket settings
    int opt = 1;
    int ret = setsockopt(server_socket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    // Save IP & PORT
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    inet_aton(IP, &server_addr.sin_addr);
    server_addr.sin_port = htons(port);

    // Assign IP & PORT for server socket
    int ret2 = bind(server_socket, (struct sockaddr*) &server_addr, sizeof(server_addr));

    // Listen for clients
    int ret3 = listen(server_socket, 1);

    return server_socket;
}
