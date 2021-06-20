#include "game.h"


// ----------- Global ----------- //
bool running = true;  // Server is running

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
    game->monster = NULL;
    game->current_player = 0;
    game->rounds = 0;
    game->playing = false;
    game->active_players = 0;
    return game;
}

// Check game state
void check_state(Game* game) {

    // Monster is defeated
    if (!game->monster->is_active) {
        char* end_message = "\nEl monstruo ha muerto!!! El combate ha finalizado...\n";
        notify_users(game->players, game->num_players, get_pkg_id(MESSAGE), end_message, -1);
        gain_loot(game);  // Loot bonus
        reset_game(game);

    } else {
        // Next player's turn
        int next = get_next_player(game, game->current_player);
        if (next >= 0) {
            game->current_player = next;
            start_turn(game->players[game->current_player], game->current_player, game);
        } else {
            // Monster wins due to surrendering
            if (!game->active_players) {
                char* end_message = "\nEl monstruo ha derrotado a todos los jugadores!!! El combate ha finalizado...\n";
                notify_users(game->players, game->num_players, get_pkg_id(MESSAGE), end_message, -1);
                reset_game(game);
                return;
            }

            // The round has ended, it's time for the monster to attack
            Character** active_characters = get_active_characters(game);
            int active_num = game->active_players;
            monster_turn(game);

            // Monster dies from bleeding
            if (!game->monster->is_active) {
                char* end_message = "\nEl monstruo ha muerto!!! El combate ha finalizado...\n";
                notify_users(game->players, game->num_players, get_pkg_id(MESSAGE), end_message, -1);
                gain_loot(game);  // Loot bonus
                reset_game(game);
                free(active_characters);
                return;
            }

            // Check deaths
            for (int p = 0; p < active_num; p++) {
                if (!active_characters[p]->is_active) {
                    game->active_players--;
                    char* death[3];
                    death[0] = "\n";
                    death[1] = active_characters[p]->name;
                    death[2] = " ha sido eliminado por el monstruo!\n";
                    char* death_msg = concatenate(death, 3);
                    notify_users(game->players, game->num_players, get_pkg_id(MESSAGE), death_msg, -1);
                    free(death_msg);
                }
            }

            // Monster wins
            if (!game->active_players) {
                char* end_message = "\nEl monstruo ha derrotado a todos los jugadores!!! El combate ha finalizado...\n";
                notify_users(game->players, game->num_players, get_pkg_id(MESSAGE), end_message, -1);
                reset_game(game);
                free(active_characters);
                return;
            }

            // Apply status effects for players
            free(active_characters);
            active_characters = get_active_characters(game);
            active_num = game->active_players;
            apply_status_effects(active_characters, game->active_players, game->players, game->num_players);

            // Check deaths
            for (int p = 0; p < active_num; p++) {
                if (!active_characters[p]->is_active) {
                    game->active_players--;
                    char* death[3];
                    death[0] = "\n";
                    death[1] = active_characters[p]->name;
                    death[2] = " ha sido eliminado por un efecto de status!\n";
                    char* death_msg = concatenate(death, 3);
                    notify_users(game->players, game->num_players, get_pkg_id(MESSAGE), death_msg, -1);
                    free(death_msg);
                }
            }
            free(active_characters);

            // Status effects kill the remaining players
            if (!game->active_players) {
                char* end_message = "\nEl monstruo ha derrotado a todos los jugadores!!! El combate ha finalizado...\n";
                notify_users(game->players, game->num_players, get_pkg_id(MESSAGE), end_message, -1);
                reset_game(game);
                return;
            }

            // Next round
            game->current_player = get_next_player(game, -1);
            game->rounds++;
            start_turn(game->players[game->current_player], game->current_player, game);
        }
    }
}

// Reset game
void reset_game(Game* game) {
    for (int p = 0; p < game->num_players; p++) {
        destroy_character(game->characters[p]);
        game->characters[p] = NULL;
    }
    destroy_character(game->monster);
    game->current_player = 0;
    game->rounds = 0;
    game->active_players = 0;

    // Ask players if they want to keep playing
    char* continue_msg = "\n¿Deseas seguir jugando?\n\n1) Sí\n2) No\n\n";
    notify_users(game->players, game->num_players, get_pkg_id(CONTINUE), continue_msg, -1);
}

// Destroy game
void destroy_game(Game* game) {
    free(game->players);
    free(game->usernames);
    free(game->characters);
    free(game);
}

// Obtain random loot as reward
void gain_loot(Game* game) {
    for (int p = 0; p < game->num_players; p++) {
        if (game->characters[p]->is_active) {
            int amount = (rand() % 3) + 3;
            int* numbers = random_loot(amount);
            for (int l = 0; l < amount; l++) {
                char* loot_path[3];
                loot_path[0] = "loot/loot";
                loot_path[1] = itoa(numbers[l]);
                loot_path[2] = ".PNG";
                char* loot_file = concatenate(loot_path, 3);
                char* img_data = calloc(51200, sizeof(char));
                int img_size = read_img(loot_file, img_data);
                server_send_image(game->players[p], get_pkg_id(IMAGE), img_data, img_size);
                free(loot_path[1]);
                free(loot_file);
                free(img_data);
            }
            char* new_loot = "\nTienes nuevo loot disponible gracias a tu victoria!\n";
            server_send_message(game->players[p], get_pkg_id(MESSAGE), new_loot);
            free(numbers);
        }
    }
}


// ----------- Networking ----------- //

// Map pkg_id to request type
Request get_request_type(int pkg_id) {
    Request type;
    switch (pkg_id) {
        case 1:
            return SET_USERNAME;
        case 2:
            return SELECT_CLASS;
        case 3:
            return START_GAME;
        case 4:
            return SELECT_MONSTER;
        case 5:
            return SELECT_ACTION;
        case 6:
            return SELECT_SKILL;
        case 7:
            return SELECT_OBJECTIVE;
        case 8:
            return CONTINUE;
        case 9:
            return MESSAGE;
    }
    return type;
}

// Map request type to pkg_id
int get_pkg_id(Request request_type) {
    switch (request_type) {
        case SET_USERNAME:
            return 1;
        case SELECT_CLASS:
            return 2;
        case START_GAME:
            return 3;
        case SELECT_MONSTER:
            return 4;
        case SELECT_ACTION:
            return 5;
        case SELECT_SKILL:
            return 6;
        case SELECT_OBJECTIVE:
            return 7;
        case CONTINUE:
            return 8;
        case MESSAGE:
            return 9;
        case IMAGE:
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
    while (running) {

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
                server_send_message(invalid_player, 0, invalid_msg);
                close(invalid_player);
            } else {  // Accept new player
                new_player = accept(server_socket, (struct sockaddr*)&client_addr, &addr_size);
                game->players[game->num_players] = new_player;
                game->num_players++;
                game->active_players++;

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
                // Package ID
                int pkg_id = server_receive_id(sd);

                // Process request
                process_request(get_request_type(pkg_id), sd, p, game);
            }
        }
    }
    destroy_game(game);
}

// Process request from client
void process_request(Request req_type, int client, int player, Game* game) {
    switch (req_type) {
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

        case SELECT_ACTION:
            select_action(client, player, game);
            break;

        case SELECT_SKILL:
            select_skill(client, player, game);
            break;

        case SELECT_OBJECTIVE:
            select_objective(client, player, game);
            break;

        case CONTINUE:
            return_to_lobby(client, player, game);
            break;

        default:
            break;
    }
}


// ----------- Handlers ----------- //

// Set username
void set_username(int client, int player, Game* game) {
    char* username = server_receive_payload(client);
    if (username == NULL) {
        char* invalid = "\nDebes ingresar algo válido!\n";
        server_send_message(client, get_pkg_id(SET_USERNAME), invalid);
        return;
    }

    game->usernames[player] = calloc(sizeof(char), strlen(username) + 1);
    strcpy(game->usernames[player], username);
    free(username);

    // Send message for class selection
    char* class = "\nIngresa tu clase:\n\n1) Cazador\n2) Médico\n3) Hacker\n\n";
    server_send_message(client, get_pkg_id(SELECT_CLASS), class);
}

// Select class
void select_class(int client, int player, Game* game) {
    char* data = server_receive_payload(client);
    if (data == NULL) {
        char* invalid = "\nDebes ingresar algo válido!\n";
        server_send_message(client, get_pkg_id(SELECT_CLASS), invalid);
        return;
    }

    int class_id = atoi(data);
    Character* new_character;
    char* selected;
    char* invalid;
    switch (class_id) {
        case 1:
            selected = "Cazador";
            new_character = create_character(HUNTER);
            break;

        case 2:
            selected = "Médico";
            new_character = create_character(MEDIC);
            break;

        case 3:
            selected = "Hacker";
            new_character = create_character(HACKER);
            break;

        default:
            invalid = "\nDebes ingresar algo válido!\n";
            server_send_message(client, get_pkg_id(SELECT_CLASS), invalid);
            free(data);
            return;
    }

    // Set player's new character
    new_character->name = game->usernames[player];
    game->characters[player] = new_character;

    // Notife action
    char* ready = " está listo para jugar como ";
    char* msg_notify[5];
    msg_notify[0] = "\n";
    msg_notify[1] = game->usernames[player];
    msg_notify[2] = ready;
    msg_notify[3] = selected;
    msg_notify[4] = "!\n";
    char* notify = concatenate(msg_notify, 5);
    server_send_message(client, get_pkg_id(MESSAGE), notify);
    if (player) {
        server_send_message(game->players[0], get_pkg_id(MESSAGE), notify);
    } else {
        char* start_msg = "\nPresiona ENTER para intentar comenzar el juego:\n";
        server_send_message(game->players[0], get_pkg_id(START_GAME), start_msg);
    }
    free(notify);
    free(data);
}

// Initialize game (leader only)
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
    game->playing = true;
    game->current_player = 0;
    char* select_monster_msg = "\nSelecciona un monstruo contra el que combatir:\n\n1) Great JagRuz\n2) Ruzalos\n3) Ruiz, el Gemelo Malvado del Profesor Ruz\n4) Monstruo Aleatorio\n\n";
    server_send_message(client, get_pkg_id(SELECT_MONSTER), select_monster_msg);
}

// Select the monster to play against
void select_monster(int client, int player, Game* game) {
    char* data = server_receive_payload(client);
    if (data == NULL) {
        char* invalid = "\nDebes ingresar algo válido!\n";
        server_send_message(client, get_pkg_id(SELECT_MONSTER), invalid);
        return;
    }

    // Chosen monster
    int monster_id = atoi(data);
    Character* monster;
    char* monster_name;
    char* invalid;
    switch (monster_id) {
        case 1:
            monster_name = "Great JagRuz";
            monster = create_character(GREAT_JAGRUZ);
            break;

        case 2:
            monster_name = "Ruzalos";
            monster = create_character(RUZALOS);
            break;

        case 3:
            monster_name = "Ruiz";
            monster = create_character(RUIZ);
            break;

        case 4:  // Random
            monster = create_character(get_random_monster());
            switch (monster->type) {
                case GREAT_JAGRUZ:
                    monster_name = "Great JagRuz";
                    break;
                case RUZALOS:
                    monster_name = "Ruzalos";
                    break;
                case RUIZ:
                    monster_name = "Ruiz";
                    break;
                default:
                    break;
            }
            break;

        default:
            invalid = "\nDebes ingresar algo válido!\n";
            server_send_message(client, get_pkg_id(SELECT_MONSTER), invalid);
            free(data);
            return;
    }

    // Set game monsters
    game->monster = monster;
    game->monster->name = monster_name;

    // Notify all game users that the game has started
    char* raw_text_0 = "\nEl juego ha comenzado! Pelearán contra el temible monstruo: ";
    char* raw_text_1 = "\n";
    char* raw_msg[3];
    raw_msg[0] = raw_text_0;
    raw_msg[1] = game->monster->name;
    raw_msg[2] = raw_text_1;
    char* start_msg = concatenate(raw_msg, 3);
    notify_users(game->players, game->num_players, get_pkg_id(MESSAGE), start_msg, -1);
    free(start_msg);
    free(data);

    // Start the first round
    start_turn(client, player, game);
}

// Start turn
void start_turn(int client, int player, Game* game) {
    // Information
    char* game_stats_raw[4 + game->active_players];

    // Build notification message
    char* game_stats_raw_header = "\n************************************************** STATS **************************************************\n";
    game_stats_raw[0] = game_stats_raw_header;

    // Get each player ability_names[selected_ability_id]
    char* players_messages[game->active_players];
    int count = 0;
    for (int p = 0; p < game->num_players; p++) {
        if (game->characters[p]->is_active) {
            char* player_raw_line[9];

            // Build character stats line
            player_raw_line[0] = game->usernames[p];
            char* raw_line_text_0 = " [";
            switch (game->characters[p]->type) {
                case HUNTER:
                    raw_line_text_0 = RED " [";
                    break;
                case MEDIC:
                    raw_line_text_0 = GREEN " [";
                    break;
                case HACKER:
                    raw_line_text_0 = CYAN " [";
                    break;
                default:
                    break;
            }
            player_raw_line[1] = raw_line_text_0;
            player_raw_line[2] = game->characters[p]->class_name;
            char* raw_line_text_1 = "] \e[0m-> VIDA: ";
            player_raw_line[3] = raw_line_text_1;
            char* current_health = itoa(game->characters[p]->current_hp);
            player_raw_line[4] = current_health;
            char* raw_line_text_2 = " / ";
            player_raw_line[5] = raw_line_text_2;
            char* max_health = itoa(game->characters[p]->max_hp);
            player_raw_line[6] = max_health;
            player_raw_line[7] = get_player_status(game->characters[p]);
            char* raw_line_text_3 = "\e[0m\n";
            player_raw_line[8] = raw_line_text_3;

            // Add line to notification message
            char* player_statistics = concatenate(player_raw_line, 9);
            players_messages[count] = player_statistics;
            game_stats_raw[count + 1] = players_messages[count];
            free(current_health);
            free(max_health);
            free(player_raw_line[7]);
            count++;
        }
    }

    // Monster stats
    game_stats_raw[1 + game->active_players] = "-----------------------------------------------------------------------------------------------------------\n";
    char* monster_stats_raw[8];
    monster_stats_raw[0] = game->monster->class_name;
    monster_stats_raw[1] = PURPLE " [MONSTRUO]";
    char* monster_raw_text_0 = " \e[0m-> VIDA: ";
    monster_stats_raw[2] = monster_raw_text_0;
    char* monster_current_hp = itoa(game->monster->current_hp);
    monster_stats_raw[3] = monster_current_hp;
    char* monster_raw_text_1 = " / ";
    monster_stats_raw[4] = monster_raw_text_1;
    char* monster_max_hp = itoa(game->monster->max_hp);
    monster_stats_raw[5] = monster_max_hp;
    monster_stats_raw[6] = get_player_status(game->monster);
    char* monster_raw_text_2 = "\e[0m\n";
    monster_stats_raw[7] = monster_raw_text_2;

    // Concatenate monster stats and add to notification
    char* monster_stats = concatenate(monster_stats_raw, 8);
    game_stats_raw[2 + game->active_players] = monster_stats;
    free(monster_current_hp);
    free(monster_max_hp);
    free(monster_stats_raw[6]);

    // Concatenate notification and notify all users
    char* game_stats_raw_footer = "***********************************************************************************************************\n\n";
    game_stats_raw[game->active_players + 3] = game_stats_raw_footer;
    char* notification = concatenate(game_stats_raw, game->active_players + 4);
    notify_users(game->players, game->num_players, get_pkg_id(MESSAGE), notification, -1);

    // Free used memory
    for (int mem = 0; mem < game->active_players; mem++) {
        free(players_messages[mem]);
    }
    free(monster_stats);
    free(notification);

    // Ask for action
    send_select_action_message(game, player);
}

// Select action
void select_action(int client, int player, Game* game) {
    char* data = server_receive_payload(client);
    if (data == NULL) {
        char* invalid = "\nDebes ingresar algo válido!\n";
        server_send_message(client, get_pkg_id(SELECT_ACTION), invalid);
        return;
    }

    int action_id = atoi(data);
    free(data);

    // Keep fighting
    if (action_id == 1) {
        send_select_skill_message(game, player);
        return;
    } else if (action_id == 2) {
        // Surrender
        game->characters[player]->is_active = false;
        game->active_players--;
        char* surrender[3];
        surrender[0] = "\n";
        surrender[1] = game->usernames[player];
        surrender[2] = " se ha rendido!\n";
        char* surrender_msg = concatenate(surrender, 3);
        notify_users(game->players, game->num_players, get_pkg_id(MESSAGE), surrender_msg, -1);
        free(surrender_msg);

        // Check game state
        check_state(game);
        return;
    }

    char* invalid = "\nDebes ingresar algo válido!\n";
    server_send_message(client, get_pkg_id(SELECT_ACTION), invalid);
}

// Select skill
void select_skill(int client, int player, Game* game) {
    char* data = server_receive_payload(client);
    if (data == NULL) {
        char* invalid = "\nDebes ingresar algo válido!\n";
        server_send_message(client, get_pkg_id(SELECT_SKILL), invalid);
        return;
    }

    // Set selected skill
    int skill_id = atoi(data);
    free(data);
    if (skill_id < 1 || skill_id > game->characters[player]->n_abilities) {
        char* invalid = "\nDebes ingresar algo válido!\n";
        server_send_message(client, get_pkg_id(SELECT_SKILL), invalid);
        return;
    }
    skill_id--;

    Ability selected_ability = get_ability(game->characters[player], skill_id);
    game->characters[player]->selected_skill_id = skill_id;

    // Ability targets an enemy
    if (game->characters[player]->enemy_target[skill_id]) {
        Character** active_characters = get_active_characters(game);
        char* notification = use_ability(get_active_index(game, player), -1, active_characters, game->active_players, game->monster, game->rounds, selected_ability);
        notify_users(game->players, game->num_players, get_pkg_id(MESSAGE), notification, -1);
        free(active_characters);
        free(notification);
        check_state(game);
        return;
    }

    // Selecting an objective
    send_select_objective_message(game, player);
}

// Select objective
void select_objective(int client, int player, Game* game) {
    char* data = server_receive_payload(client);
    if (data == NULL) {
        char* invalid = "\nDebes ingresar algo válido!\n";
        server_send_message(client, get_pkg_id(SELECT_OBJECTIVE), invalid);
        return;
    }

    // Get attacker and selected id
    Character* player_character = game->characters[player];
    Ability selected_skill = player_character->abilities[player_character->selected_skill_id];
    int objective_id = atoi(data);
    free(data);
    if (objective_id < 1 || objective_id > game->active_players) {
        char* invalid = "\nDebes ingresar algo válido!\n";
        server_send_message(client, get_pkg_id(SELECT_OBJECTIVE), invalid);
        return;
    }
    objective_id--;
    Character* defender = game->characters[objective_id];

    Character** active_characters = get_active_characters(game);
    char* message = use_ability(get_active_index(game, player), objective_id, active_characters, game->active_players, game->monster, game->rounds, selected_skill);
    notify_users(game->players, game->num_players, get_pkg_id(MESSAGE), message, -1);
    free(active_characters);
    free(message);

    // Check state of the board and pass to the next turn
    check_state(game);
}

// Continue playing or exit server
void return_to_lobby(int client, int player, Game* game) {
    char* data = server_receive_payload(client);
    if (data == NULL) {
        char* invalid = "\nDebes ingresar algo válido!\n";
        server_send_message(client, get_pkg_id(CONTINUE), invalid);
        return;
    }

    int option_id = atoi(data);
    free(data);

    // Keep playing
    if (option_id == 1) {
        game->active_players++;
        char* waiting = "\nEsperando al resto del equipo...\n";
        server_send_message(client, get_pkg_id(MESSAGE), waiting);

        // Restart lobby
        if (game->active_players == game->num_players) {
            remake_lobby(game);
        }
        return;
    } else if (option_id == 2) {

        // Exit server
        char* goodbye = "\nHasta pronto!\n";
        server_send_message(game->players[player], 0, goodbye);
        game->players[player] = 0;
        free(game->usernames[player]);
        game->num_players--;
        close(client);

        // Shutdown server
        if (!game->num_players) {
            running = false;
            return;
        } else if (!player) {  // Leader leaves
            int new_leader = get_active_socket(game, get_random_character_id(game->num_players));
            game->usernames[0] = game->usernames[new_leader];
            game->usernames[new_leader] = NULL;
            game->players[0] = game->players[new_leader];
            game->players[new_leader] = 0;
            char* leader = "\nAhora eres el líder del equipo!\n";
            server_send_message(game->players[0], get_pkg_id(MESSAGE), leader);
        }

        // Restart lobby
        if (game->active_players == game->num_players) {
            remake_lobby(game);
        }
        return;
    }

    char* invalid = "\nDebes ingresar algo válido!\n";
    server_send_message(client, get_pkg_id(CONTINUE), invalid);
}

// Restart lobby
void remake_lobby(Game* game) {
    game->monster = NULL;
    game->active_players = game->num_players;

    // Reorder remaining players
    reorder_players(game);
    char* class = "\nIngresa tu nueva clase:\n\n1) Cazador\n2) Médico\n3) Hacker\n\n";
    notify_users(game->players, game->num_players, get_pkg_id(SELECT_CLASS), class, -1);
    game->playing = false;
}

// ----------- Helper Functions ----------- //

// Send action selection message
void send_select_action_message(Game* game, int player) {
    // Notify all users except the current player
    char* raw_notification_msg[2];
    raw_notification_msg[0] = game->usernames[player];
    char* raw_not_text = " está realizando su turno...\n";
    raw_notification_msg[1] = raw_not_text;
    char* notification_msg = concatenate(raw_notification_msg, 2);
    notify_users(game->players, game->num_players, get_pkg_id(MESSAGE), notification_msg, player);
    free(notification_msg);

    // Notify current player
    char* current[2];
    current[0] = game->usernames[player];
    current[1] = ", es tu turno...\n";
    char* current_msg = concatenate(current, 2);
    server_send_message(game->players[player], get_pkg_id(MESSAGE), current_msg);
    free(current_msg);

    // Send select action to current player
    Character* current_player = game->characters[player];
    char* message = "\n¿Qué vas a hacer?:\n\n1) Luchar\n2) Rendirte\n\n";

    // Send message to user
    server_send_message(game->players[player], get_pkg_id(SELECT_ACTION), message);
}

// Send skill selection message
void send_select_skill_message(Game* game, int player) {

    // Send select skill to current player
    Character* current_player = game->characters[player];
    char* raw_select_msg[2 + current_player->n_abilities];
    char* raw_select_header = "\nElige una de las siguientes habilidades:\n\n";
    raw_select_msg[0] = raw_select_header;

    char* player_abilities[current_player->n_abilities];
    for (int a = 0; a < current_player->n_abilities; a++) {
        char* raw_ability_line[4];
        char* index = itoa(a + 1);
        raw_ability_line[0] = index;
        char* raw_ability_text_0 = ") ";
        raw_ability_line[1] = raw_ability_text_0;
        raw_ability_line[2] = get_ability_name(current_player->abilities[a]);
        char* newline = "\n";
        raw_ability_line[3] = newline;

        // Concatenate message
        char* ability_message = concatenate(raw_ability_line, 4);
        player_abilities[a] = ability_message;
        raw_select_msg[1 + a] = player_abilities[a];
        free(index);
    }
    raw_select_msg[1 + current_player->n_abilities] = "\n";

    // Send message to user
    char* select_message = concatenate(raw_select_msg, 2 + current_player->n_abilities);
    server_send_message(game->players[player], get_pkg_id(SELECT_SKILL), select_message);
    free(select_message);

    // Free memory from notification messages
    for (int a = 0; a < current_player->n_abilities; a++) {
        free(player_abilities[a]);
    }
}

// Send objective message if needed
void send_select_objective_message(Game* game, int player) {
    Character* current_character = game->characters[player];

    // Select objective menu
    char* raw_selection_menu[2 + game->active_players];
    char* raw_selection_header = "\nSelecciona un aliado:\n\n";
    raw_selection_menu[0] = raw_selection_header;
    char* objectives_lines[game->active_players];
    int count = 0;
    for (int p = 0; p < game->num_players; p++) {
        if (game->characters[p]->is_active) {
            // Parse player name and life
            char* raw_objective_line[8];
            char* index = itoa(count + 1);
            raw_objective_line[0] = index;
            char* raw_objective_text_0 = ") ";
            raw_objective_line[1] = raw_objective_text_0;
            raw_objective_line[2] = game->usernames[p];
            char* raw_objective_text_1 = " [";
            raw_objective_line[3] = raw_objective_text_1;
            char* current_health = itoa(game->characters[p]->current_hp);
            raw_objective_line[4] = current_health;
            char* raw_objective_text_2 = " / ";
            raw_objective_line[5] = raw_objective_text_2;
            char* max_health = itoa(game->characters[p]->max_hp);
            raw_objective_line[6] = max_health;
            char* raw_objective_text_3 = "]\n";
            raw_objective_line[7] = raw_objective_text_3;

            char* objective_line = concatenate(raw_objective_line, 8);
            objectives_lines[count] = objective_line;
            raw_selection_menu[1 + count] = objectives_lines[count];
            free(current_health);
            free(max_health);
            free(index);
            count++;
        }
    }
    raw_selection_menu[1 + game->active_players] = "\n";

    // Send menu to user
    char* select_message = concatenate(raw_selection_menu, 2 + game->active_players);
    server_send_message(game->players[player], get_pkg_id(SELECT_OBJECTIVE), select_message);
    free(select_message);

    // Free memory
    for (int p = 0; p < game->active_players; p++) {
        free(objectives_lines[p]);
    }
}

// The monster uses an ability on a random user
void monster_turn(Game* game) {
    // Bleeding effect
    if (game->monster->bleeding_counter) {
        char* monster_bleed[5];
        monster_bleed[0] = "MONSTRUO: ";
        monster_bleed[1] = game->monster->class_name;
        monster_bleed[2] = " perdió ";
        monster_bleed[3] = itoa(int_min(500 * game->monster->bleeding_counter, game->monster->current_hp));
        monster_bleed[4] = " puntos de vida debido al sangrado!\n";
        char* bleed_msg = concatenate(monster_bleed, 5);
        notify_users(game->players, game->num_players, get_pkg_id(MESSAGE), bleed_msg, -1);
        free(monster_bleed[3]);
        free(bleed_msg);
    }
    lose_hp(game->monster, 500 * game->monster->bleeding_counter);

    // Monster dies from bleeding
    if (!game->monster->is_active) {
        return;
    }

    // Get random ability & target
    int selected_ability_id = get_random_ability_id(game->monster);
    Ability selected_ability = game->monster->abilities[selected_ability_id];
    int defender_player_id = get_random_character_id(game->active_players);
    Character** active_characters = get_active_characters(game);
    Character* defender = active_characters[defender_player_id];

    // Monster attacks
    char* notification = use_ability(-1, defender_player_id, active_characters, game->active_players, game->monster, game->rounds, selected_ability);
    notify_users(game->players, game->num_players, get_pkg_id(MESSAGE), notification, -1);
    free(active_characters);
    free(notification);
}

// Gets the next active player
int get_next_player(Game* game, int index) {
    for (int i = index + 1; i < game->num_players; i++) {
        if (game->characters[i]->is_active) {
            return i;
        }
    }
    return -1;
}

// Get active player index
int get_active_index(Game* game, int player) {
    int count = 0;
    for (int i = 0; i < player; i++) {
        if (game->characters[i]->is_active) {
            count++;
        }
    }
    return count;
}

// Get active socket by index
int get_active_socket(Game* game, int random) {
    int count = 0;
    for (int i = 0; i < game->max_players; i++) {
        if (game->players[i]) {
            if (count == random) {
                return i;
            }
            count++;
        }
    }
    return -1;
}

// Get active characters
Character** get_active_characters(Game* game) {
    Character** active_characters = calloc(game->active_players, sizeof(Character*));
    int count = 0;
    for (int i = 0; i < game->num_players; i++) {
        if (game->characters[i]->is_active) {
            active_characters[count] = game->characters[i];
            count++;
        }
    }
    return active_characters;
}

// Reorder old players for new game
void reorder_players(Game* game) {
    int index = 0;
    int current = 0;
    while (index < game->max_players) {
        if (game->players[index] && current) {
            game->players[current] = game->players[index];
            game->players[index] = 0;
            game->usernames[current] = game->usernames[index];
            game->usernames[index] = NULL;
            index = current;
            current = 0;
        } else if (!game->players[index] && !current) {
            current = index;
        }
        index++;
    }
}

// Get random loot for a player
int* random_loot(int amount) {
    int* loot = malloc(amount * sizeof(int));
    for (int l = 0; l < amount; l++) {
        loot[l] = rand() % 10;
    }
    return loot;
}
