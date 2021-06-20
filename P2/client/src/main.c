#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include "connection.h"
#include "communication.h"
#include "file_manager.h"
#include "utils.h"


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
    int loot_amount = 0;
    create_dir("loot_conseguido");

    // Run client
    bool running = true;
    if (connection[1] == -1) {
        running = false;
    }
    while (running) {
        int msg_code = client_receive_id(connection[0]);

        // Receive loot
        if (msg_code == 10) {
            bool receiving = true;
            char* img_data = calloc(51200, sizeof(char));
            int n_bytes = 0;
            int* status;
            while (receiving) {
                if (n_bytes) {
                    client_receive_id(connection[0]);
                }
                status = client_receive_image(connection[0], img_data);
                n_bytes += status[1];
                receiving = status[0];
                free(status);
            }

            // Save image
            char* img_path[3];
            img_path[0] = "loot_conseguido/loot";
            img_path[1] = itoa(loot_amount);
            img_path[2] = ".PNG";
            char* loot_path = concatenate(img_path, 3);
            write_img(loot_path, img_data, n_bytes);
            loot_amount++;
            free(img_data);
            free(img_path[1]);
            free(loot_path);
        } else {
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
    }

    // Close client socket
    close(connection[0]);
    free(connection);

    return 0;
}
