#include "game.h"


// ----------- Game ----------- //

// Initialize game object
Game* init_game(int max_players) {
    Game* game = malloc(sizeof(Game));
    game->max_players = max_players;
    game->num_players = 0;
    game->players = malloc(max_players * sizeof(int));
    game->usernames = malloc(max_players * sizeof(char*));
    game->characters = malloc(max_players * sizeof(Character*));
    game->current_player = 0;
    game->rounds = 0;
    game->playing = false;
    return game;
}

// Destroy game
void destroy_game(Game* game) {
    free(game->players);
    for (int p = 0; p < game->num_players; p++) {
        destroy_character(game->characters[p]);
    }
    free(game->characters);
    free(game);
}


// ----------- Networking ----------- //

// Map pkg_id to request type
Request get_request_type(int pkg_id) {
    switch (pkg_id) {
        case 0:
            return DISCONNECT;
        case 1:
            return SET_USERNAME;
        case 2:
            return SELECT_CLASS;
        case 3:
            return START_GAME;
        case 4:
            return SELECT_MONSTER;
        case 5:
            return SELECT_SKILL;
        case 6:
            return SELECT_OBJECTIVE;
        case 7:
            return SURRENDER;
        case 8:
            return CONTINUE;
        case 9:
            return MESSAGE;
    }
}

// Map request type to pkg_id
int get_pkg_id(Request request_type) {
    switch (request_type) {
        case DISCONNECT:
            return 0;
        case SET_USERNAME:
            return 1;
        case SELECT_CLASS:
            return 2;
        case START_GAME:
            return 3;
        case SELECT_MONSTER:
            return 4;
        case SELECT_SKILL:
            return 5;
        case SELECT_OBJECTIVE:
            return 6;
        case SURRENDER:
            return 7;
        case CONTINUE:
            return 8;
        case MESSAGE:
            return 9;
    }
}

// Listen to multiple clients for requests
void await_requests(int server_socket, Game* game) {

    // Define vars
    int new_player;  // Player ID
    int activity;  // Used for select()
    int sd;  // Socket ID
    int max_sd;  // Maximum socket descriptor

    // Set of socket descriptors
    fd_set readfds;

    // Initialize all client sockets to 0
    for (int p = 0; p < game->max_players; p++) {
        game->players[p] = 0;
    }

    // Accept incoming requests/connections
    struct sockaddr_in client_addr;
    socklen_t addr_size = sizeof(client_addr);
    while (true) {

        // Clear the socket set
        FD_ZERO(&readfds);

        // Add server socket to set
        FD_SET(server_socket, &readfds);
        max_sd = server_socket;

        // Add child sockets to set
        for (int d = 0 ; d < game->max_players; d++) {

            // Socket descriptor
            sd = game->players[d];

            // If valid socket descriptor then add to read list
            if (sd > 0) {
                FD_SET(sd, &readfds);
            }

            // Highest file descriptor number, needed for select
            if (sd > max_sd) {
                max_sd = sd;
            }
        }

        // Wait indefinitely for an activity on one of the sockets
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        // New client connection
        if (FD_ISSET(server_socket, &readfds) && (game->num_players < game->max_players)) {

            // Accept new player
            new_player = accept(server_socket, (struct sockaddr*) &client_addr, &addr_size);
            game->players[game->num_players] = new_player;
            game->num_players++;

            // Send new connection welcome message
            char* num = itoa(game->num_players);
            char* welcome = "Bienvenido Jugador ";
            char* end = "!";
            char* msg[3];
            msg[0] = welcome;
            msg[1] = num;
            msg[2] = end;
            char* welcome_msg = concatenate(msg, 3);
            server_send_message(new_player, get_pkg_id(MESSAGE), welcome_msg);
            free(num);
            free(welcome_msg);
            char* username = "Ingresa tu nombre de usuario:";
            server_send_message(new_player, get_pkg_id(SET_USERNAME), username);
        }

        // Request from client
        for (int p = 0; p < game->max_players; p++) {

            // Current client
            sd = game->players[p];
            if (FD_ISSET(sd, &readfds)) {

                if (game->playing && game->current_player == p) {
                    // Package ID
                    int pkg_id = server_receive_id(sd);

                    // Process request
                    process_request(get_request_type(pkg_id), sd, p, game);
                } else if (!game->playing) {
                    // Package ID
                    int pkg_id = server_receive_id(sd);

                    // Process request
                    process_request(get_request_type(pkg_id), sd, p, game);
                }
            }
        }
    }
}

// Process request from client
void process_request(Request req_type, int client, int player, Game* game) {
    switch (req_type) {

        case DISCONNECT:
            disconnect(client, player, game);
            break;

        case SET_USERNAME:
            set_username(client, player, game);
            break;

        case SELECT_CLASS:
            select_class(client, player, game);
            break;

        case START_GAME:
            start_game(client, player, game);
            break;

        case SELECT_MONSTER:
            select_monster(client, player, game);
            break;

        case SELECT_SKILL:
            select_skill(client, player, game);
            break;

        case SELECT_OBJECTIVE:
            select_objective(client, player, game);
            break;

        case SURRENDER:
            // disconnect(client, player, players);
            break;

        case CONTINUE:
            // disconnect(client, player, players);
            break;

        default:
            break;
    }
}


// ----------- Handlers ----------- //

// Disconnect player
void disconnect(int client, int player, Game* game) {
    close(client);
    game->players[player] = 0;
    game->num_players--;
}

// Set Username for given player
void set_username(int client, int player, Game* game) {
    char* username = server_receive_payload(client);
    game->usernames[player] = calloc(sizeof(char), strlen(username) + 1);
    strcpy(game->usernames[player], username);
    free(username);

    // Send message for class selection
    char* class = "Ingresa tu clase:\n\n1) Cazador\n2) Médico\n3) Hacker\n";
    server_send_message(client, get_pkg_id(SELECT_CLASS), class);
}

// Set Username for given player
void select_class(int client, int player, Game* game) {
    int class_id = atoi(server_receive_payload(client)) - 1;
    Character* new_character;
    char* selected;
    switch (class_id) {
        case 0:
            selected = "Cazador";
            new_character = create_character(HUNTER);
            break;

        case 1:
            selected = "Médico";
            new_character = create_character(MEDIC);
            break;

        case 2:
            selected = "Hacker";
            new_character = create_character(HACKER);
            break;
    }
    game->characters[player] = new_character;
    char* first = "El jugador ";
    char* second = " está listo para jugar como ";
    char* msg_notify[5];
    msg_notify[0] = first;
    msg_notify[1] = game->usernames[player];
    msg_notify[2] = second;
    msg_notify[3] = selected;
    msg_notify[4] = "!";
    char* notify = concatenate(msg_notify, 5);
    server_send_message(client, get_pkg_id(MESSAGE), notify);
    if (player) {
        server_send_message(game->players[0], get_pkg_id(MESSAGE), notify);
    } else {
        char* start_msg = "Presiona ENTER para comenzar el juego";
        server_send_message(game->players[0], get_pkg_id(START_GAME), start_msg);
    }
    free(notify);
}

// Initialize Game (Leader Only)
void start_game(int client, int player, Game* game) {
    if (player != 0) {
        return;
    }
    game->playing = true;
    char* select_monster_msg = "Selecciona un monstruo contra el que pelear:\n\n1)Great JagRuz\n2)Ruzalos\n3)Ruiz, el Gemelo Malvado del Profesor Ruz\n";
    server_send_message(client, get_pkg_id(SELECT_MONSTER), select_monster_msg);
}

void select_monster(int client, int player, Game* game) {
    if (player != 0) {
        return;
    }

    int monster_id = atoi(server_receive_payload(client)) - 1;
    Character* monster;
    char* monster_name;
    switch (monster_id) {
        case 0:
            monster_name = "Great JagRuz";
            monster = create_character(GREAT_JAGRUZ);
            break;

        case 1:
            monster_name = "Ruzalos";
            monster = create_character(RUZALOS);
            break;

        case 2:
            monster_name = "Ruiz";
            monster = create_character(RUIZ);
            break;
    }

    // Notifies all game users that the game has started
    char* raw_msg[3];
    char* raw_text_0 = ">> El juego ha iniciado! Pelearán contra el temible mounstro: ";
    raw_msg[0] = raw_text_0;
    raw_msg[1] = monster_name;
    char* raw_text_1 = "...\n";
    raw_msg[2] = raw_text_1;
    char* start_msg = concatenate(raw_msg, 3);
    notify_users(game->players, game->num_players, get_pkg_id(MESSAGE), start_msg, -1);
    free(start_msg);

    // Start the first round
    start_turn(game, 0);
}


// Select skill
void select_skill(int client, int player, Game* game) {
    int skill_id = atoi(server_receive_payload(client)) - 1;
    Ability selected = get_ability(game->characters[player], skill_id);
    char* msg = "Selecciona objetivo:\n";
    server_send_message(client, get_pkg_id(MESSAGE), msg);
    char* player_list[game->num_players];
    for (int p = 0; p < game->num_players; p++) {
        char* index = itoa(p + 1);
        char* parent = ") ";
        char* username = game->usernames[player];
        char* newline = "\n";
        char* item[4];
        item[0] = index;
        item[1] = parent;
        item[2] = username;
        item[3] = newline;
        char* player_item = concatenate(item, 4);
        strcpy(player_list[p], player_item);
        free(index);
        free(player_item);
    }
    char* obj_msg = concatenate(player_list, game->num_players);
    server_send_message(client, get_pkg_id(SELECT_OBJECTIVE), obj_msg);
    free(obj_msg);
}

// Select objective
void select_objective(int client, int player, Game* game) {
    int objective_id = atoi(server_receive_payload(client)) - 1;
    return;
}

// ----------- Game Functions ----------- //

// Start the turn of the given player
void start_turn(Game* game, int player) {

    // Notify all users except the current players
    char* raw_msg[3];
    char* raw_text_0 = "El jugador";
    raw_msg[0] = raw_text_0;
    raw_msg[1] = game->usernames[player];
    char* raw_text_1 = "está jugando...\n";
    raw_msg[2] = raw_text_1;
    char* notification_msg = concatenate(raw_msg, 3);
    notify_users(game->players, game->num_players, get_pkg_id(MESSAGE), notification_msg, player);
    free(notification_msg);
}
