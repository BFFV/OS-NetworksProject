#include "characters.h"


// Create new character
Character* create_character(Class type) {
    Character* character = malloc(sizeof(Character));

    // Initialize common attributes and counters
    character->is_active = true;
    character->intoxicated_counter = 0;
    character->bleeding_counter = 0;

    // buffs and debuffs
    character->n_buffs = 0;
    character->failed = false;

    // special classes
    character->brute_force_counter = 0;
    character->jumped = false;

    // Define abilities for every class
    switch (type) {
        case HUNTER:
            character->probabilities = malloc(sizeof(double) * 1);

            character->abilities = malloc(sizeof(Ability) * 3);
            character->abilities[0] = ESTOCADA;
            character->abilities[1] = CORTE_CRUZADO;
            character->abilities[2] = DISTRAER;

            character->is_monster = false;
            character->max_hp = 5000;
            character->current_hp = character->max_hp;
            break;

        case MEDIC:
            character->probabilities = malloc(sizeof(double) * 1);

            character->abilities = malloc(sizeof(Ability) * 3);
            character->abilities[0] = CURAR;
            character->abilities[1] = DESTELLO_REGENERADOR;
            character->abilities[2] = DESCARGA_VITAL;

            character->is_monster = false;
            character->max_hp = 3000;
            character->current_hp = character->max_hp;
            break;

        case HACKER:
            character->probabilities = malloc(sizeof(double) * 1);

            character->abilities = malloc(sizeof(Ability) * 3);
            character->abilities[0] = INYECCION_SQL;
            character->abilities[1] = ATAQUE_DDOS;
            character->abilities[2] = FUERZA_BRUTA;

            character->is_monster = false;
            character->max_hp = 2500;
            character->current_hp = character->max_hp;
            break;

        case GREAT_JAGRUZ:
            character->probabilities = malloc(sizeof(double) * 2);
            character->probabilities[0] = 0.5;
            character->probabilities[1] = 0.5;

            character->abilities = malloc(sizeof(Ability) * 2);
            character->abilities[0] = RUZALOS;
            character->abilities[1] = COLETAZO;

            character->is_monster = true;
            character->max_hp = 10000;
            character->current_hp = character->max_hp;
            break;

        case RUZALOS:
            character->probabilities = malloc(sizeof(double) * 2);
            character->probabilities[0] = 0.4;
            character->probabilities[1] = 0.6;

            character->abilities = malloc(sizeof(Ability) * 2);
            character->abilities[0] = SALTO;
            character->abilities[1] = ESPINA_VENENOSA;

            character->is_monster = true;
            character->max_hp = 20000;
            character->current_hp = character->max_hp;
            break;

        case RUIZ:
            character->probabilities = malloc(sizeof(double) * 3);
            character->probabilities[0] = 0.4;
            character->probabilities[1] = 0.2;
            character->probabilities[2] = 0.4;

            character->abilities = malloc(sizeof(Ability) * 3);
            character->abilities[0] = CASO_COPIA;
            character->abilities[1] = REPROBATRON_9000;
            character->abilities[2] = SUDO_RM_RF;

            character->is_monster = true;
            character->max_hp = 25000;
            character->current_hp = character->max_hp;
            break;

    }
    return character;
}

// Damage character
void lose_hp(Character* character, int hp) {
    if (character->failed) {
        hp = (int) (1.5 * (float) hp);
    }
    character->current_hp -= (int) hp;
    if (character->current_hp < 0) {
        character->current_hp = 0;
        character->is_active = false;
    }
}

// Heal character
void recover_hp(Character* character, int hp) {
    character->current_hp += (int) hp;
    if (character->current_hp > character->max_hp) {
        character->current_hp = character->max_hp;
    }
}

Ability get_random_ability(Character* character) {
    double prob = ((double) rand()) / ((double) RAND_MAX);
    double current_prob = 0;
    Ability selected;
    for (int id = 0; id < character->n_abilities; id++) {
        if ((current_prob <= prob) && (prob < current_prob + character->probabilities[id])) {
            selected =  character->abilities[id];
            break;
        }
        current_prob += character->probabilities[id];
    }
    // Checks jump can be used
    if ((selected == SALTO) && (character->jumped)) {
        return ESPINA_VENENOSA;
    };
    return selected;
}

// Get chosen ability from character
Ability get_ability(Character* character, int ability_id) {
    return character->abilities[ability_id];
}

// Calculates character damage for the buff queue
double get_character_multiplier(Character* character) {

    double multiplier = 1.0;
    Buff* this_buff = character->buffs;
    for (int buff = 0; buff < character->n_buffs; buff++){
        multiplier *= this_buff->multiplier;
        this_buff = this_buff->next_buff;
    }
    if (character->failed){
        multiplier *= 0.5;
    }
    return multiplier;
};


// Use character ability
void use_ability(Character* attacker, Character* defender, Ability ability, int n_characters, Character** characters, int round) {
    int damage, hp_diff;
    switch (ability) {

        case ESTOCADA:
            lose_hp(defender, 1000 * get_character_multiplier(attacker));

            // Apply bleeding
            if (defender->bleeding_counter < 3) {
                defender->bleeding_counter++;
            }

            break;

        case CORTE_CRUZADO:
            lose_hp(defender, 3000 * get_character_multiplier(attacker));
            break;

        case DISTRAER:
            /*
            TODO:
            Si next_defender = NULL -> Pido target (usuario) o escojo uno aleatorio (monstruo)
            else:
                Uso mi habilidad en next_defender(Independiente usuario y monstruo)
            */
            defender->next_defender = attacker;
            break;

        case CURAR:
            recover_hp(defender, 2000 * get_character_multiplier(attacker));
            break;

        case DESTELLO_REGENERADOR:
            damage = (750 + (rand() % 1250)) * get_character_multiplier(attacker);
            lose_hp(defender, damage);

            if (attacker->is_monster){ // If the attacker was a montser
                recover_hp(attacker, ceil(damage / 2));
            } else { // Select a random character to heal
                recover_hp(characters[rand() % n_characters], ceil(damage / 2));
            }
            break;

        case DESCARGA_VITAL:
            hp_diff = (attacker->max_hp) - (attacker->current_hp);
            lose_hp(defender, 2 * hp_diff * get_character_multiplier(attacker));
            break;

        case INYECCION_SQL:
            add_buff(defender);
            break;

        case ATAQUE_DDOS:
            lose_hp(defender, 1500 * get_character_multiplier(attacker));
            break;

        case FUERZA_BRUTA:
            attacker->brute_force_counter++;
            if (attacker->brute_force_counter == 3) {
                lose_hp(defender, 10000 * get_character_multiplier(attacker));
                attacker->brute_force_counter = 0;
            }
            break;

        case RUZGAR:
            lose_hp(defender, 1000 * get_character_multiplier(attacker));
            break;

        case COLETAZO:
            for (int ch; ch < n_characters; ch++) {
                lose_hp(characters[ch], 500 * get_character_multiplier(attacker));
            }
            break;

        case SALTO:
            lose_hp(defender, 1500 * get_character_multiplier(attacker));

            // Block jump for the next turn
            attacker->jumped = true;
            break;

        case ESPINA_VENENOSA:

            // Check if target already intoxicated
            if (defender->intoxicated_counter > 0) {
                lose_hp(defender, 500 * get_character_multiplier(attacker));
            }
            defender->intoxicated_counter = 3;

            // Unblock jump for the next turn
            attacker->jumped = false;
            break;

        case CASO_COPIA:
            copy_ability(attacker, n_characters, characters, round);
            break;

        case REPROBATRON_9000:
            attacker->failed = true;
            break;

        case SUDO_RM_RF:
            lose_hp(attacker, (100 * round) * get_character_multiplier(attacker));
            break;
    }
}

void apply_status_effects(Character* character) {

    // Update buff counters
    Buff* this_buff = character->buffs;
    Buff* new_head  = character->buffs;
    for (int buff = 0; buff < character->n_buffs; buff++){
        this_buff->rounds--;
        if (this_buff->rounds == 0) {
            new_head = this_buff->next_buff;
        } else {
            break;
        }
        this_buff = this_buff -> next_buff;
    }

    // Intoxicated decrease counter
    if (character->intoxicated_counter > 0) {
        character->intoxicated_counter--;
        lose_hp(character, 500);
    }

    // Bleeding loss hp
    for (int i = 0; i < character->bleeding_counter; i++) {
        lose_hp(character, 500);
    }

}

// Destroy character
void destroy_character(Character* character) {

    // Clean memory from buffs
    Buff* next_buff = character->buffs;
    Buff* this_buff;
    for (int buff = 0; buff <  character->n_buffs; buff++) {
        this_buff = next_buff;
        next_buff = this_buff->next_buff;
        free(this_buff);
    }

    free(character->probabilities);
    free(character->abilities);
    free(character);
}

void add_buff(Character* character) {

    // Creates a new buff for the defender
    Buff* new_buff = malloc(sizeof(Buff));
    new_buff->rounds = 2 + 1;
    new_buff->multiplier = 2;
    new_buff->next_buff = NULL;

    // creates a new buff for the defender
    if (character->n_buffs == 0){
        character->buffs = new_buff;
        character->n_buffs++;
    } else {
        Buff* last_buff = character->buffs;
        for (int buff = 1; buff < character->n_buffs; buff++){
            last_buff = last_buff->next_buff;
        }
        last_buff->next_buff = new_buff;
        character->n_buffs++;
    }
}


void copy_ability(Character* attacker, int n_characters, Character** characters, int round) {
    // Select a random player
    int player_abl_sel = (int) (rand() % n_characters);
    // Select random ability
    int ability_ind_sel = (int) (rand() % characters[player_abl_sel]->n_abilities);
    // Select random target
    int player_def_sel = (int) (rand() % n_characters);
    Ability ab_sel = characters[player_abl_sel]->abilities[ability_ind_sel];
    // Use the selected ability
    use_ability(attacker, characters[player_def_sel], ab_sel, n_characters, characters, round);
}
