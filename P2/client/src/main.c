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
        response[pos] = c;
        pos++;
    }
    response[pos] = '\0';
    return response;
}

// Monster Hunter Client
int main (int argc, char *argv[]) {

    // Initialize client
    int client = prepare_socket(argv[2], atoi(argv[4]));

    // Run client
    bool running = true;
    while (running) {
        int msg_code = client_receive_id(client);
        char* message = client_receive_payload(client);
        printf("%s", message);
        free(message);

        // End connection
        if (!msg_code) {
            running = false;
        } else if (msg_code != 9) {  // Input required
            char* response = get_input();
            client_send_message(client, msg_code, response);
        }
    }

    // Close client socket
    close(client);

    return 0;
}
