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
    for (int p = 0; p < game->max_players; p++) {
        game->characters[p] = NULL;
    }
    game->monster = malloc(sizeof(Character*));
    game->current_player = 0;
    game->rounds = 0;
    game->playing = false;
    return game;
}

// Check game state
void check_state(Game* game) {

    // Check for dead monster
    if (game->monster->current_hp <= 0) {
        char* end_message = "\nEl monstruo ha muerto!!! El combate ha finalizado...\n";
        notify_users(game->players, game->num_players, get_pkg_id(MESSAGE), end_message, -1);
        return;
        // TODO: Re- partir el juego, chequear por nuevo lider y preguntar qué hacer...

    } else {

        // Next player's turn
        if (game->current_player < (game->num_players - 1)) {
            game->current_player++;

        } else {
            // The round has ended, it's time for the monster to attack
            monster_turn(game);
            apply_status_effects(game->characters, game->num_players);
            // TODO: death
            game->current_player = 0;
            game->rounds++;
        }
        start_turn(game->players[game->current_player], game->current_player, game);
    }
}

// Destroy game
void destroy_game(Game* game) {
    free(game->players);
    for (int p = 0; p < game->num_players; p++) {
        destroy_character(game->characters[p]);
        free(game->usernames[p]);
    }
    destroy_character(game->monster);
    free(game->usernames);
    free(game->characters);
    free(game);
}


// ----------- Networking ----------- //

// Map pkg_id to request type
Request get_request_type(int pkg_id) {
    Request type;
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
        case 10:
            return TURN;
    }
    return type;
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
        case TURN:
            return 10;
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
        for (int d = 0; d < game->max_players; d++) {

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
        if (FD_ISSET(server_socket, &readfds)) {

            // Invalid new player
            if ((game->num_players == game->max_players) || (game->playing)) {
                int invalid_player = accept(server_socket, (struct sockaddr*)&client_addr, &addr_size);
                char* invalid_msg = "\nNo es posible conectarse al juego actualmente!\n";
                server_send_message(invalid_player, get_pkg_id(MESSAGE), invalid_msg);
                close(invalid_player);
            } else {  // Accept new player
                new_player = accept(server_socket, (struct sockaddr*)&client_addr, &addr_size);
                game->players[game->num_players] = new_player;
                game->num_players++;

                // Send new connection welcome message
                char* num = itoa(game->num_players);
                char* welcome = "\nBienvenido Jugador ";
                char* end = "!\n";
                char* msg[3];
                msg[0] = welcome;
                msg[1] = num;
                msg[2] = end;
                char* welcome_msg = concatenate(msg, 3);
                server_send_message(new_player, get_pkg_id(MESSAGE), welcome_msg);
                free(welcome_msg);
                if (game->num_players == 1) {
                    char* leader = "\nAhora eres el líder del equipo!\n";
                    server_send_message(new_player, get_pkg_id(MESSAGE), leader);
                } else {
                    char* added = "\nSe ha unido el Jugador ";
                    char* new_msg[3];
                    new_msg[0] = added;
                    new_msg[1] = num;
                    new_msg[2] = end;
                    char* notify_leader = concatenate(new_msg, 3);
                    server_send_message(game->players[0], get_pkg_id(MESSAGE), notify_leader);
                    free(notify_leader);
                }
                free(num);
                char* username = "\nIngresa tu nombre de usuario:\n\n";
                server_send_message(new_player, get_pkg_id(SET_USERNAME), username);
            }
        }

        // Request from client
        for (int p = 0; p < game->max_players; p++) {

            // Current client
            sd = game->players[p];
            if (FD_ISSET(sd, &readfds)) {
                if ((game->playing && game->current_player == p) || (!game->playing)) {

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

// Set username
void set_username(int client, int player, Game* game) {
    char* username = server_receive_payload(client);
    game->usernames[player] = calloc(sizeof(char), strlen(username) + 1);
    strcpy(game->usernames[player], username);
    free(username);

    // Send message for class selection
    char* class = "\nIngresa tu clase:\n\n1) Cazador\n2) Médico\n3) Hacker\n\n";
    server_send_message(client, get_pkg_id(SELECT_CLASS), class);
}

// Select class
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
    char* ready = " está listo para jugar como ";
    char* msg_notify[4];
    msg_notify[0] = game->usernames[player];
    msg_notify[1] = ready;
    msg_notify[2] = selected;
    msg_notify[3] = "!\n";
    char* notify = concatenate(msg_notify, 4);
    server_send_message(client, get_pkg_id(MESSAGE), notify);
    if (player) {
        server_send_message(game->players[0], get_pkg_id(MESSAGE), notify);
    } else {
        char* start_msg = "\nPresiona ENTER para intentar comenzar el juego:\n";
        server_send_message(game->players[0], get_pkg_id(START_GAME), start_msg);
    }
    free(notify);
}

// Initialize game (Leader Only)
void start_game(int client, int player, Game* game) {
    // Ignore payload
    char* ignore = server_receive_payload(client);
    if (ignore != NULL) {
        free(ignore);
    }

    // Check if all players are ready
    for (int p = 0; p < game->num_players; p++) {
        if (game->characters[p] == NULL) {
            char* not_ready_msg = "\nAlgunos jugadores aún no están listos!\n";
            server_send_message(client, get_pkg_id(MESSAGE), not_ready_msg);
            char* start_msg = "\nPresiona ENTER para intentar comenzar el juego:\n";
            server_send_message(client, get_pkg_id(START_GAME), start_msg);
            return;
        }
    }

    // All players ready
    printf("READY\n");
    game->playing = true;
    game->current_player = 0;
    char* select_monster_msg = "\nSelecciona un monstruo contra el que combatir:\n\n1) Great JagRuz\n2) Ruzalos\n3) Ruiz, el Gemelo Malvado del Profesor Ruz\n4) Monstruo Aleatorio\n";
    server_send_message(client, get_pkg_id(SELECT_MONSTER), select_monster_msg);
}

// Select the monster to play against
void select_monster(int client, int player, Game* game) {

    // Chosen monster
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

        case 3:  // Random
            monster = create_character(get_random_monster());
            break;
    }
    // Set game monsters
    game->monster = monster;

    // Notify all game users that the game has started
    char* raw_text_0 = "\nEl juego ha comenzado! Pelearán contra el temible monstruo: ";
    char* raw_text_1 = "...\n";
    char* raw_msg[3];
    raw_msg[0] = raw_text_0;
    raw_msg[1] = monster_name;
    raw_msg[2] = raw_text_1;
    char* start_msg = concatenate(raw_msg, 3);
    notify_users(game->players, game->num_players, get_pkg_id(MESSAGE), start_msg, -1);
    free(start_msg);

    // Start the first round
    start_turn(client, player, game);
}

// Start turn
void start_turn(int client, int player, Game* game) {

    // Information for leader
    if (!player) {
        char* game_stats_raw[3 + game->num_players];

        // Build notification message
        char* game_stats_raw_header = "$$$$$$$$$$$ STATS $$$$$$$$$$$$\n";
        game_stats_raw[0] = game_stats_raw_header;

        // Get each player stats
        char* players_messages[game->num_players];
        for (int p = 0; p < game->num_players; p++) {
            char* player_raw_line[8];

            // Build character stats line
            player_raw_line[0] = game->usernames[p];
            char* raw_line_text_0 = "[";
            player_raw_line[1] = raw_line_text_0;
            player_raw_line[2] = game->characters[p]->class_name;
            char* raw_line_text_1 = "] -> VIDA: ";
            player_raw_line[3] = raw_line_text_1;
            char* current_health = itoa(game->characters[p]->current_hp);
            player_raw_line[4] = current_health;
            char* raw_line_text_2 = " / ";
            player_raw_line[5] = raw_line_text_2;
            char* max_health = itoa(game->characters[p]->max_hp);
            player_raw_line[6] = max_health;
            char* raw_line_text_3 = "\n";
            player_raw_line[7] = raw_line_text_3;

            // Add line to notification message
            char* player_statistics = concatenate(player_raw_line, 8);
            players_messages[p] = player_statistics;
            game_stats_raw[p + 1] = players_messages[p];
            free(current_health);
            free(max_health);
        }

        // Monster stats
        char* monster_stats_raw[6];
        monster_stats_raw[0] = game->monster->class_name;
        char* monster_raw_text_0 = " -> VIDA: ";
        monster_stats_raw[1] = monster_raw_text_0;
        char* monster_current_hp = itoa(game->monster->current_hp);
        monster_stats_raw[2] = monster_current_hp;
        char* monster_raw_text_1 = " / ";
        monster_stats_raw[3] = monster_raw_text_1;
        char* monster_max_hp = itoa(game->monster->max_hp);
        monster_stats_raw[4] = monster_max_hp;
        char* monster_raw_text_2 = "\n";
        monster_stats_raw[5] = monster_raw_text_2;

        // Concatenate monster stats and add to notification
        char* monster_stats = concatenate(monster_stats_raw, 6);
        game_stats_raw[1 + game->num_players] = monster_stats;
        free(monster_current_hp);
        free(monster_max_hp);

        // Concatenate notification and notify all users
        char* game_stats_raw_footer = "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n";
        game_stats_raw[game->num_players + 2] = game_stats_raw_footer;
        char* notification = concatenate(game_stats_raw, game->num_players + 3);
        notify_users(game->players, game->num_players, get_pkg_id(MESSAGE), notification, -1);

        // Free used memory
        for (int mem = 0; mem < game->num_players; mem++) {
            free(players_messages[mem]);
        }
        free(monster_stats);
        free(notification);

    }

    // Ask for skill
    send_select_skill_message(game, player);
}

// Select skill
void select_skill(int client, int player, Game* game) {
    int skill_id = atoi(server_receive_payload(client)) - 1;
    Ability selected = get_ability(game->characters[player], skill_id);
    game->characters[player]->selected_skill_id = skill_id;
    if (game->characters[player]->enemy_target[skill_id]) {
        use_ability(game->characters[player], game->monster, selected, game->num_players, game->characters, game->rounds);
        // TODO: Notify action
        check_state(game);
        return;
    }
    select_objective(client, player, game);
}

// Select objective
void select_objective(int client, int player, Game* game) {
    Character* player_character = game->characters[player];
    int objective_id = atoi(server_receive_payload(client)) - 1;

    // Notifies other users about the action
    char* raw_notification[7];
    char* raw_notification_header = "\n";
    raw_notification[0] = raw_notification_header;
    raw_notification[1] = game->usernames[player];
    char* raw_not_text_1 = " usó la habilidad -";
    raw_notification[2] = raw_not_text_1;
    raw_notification[3] = player_character->ability_names[player_character->selected_skill_id];
    char* raw_not_text_2 = "- en su aliado ";
    raw_notification[4] = raw_not_text_2;
    raw_notification[5] = game->usernames[objective_id];
    char* newline = "\n";
    raw_notification[6] = newline;

    // Send notification to all players
    char* notification_msg = concatenate(raw_notification, 7);
    notify_users(game->players, game->num_players, get_pkg_id(MESSAGE), notification_msg, -1);
    free(notification_msg);

    // TODO: retornar un mensaje con lo que hizo la habilidad
    use_ability(player_character,
        game->characters[objective_id],
        player_character->abilities[player_character->selected_skill_id],
        game->num_players, game->characters, game->rounds);
    check_state(game);
}

// ----------- Helper Functions ----------- //

// Send skill selection message
void send_select_skill_message(Game* game, int player) {

    // Notify all users except the current player
    char* raw_msg[3];
    char* raw_text_0 = "\n";
    raw_msg[0] = raw_text_0;
    raw_msg[1] = game->usernames[player];
    char* raw_text_1 = " está realizando su turno...\n";
    raw_msg[2] = raw_text_1;
    char* notification_msg = concatenate(raw_msg, 3);
    notify_users(game->players, game->num_players, get_pkg_id(MESSAGE), notification_msg, player);
    free(notification_msg);

    // Send select skill to current player
    char* raw_select_msg[1 + game->characters[player]->n_abilities];
    char* raw_select_header = "\nElige una de las siguientes habilidades:\n";
    raw_select_msg[0] = raw_select_header;
    for (int a = 0; a < game->characters[player]->n_abilities; a++) {
        char* raw_skill_line[4];
        char* index = itoa(a + 1);
        strcpy(raw_skill_line[0], index);
        char* raw_text_ability = ") ";
        raw_skill_line[1] = raw_text_ability;
        raw_skill_line[2] = game->characters[player]->ability_names[a];
        char* newline = "\n";
        raw_skill_line[3] = newline;
        char* skill_msg = concatenate(raw_skill_line, 4);
        strcpy(raw_select_msg[1 + a], skill_msg);
        free(skill_msg);
        free(index);
    }

    // Send selection message to user
    char* select_message = concatenate(raw_select_msg, 1 + game->characters[player]->n_abilities);
    server_send_message(game->players[player], get_pkg_id(SELECT_SKILL), select_message);
    free(select_message);
}

// Send objective message if needed
void send_select_objective_message(Game* game, int player) {
    char* raw_select_msg[1 + game->num_players];
    char* raw_header = "Selecciona objetivo:\n";
    raw_select_msg[0] = raw_header;

    for (int p = 0; p < game->num_players; p++) {
        char* raw_objective_line[4];
        char* index = itoa(p + 1);
        strcpy(raw_objective_line[0], index);
        char* raw_obj_par = ") ";
        raw_objective_line[1] = raw_obj_par;
        raw_objective_line[2] = game->usernames[player];
        char* newline = ".\n";
        raw_objective_line[3] = newline;
        char* objective_line = concatenate(raw_objective_line, 4);
        strcpy(raw_select_msg[p], objective_line);
        free(objective_line);
        free(index);
    }
    char* select_message = concatenate(raw_select_msg, 1 + game->num_players);
    server_send_message(game->players[player], get_pkg_id(SELECT_OBJECTIVE), select_message);
    free(select_message);
}

// The monster uses an ability on a random user
void monster_turn(Game* game) {

    // Get random ability
    int selected_ability_id = get_random_ability_id(game->monster);
    Ability selected_ability = game->monster->abilities[selected_ability_id];
    Character* defender = NULL;
    int defender_player_id;

    // Hunter's ability: Distraer
    if (game->monster->next_defender) {
        defender = game->monster->next_defender;
        // TODO: defender player id as attribute for Character
        defender_player_id = 0;
        game->monster->next_defender = NULL;

    } else {
        // Get a random player
        defender_player_id = get_random_character_id(game->num_players);
        defender = game->characters[defender_player_id];
    }

    // Monster attacks
    use_ability(game->monster, defender, selected_ability, game->num_players, game->characters, game->rounds);

    // TODO: Esta notificación debería retornarse en el use_ability
    // Notifies users of the monster attack
    char* raw_notification[5];
    char* raw_notification_header = "\nEl monstruo usó la habilidad -";
    raw_notification[0] = raw_notification_header;
    raw_notification[1] = game->monster->ability_names[selected_ability_id];
    char* raw_not_text_2 = "- sobre ";
    raw_notification[2] = raw_not_text_2;
    raw_notification[3] = game->usernames[defender_player_id];
    char* newline = "\n";
    raw_notification[4] = newline;
    char* notification_msg = concatenate(raw_notification, 5);
    notify_users(game->players, game->num_players, get_pkg_id(MESSAGE), notification_msg, -1);
    free(notification_msg);
}
