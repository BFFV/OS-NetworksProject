#include "connection.h"


// Connect to server
int* prepare_socket(char* IP, int PORT) {
    struct sockaddr_in server_addr;

    // Create socket for client
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);

    // Server params
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_aton(IP, &server_addr.sin_addr);

    // Connect to server
    int ret = connect(client_socket, (struct sockaddr*) &server_addr, sizeof(server_addr));

    // Socket + Connection
    int* connection = malloc(2 * sizeof(int));
    connection[0] = client_socket;
    connection[1] = ret;

    return connection;
}
