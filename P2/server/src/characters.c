#include "characters.h"

// -------- Main Functions ---------

// Create new character
Character* create_character(Class type) {
    Character* character = malloc(sizeof(Character));
    character->type = type;

    // Initialize common attributes and counters
    character->is_active = true;
    character->intoxicated_counter = 0;
    character->bleeding_counter = 0;

    // buffs and debuffs
    character->n_buffs = 0;
    character->failed_counter = 0;

    // special classes
    character->brute_force_counter = 0;
    character->jumped = false;
    character->next_defender_id = 999;

    // Define abilities for every class
    switch (type) {
        case HUNTER:
            character->class_name = "CAZADOR";
            character->probabilities = malloc(sizeof(double) * 1);

            character->n_abilities = 3;
            character->abilities = malloc(sizeof(Ability) * 3);
            character->abilities[0] = ESTOCADA;
            character->abilities[1] = CORTE_CRUZADO;
            character->abilities[2] = DISTRAER;
            character->enemy_target[0] = 1;
            character->enemy_target[1] = 1;
            character->enemy_target[2] = 1;

            character->is_monster = false;
            character->max_hp = 5000;
            character->current_hp = character->max_hp;
            break;

        case MEDIC:
            character->class_name = "MÉDICO";
            character->probabilities = malloc(sizeof(double) * 1);

            character->n_abilities = 3;
            character->abilities = malloc(sizeof(Ability) * 3);
            character->abilities[0] = CURAR;
            character->abilities[1] = DESTELLO_REGENERADOR;
            character->abilities[2] = DESCARGA_VITAL;
            character->enemy_target[0] = 0;
            character->enemy_target[1] = 1;
            character->enemy_target[2] = 1;

            character->is_monster = false;
            character->max_hp = 3000;
            character->current_hp = character->max_hp;
            break;

        case HACKER:
            character->class_name = "HACKER";
            character->probabilities = malloc(sizeof(double) * 1);

            character->n_abilities = 3;
            character->abilities = malloc(sizeof(Ability) * 3);
            character->abilities[0] = INYECCION_SQL;
            character->abilities[1] = ATAQUE_DDOS;
            character->abilities[2] = FUERZA_BRUTA;
            character->enemy_target[0] = 0;
            character->enemy_target[1] = 1;
            character->enemy_target[2] = 1;

            character->is_monster = false;
            character->max_hp = 2500;
            character->current_hp = character->max_hp;
            break;

        case GREAT_JAGRUZ:
            character->class_name = "GREAT JAGRUZ";
            character->probabilities = malloc(sizeof(double) * 2);
            character->probabilities[0] = 0.5;
            character->probabilities[1] = 0.5;

            character->n_abilities = 2;
            character->abilities = malloc(sizeof(Ability) * 2);
            character->abilities[0] = RUZGAR;
            character->abilities[1] = COLETAZO;
            character->enemy_target[0] = 1;
            character->enemy_target[1] = 1;

            character->is_monster = true;
            character->max_hp = 10000;
            character->current_hp = character->max_hp;
            break;

        case RUZALOS:
            character->class_name = "RUZALOS";
            character->probabilities = malloc(sizeof(double) * 2);
            character->probabilities[0] = 0.4;
            character->probabilities[1] = 0.6;

            character->n_abilities = 2;
            character->abilities = malloc(sizeof(Ability) * 2);
            character->abilities[0] = SALTO;
            character->abilities[1] = ESPINA_VENENOSA;
            character->enemy_target[0] = 1;
            character->enemy_target[1] = 1;

            character->is_monster = true;
            character->max_hp = 20000;
            character->current_hp = character->max_hp;
            break;

        case RUIZ:
            character->class_name = "RUIZ";
            character->probabilities = malloc(sizeof(double) * 3);
            character->probabilities[0] = 0.4;
            character->probabilities[1] = 0.2;
            character->probabilities[2] = 0.4;

            character->n_abilities = 3;
            character->abilities = malloc(sizeof(Ability) * 3);
            character->abilities[0] = CASO_COPIA;
            character->abilities[1] = REPROBATRON_9000;
            character->abilities[2] = SUDO_RM_RF;
            character->enemy_target[0] = 1;
            character->enemy_target[1] = 1;
            character->enemy_target[2] = 1;

            character->is_monster = true;
            character->max_hp = 25000;
            character->current_hp = character->max_hp;
            break;

    }
    return character;
}

// Destroy character
void destroy_character(Character* character) {

    // Clean memory from buffs
    Buff* next_buff = character->buffs;
    Buff* this_buff;
    for (int buff = 0; buff < character->n_buffs; buff++) {
        this_buff = next_buff;
        next_buff = this_buff->next_buff;
        free(this_buff);
    }

    free(character->probabilities);
    free(character->abilities);
    free(character);
}

// Damage character
void lose_hp(Character* character, int hp) {
    if (character->failed_counter) {
        hp = (int)(1.5 * (float)hp);
    }
    character->current_hp -= hp;
    if (character->current_hp <= 0) {
        character->current_hp = 0;
        character->is_active = false;
    }
}

// Heal character
void recover_hp(Character* character, int hp) {
    character->current_hp += hp;
    if (character->current_hp > character->max_hp) {
        character->current_hp = character->max_hp;
    }
}

// Use character ability
char* use_ability(int attacker_id, int defender_id, Character** characters, int n_characters, int rounds, Ability ability) {
    char* message;

    // Get attacker and defender (Special case when the enemy is distracted)
    Character* attacker;
    if (attacker_id != -1) {
        attacker = characters[attacker_id];
    } else {
        attacker = characters[n_characters];
    }

    Character* defender;
    if (attacker->next_defender_id != 999) {
        if (attacker->next_defender_id != -1) {
            defender = characters[attacker->next_defender_id];
        } else {
            defender = characters[n_characters];
        }

        // reset next defender
        attacker->next_defender_id = 999;

    } else if (defender_id == -1) {
        defender = characters[n_characters];
    } else {
        defender = characters[defender_id];
    }
    printf("Attacker: %s || Defender: %s\n", attacker->name, defender->name);

    switch (ability) {

        case ESTOCADA:
            message = estocada(attacker, defender);
            break;

        case CORTE_CRUZADO:
            message = corte_cruzado(attacker, defender);
            break;

        case DISTRAER:
            message = distraer(attacker, defender, attacker_id);
            break;

        case CURAR:
            message = curar(attacker, defender);
            break;

        case DESTELLO_REGENERADOR:
            message = destello_regenerador(attacker, defender, characters, n_characters);
            break;

        case DESCARGA_VITAL:
            message = descarga_vital(attacker, defender);
            break;

        case INYECCION_SQL:
            message = inyeccion_sql(attacker, defender);
            break;

        case ATAQUE_DDOS:
            message = ataque_ddos(attacker, defender);
            break;

        case FUERZA_BRUTA:
            message = brute_force_attack(attacker, defender);
            break;

        case RUZGAR:
            message = ruzgar(attacker, defender);
            break;

        case COLETAZO:
            message = coletazo(attacker, characters, n_characters);
            break;

        case SALTO:
            message = salto(attacker, defender);
            break;

        case ESPINA_VENENOSA:
            message = espina_venenosa(attacker, defender);
            break;

        case CASO_COPIA:
            message = caso_copia(attacker, characters, n_characters, rounds);
            break;

        case REPROBATRON_9000:
            message = reprobaton_9000(attacker, defender);
            break;

        case SUDO_RM_RF:
            message = sudo_rm_rf(attacker, characters, n_characters, rounds);
            break;
    }

    return message;
}

// Proc long term ability effects
int apply_status_effects(Character** characters, int n_characters) {
    // Update active players
    int death_count = 0;

    for (int c = 0; c < n_characters; c++) {

        // Update buffs
        Buff* this_buff = characters[c]->buffs;
        Buff* new_head = characters[c]->buffs;
        for (int buff = 0; buff < characters[c]->n_buffs; buff++) {
            this_buff->rounds--;
            if (this_buff->rounds == 0) {
                new_head = this_buff->next_buff;
            } else {
                break;
            }
            this_buff = this_buff->next_buff;
        }

        // Intoxicated
        if (characters[c]->intoxicated_counter > 0) {
            characters[c]->intoxicated_counter--;
            lose_hp(characters[c], 400);
        }

        // Bleeding loss hp
        lose_hp(characters[c], 500 * characters[c]->bleeding_counter);

        // Failed status
        if (characters[c]->failed_counter) {
            characters[c]->failed_counter--;
        }

        if (!characters[c]->is_active) {
            death_count++;
        }
    }

    return death_count;
}


// ---------- Helpers -------------------

// Used to select objective when the monster attacks
int get_random_character_id(int active_players) {
    return (int)(rand() % active_players);
}

// Get random monster to fight
Class get_random_monster() {
    int chosen = rand() % 3;
    Class selected;
    switch (chosen) {
        case 0:
            return GREAT_JAGRUZ;
        case 1:
            return RUZALOS;
        case 2:
            return RUIZ;
    }
    return selected;
}

// Select a monster's ability, considering each probability
int get_random_ability_id(Character* monster) {
    double prob = ((double)rand()) / ((double)RAND_MAX);
    double current_prob = 0;
    int selected;
    for (int id = 0; id < monster->n_abilities; id++) {
        if ((current_prob <= prob) && (prob < current_prob + monster->probabilities[id])) {
            selected = id;
            break;
        }
        current_prob += monster->probabilities[id];
    }
    // Check if jump can be used
    if ((monster->abilities[selected] == SALTO) && (monster->jumped)) {
        return 1;
    };
    return selected;
}

// Get chosen ability from character
Ability get_ability(Character* character, int ability_id) {
    return character->abilities[ability_id];
}

// Gest ability name
char* get_ability_name(Ability ability) {

    char* name;
    switch (ability) {
        case ESTOCADA:
            name = "Estocada";
            break;

        case CORTE_CRUZADO:
            name = "Corte Cruzado";
            break;

        case DISTRAER:
            name = "Distraer";
            break;

        case CURAR:
            name = "Curar";
            break;

        case DESTELLO_REGENERADOR:
            name = "Destello Regenerador";
            break;

        case DESCARGA_VITAL:
            name = "Descarga Vital";
            break;

        case INYECCION_SQL:
            name = "Inyección SQL";
            break;

        case ATAQUE_DDOS:
            name = "Ataque DDOS";
            break;

        case FUERZA_BRUTA:
            name = "Fuerza Bruta";
            break;

        case RUZGAR:
            name = "Ruzgar";
            break;

        case COLETAZO:
            name = "Coletazo";
            break;

        case SALTO:
            name = "Salto";
            break;

        case ESPINA_VENENOSA:
            name = "Espina Venenosa";
            break;

        case CASO_COPIA:
            name = "Caso Copia";
            break;

        case REPROBATRON_9000:
            name = "Reprobaton 9000";
            break;

        case SUDO_RM_RF:
            name = "sudo rm -rf";
            break;
    }
    return name;
}

// Calculates character damage for the buff queue
double get_character_multiplier(Character* character) {

    double multiplier = 1.0;
    Buff* this_buff = character->buffs;
    for (int buff = 0; buff < character->n_buffs; buff++) {
        multiplier *= this_buff->multiplier;
        this_buff = this_buff->next_buff;
    }

    // Demoralized
    if (character->failed_counter) {
        multiplier *= 0.5;
    }
    return multiplier;
};


// -------------- Attack Functions ---------------

char* estocada(Character* attacker, Character* defender) {
    char* raw_message[6];

    // Deals damage to defender
    int damage = (int)(1000 * get_character_multiplier(attacker));
    lose_hp(defender, damage);

    // Apply bleeding
    if (defender->bleeding_counter < 3) {
        defender->bleeding_counter++;
    }

    // Transform damage to string
    char* str_damage = itoa(damage);

    // Write message
    raw_message[0] = attacker->name;
    raw_message[1] = " usó <Estocada> sobre ";
    raw_message[2] = defender->name;
    raw_message[3] = " restándole ";
    raw_message[4] = str_damage;
    raw_message[5] = " puntos de vida!\n";
    char* notification = concatenate(raw_message, 6);
    free(str_damage);

    return notification;
}

char* corte_cruzado(Character* attacker, Character* defender) {
    char* raw_message[6];
    int damage = (int)(3000 * get_character_multiplier(attacker));
    lose_hp(defender, damage);

    // Transform damage to string
    char* str_damage = itoa(damage);

    // Write message
    raw_message[0] = attacker->name;
    raw_message[1] = " usó <Corte Cruzado> sobre ";
    raw_message[2] = defender->name;
    raw_message[3] = " restándole ";
    raw_message[4] = str_damage;
    raw_message[5] = " puntos de vida!\n";
    char* notification = concatenate(raw_message, 6);
    free(str_damage);

    return notification;
}

char* distraer(Character* attacker, Character* defender, int attacker_id) {
    char* raw_message[4];
    defender->next_defender_id = attacker_id;

    // Write message
    raw_message[0] = attacker->name;
    raw_message[1] = " usó <Distraer> sobre ";
    raw_message[2] = defender->name;
    raw_message[3] = " su próxima habilidad será redirigida!\n";
    char* notification = concatenate(raw_message, 4);

    return notification;
}

char* curar(Character* healer, Character* defender) {
    char* raw_message[6];
    int heal = (int)(2000 * get_character_multiplier(healer));
    if (healer->is_monster) {
        defender = healer;
    }
    recover_hp(defender, heal);

    // Transform heal to string
    char* str_heal = itoa(heal);

    // Write message
    raw_message[0] = healer->name;
    raw_message[1] = " usó <Curar> sobre ";
    raw_message[2] = defender->name;
    raw_message[3] = " recuperándole ";
    raw_message[4] = str_heal;
    raw_message[5] = " puntos de vida!\n";
    char* notification = concatenate(raw_message, 6);
    free(str_heal);

    return notification;
}

char* destello_regenerador(Character* attacker, Character* defender, Character** characters, int n_characters) {
    char* raw_message[10];

    // calculates damage and heal
    int damage = (int)((750 + rand() % 1250) * get_character_multiplier(attacker));
    int heal = div_ceil(damage, 2);

    lose_hp(defender, damage);
    Character* objective;
    if (attacker->is_monster) {
        objective = attacker;
    } else { // Select a random character to heal
        objective = characters[rand() % n_characters];
    }
    recover_hp(objective, heal);

    // Transform damage and heal to string
    char* str_damage = itoa(damage);
    char* str_heal = itoa(heal);

    // Write message
    raw_message[0] = attacker->name;
    raw_message[1] = " usó <Destello Regenerador> sobre ";
    raw_message[2] = defender->name;
    raw_message[3] = " quitándole ";
    raw_message[4] = str_damage;
    raw_message[5] = " y sanando a ";
    raw_message[6] = objective->name;
    raw_message[7] = " ";
    raw_message[8] = str_heal;
    raw_message[9] = " puntos de vida!\n";

    char* notification = concatenate(raw_message, 10);
    free(str_damage);
    free(str_heal);

    return notification;
}

char* descarga_vital(Character* attacker, Character* defender) {
    char* raw_message[6];
    int hp_diff = (attacker->max_hp) - (attacker->current_hp);
    int damage = (int)(2 * hp_diff * get_character_multiplier(attacker));
    lose_hp(defender, damage);

    // Transform damage to string
    char* str_damage = itoa(damage);

    // Write message
    raw_message[0] = attacker->name;
    raw_message[1] = " usó <Descarga Vital> y libreró todo su daño sobre ";
    raw_message[2] = defender->name;
    raw_message[3] = " restándole ";
    raw_message[4] = str_damage;
    raw_message[5] = " puntos de vida!\n";
    char* notification = concatenate(raw_message, 6);
    free(str_damage);

    return notification;
}

char* inyeccion_sql(Character* attacker, Character* objective) {
    char* raw_message[4];

    if (attacker->is_monster) {
        objective = attacker;
    }

    // Creates a new buff for the defender
    Buff* new_buff = malloc(sizeof(Buff));
    new_buff->rounds = 2;  // Buff considers this round
    new_buff->multiplier = 2;
    new_buff->next_buff = NULL;

    // Creates a new buff for the defender
    if (objective->n_buffs == 0) {
        objective->buffs = new_buff;
        objective->n_buffs++;
    } else {
        Buff* last_buff = objective->buffs;
        for (int buff = 1; buff < objective->n_buffs; buff++) {
            last_buff = last_buff->next_buff;
        }
        last_buff->next_buff = new_buff;
        objective->n_buffs++;
    }

    // Write message
    raw_message[0] = attacker->name;
    raw_message[1] = " usó <Inyección SQL> y sobrecargo a ";
    raw_message[2] = objective->name;
    raw_message[3] = " duplicando su daño total!\n";
    char* notification = concatenate(raw_message, 4);

    return notification;
}

char* ataque_ddos(Character* attacker, Character* defender) {
    char* raw_message[6];
    int damage = (int)(1500 * get_character_multiplier(attacker));
    lose_hp(defender, damage);

    // Transform damage to string
    char* str_damage = itoa(damage);

    // Write message
    raw_message[0] = attacker->name;
    raw_message[1] = " usó <Ataque DDOS> sobre ";
    raw_message[2] = defender->name;
    raw_message[3] = " restándole ";
    raw_message[4] = str_damage;
    raw_message[5] = " puntos de vida!\n";
    char* notification = concatenate(raw_message, 6);
    free(str_damage);

    return notification;
}

char* brute_force_attack(Character* attacker, Character* defender) {
    char* raw_message[6];
    char* notification;

    attacker->brute_force_counter++;
    if (attacker->brute_force_counter == 3) {
        int damage = (int)(10000 * get_character_multiplier(attacker));
        lose_hp(defender, damage);
        attacker->brute_force_counter = 0;

        // Damage to string
        char* str_damage = itoa(damage);

        // Write message
        raw_message[0] = attacker->name;
        raw_message[1] = " logró hacer un <Brute Force Attack> sobre ";
        raw_message[2] = defender->name;
        raw_message[3] = " quitándole ";
        raw_message[4] = str_damage;
        raw_message[5] = " puntos de vida!\n";
        notification = concatenate(raw_message, 6);
        free(str_damage);

    } else {

        // Get string counter
        char* counter = itoa(attacker->brute_force_counter);

        // Write message
        raw_message[0] = attacker->name;
        raw_message[1] = " intentó hacer un Brute Force Attack sobre ";
        raw_message[2] = defender->name;
        raw_message[3] = " y acumuló un nuevo intento (Contador:";
        raw_message[4] = counter;
        raw_message[5] = ")!\n";
        notification = concatenate(raw_message, 6);
        free(counter);
    }
    return notification;
}

char* ruzgar(Character* attacker, Character* defender) {
    char* raw_message[6];
    int damage = (int)(1000 * get_character_multiplier(attacker));
    lose_hp(defender, damage);

    // Transform damage to string
    char* str_damage = itoa(damage);

    // Write message
    raw_message[0] = attacker->name;
    raw_message[1] = " usó <Ruzgar> sobre ";
    raw_message[2] = defender->name;
    raw_message[3] = " restándole ";
    raw_message[4] = str_damage;
    raw_message[5] = " puntos de vida!\n";
    char* notification = concatenate(raw_message, 6);
    free(str_damage);

    return notification;
}

char* coletazo(Character* attacker, Character** characters, int n_characters) {
    char* raw_message[4];
    int damage = (int)(500 * get_character_multiplier(attacker));
    for (int ch = 0; ch < n_characters; ch++) {
        lose_hp(characters[ch], damage);
    }

    // Transform damage to string
    char* str_damage = itoa(damage);

    // Write message
    raw_message[0] = attacker->name;
    raw_message[1] = " usó <COLETAZO> restando ";
    raw_message[2] = str_damage;
    raw_message[3] = " puntos de vida a todos los jugadores!\n";
    char* notification = concatenate(raw_message, 4);
    free(str_damage);

    return notification;
}

char* salto(Character* attacker, Character* defender) {
    char* raw_message[6];
    int damage = (int)(1500 * get_character_multiplier(attacker));
    lose_hp(defender, damage);

    // Block jump for the next turn
    attacker->jumped = true;

    // Transform damage to string
    char* str_damage = itoa(damage);

    // Write message
    raw_message[0] = attacker->name;
    raw_message[1] = " usó <Salto> sobre ";
    raw_message[2] = defender->name;
    raw_message[3] = " restándole ";
    raw_message[4] = str_damage;
    raw_message[5] = " puntos de vida!\n";
    char* notification = concatenate(raw_message, 6);
    free(str_damage);

    return notification;
}

char* espina_venenosa(Character* attacker, Character* defender) {
    char* notification;

    // Check if target already intoxicated
    if (defender->intoxicated_counter > 0) {
        int damage = (int)(500 * get_character_multiplier(attacker));
        lose_hp(defender, damage);

        // Transform damage to string
        char* str_damage = itoa(damage);

        // Write message
        char* raw_message[6];
        raw_message[0] = attacker->name;
        raw_message[1] = " usó <Espina Venenosa> sobre ";
        raw_message[2] = defender->name;
        raw_message[3] = " restándole ";
        raw_message[4] = str_damage;
        raw_message[5] = " puntos de vida!\n";
        notification = concatenate(raw_message, 6);
        free(str_damage);

    } else {
        defender->intoxicated_counter = 3;

        // Write message
        char* raw_message[4];
        raw_message[0] = attacker->name;
        raw_message[1] = " usó <Espina Venenosa> sobre ";
        raw_message[2] = defender->name;
        raw_message[3] = " y lo intoxicó por los próximos 3 turnos \n";
        notification = concatenate(raw_message, 4);

    }
    attacker->jumped = false;
    return notification;
}

// TODO: Healing only for the monster, show chosen abilities
char* caso_copia(Character* attacker, Character** characters, int n_characters, int rounds) {
    char* notification_message[2];

    // Select a random player and ability
    int player_abl_sel = (int)(rand() % n_characters);
    int ability_ind_sel = (int)(rand() % characters[player_abl_sel]->n_abilities);

    // Select random target
    int defender_id_sel = (int)(rand() % n_characters);
    Ability ab_sel = characters[player_abl_sel]->abilities[ability_ind_sel];


    char* raw_message[6];
    raw_message[0] = attacker->name;
    raw_message[1] = " usó <Caso Copia>. Copió la habilidad ";
    raw_message[2] = get_ability_name(ab_sel);
    raw_message[3] = " del jugador ";
    raw_message[4] = characters[defender_id_sel]->name;
    raw_message[5] = "!\n";
    notification_message[0] = concatenate(raw_message, 6);

    // Use the selected ability
    notification_message[1] = use_ability(-1, defender_id_sel, characters, n_characters, rounds, ab_sel);
    char* notification = concatenate(notification_message, 2);
    free(notification_message[0]);
    free(notification_message[1]);
    return notification;
}

char* reprobaton_9000(Character* attacker, Character* defender) {
    char* raw_message[5];
    defender->failed_counter = 2;

    raw_message[0] = attacker->name;
    raw_message[1] = " usó <Reprobaton 9000> contra ";
    raw_message[2] = defender->name;
    raw_message[3] = "!\nRecibirá 50\% de daño extra y sus ataques";
    raw_message[4] = " harán un 50\% menos de daño";

    char* message = concatenate(raw_message, 5);
    return message;
}

char* sudo_rm_rf(Character* attacker, Character** characters, int n_characters, int rounds) {
    char* raw_message[6];
    for (int ch = 0; ch < n_characters; ch++) {
        lose_hp(characters[ch], (int)(100 * rounds * get_character_multiplier(attacker)));
    }
    raw_message[0] = attacker->name;
    raw_message[1] = " usó <sudo rm -rf>. Todos recibirán 100 de daño por cada ronda. Se han jugado ";
    raw_message[2] = itoa(rounds);
    raw_message[3] = "!\nPor lo tanto recibirán ";
    raw_message[4] = itoa(100 * rounds * get_character_multiplier(attacker));
    raw_message[5] = " de daño!\n";
    char* message = concatenate(raw_message, 6);
    free(raw_message[2]);
    free(raw_message[4]);
    return message;
}
