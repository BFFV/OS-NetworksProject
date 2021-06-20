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

    // https://api.libssh.org/stable/libssh_tutor_guided_tour.html
    // https://api.libssh.org/stable/libssh_tutor_forwarding.html (se debe implementar reverse port forwarding) según enunciado
    // TODO: eliminar el undefined symbols for arch x86_64 (error)
    //          Posible workaround si se usa libssh2.h
    ssh_session client_ssh_session = ssh_new();
    int rc;
    char *password;
    int verbosity = SSH_LOG_PROTOCOL;
    int port = 22;
    if (client_ssh_session == NULL) {
        exit(-1);
    }

    // SSH options
    ssh_options_set(client_ssh_session, SSH_OPTIONS_HOST, "iic2333.ing.puc.cl");
    ssh_options_set(client_ssh_session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
    ssh_options_set(client_ssh_session, SSH_OPTIONS_PORT, &port);

    // 1, Connect to SSH server
    rc = ssh_connect(client_ssh_session);
    if (rc != SSH_OK) {
        fprintf(stderr, "Error connecting to iic2333.ing.puc.cl: %s\n",
                ssh_get_error(client_ssh_session));
        exit(-1);
    }

    // 2. Verify the server's identity
    // For the source code of verify_knownhost(), check previous example
    if (verify_knownhost(client_ssh_session) < 0) {
        ssh_disconnect(client_ssh_session);
        ssh_free(client_ssh_session);
        exit(-1);
    }

    // 3. Authenticate ourselves
    rc = ssh_userauth_none(client_ssh_session, NULL);  // no password is asked
    if (rc != SSH_AUTH_SUCCESS) {
        fprintf(stderr, "Error authenticating with password: %s\n",
                ssh_get_error(client_ssh_session));
        ssh_disconnect(client_ssh_session);
        ssh_free(client_ssh_session);
        exit(-1);
    }

    // -------------------------------------------------------------------

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

    // -------------------------------------------------------------------

    // 5. Close the ssh session
    ssh_disconnect(client_ssh_session);
    ssh_free(client_ssh_session);

    return 0;
}
