#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include "connection.h"
#include "communication.h"


// Receive input
char* get_input() {
    char* response = calloc(sizeof(char), 255);
    int pos = 0;
    printf(">> ");
    while (true) {
        char c = getchar();
        if (c == '\n') break;
        if (pos <= 253) {
            response[pos] = c;
        }
        pos++;
    }
    return response;
}

// Monster Hunter Client
int main (int argc, char* argv[]) {

    // Check input
    if (argc < 5) {
        printf("[Error] Not enough arguments were provided!\n");
        return 1;
    }

    // Initialize client
    int* connection = prepare_socket(argv[2], atoi(argv[4]));

    // Run client
    bool running = true;
    if (connection[1] == -1) {
        running = false;
    }
    while (running) {
        int msg_code = client_receive_id(connection[0]);
        char* message = client_receive_payload(connection[0]);
        printf("%s", message);
        free(message);

        // End connection
        if (!msg_code) {
            running = false;
        } else if (msg_code != 9) {  // Input required
            char* response = get_input();
            client_send_message(connection[0], msg_code, response);
            free(response);
        }
    }

    // Close client socket
    close(connection[0]);
    free(connection);

    return 0;
}
