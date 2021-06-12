#include "characters.h"


// Create new character
Character* create_character(Class type) {
    Character* character = malloc(sizeof(Character));

    // Initialize common attributes
    character->is_active = true;
    character->attack_mult = 1;
    character->intoxicated_counter = 0;
    character->bleeding_counter = 0;
    character->failed = false;
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
Ability get_ability(Character* character, int ability_n) {
    return ESTOCADA;
}

// Use character ability
void use_ability(Character* attacker, Character* defender, Ability ability, int n_characters, Character** characters, int round) {
    int damage, hp_diff;
    switch (ability) {

        case ESTOCADA:
            lose_hp(defender, 1000 * attacker->attack_mult);

            // Apply bleeding
            if (defender->bleeding_counter < 3) {
                defender->bleeding_counter++;
            }

            break;

        case CORTE_CRUZADO:
            lose_hp(defender, 3000 * (attacker->attack_mult));
            break;

        case DISTRAER:
            defender->next_defender = attacker;
            break;

        case CURAR:
            recover_hp(defender, 2000 * (attacker->attack_mult));
            break;

        case DESTELLO_REGENERADOR:
            damage = (750 + (rand() % 1250)) * attacker->attack_mult;
            lose_hp(defender, damage);

            if (attacker->is_monster){ // If the attacker was a montser
                recover_hp(attacker, ceil(damage / 2));
            } else { // Select a random character to heal
                recover_hp(characters[rand() % n_characters], ceil(damage / 2));
            }
            break;

        case DESCARGA_VITAL:
            hp_diff = (attacker->max_hp) - (attacker->current_hp);
            lose_hp(defender, 2 * hp_diff * (attacker->attack_mult));
            break;

        case INYECCION_SQL:
            /*
            x2_mul -> attack_mul * 2 * x2_mul
            x2_count -> cantidad de boost (Solo incrementa una vez por turno)
            x2_time -> Cantidad de turnos restantes (Aumenta de 2 en 2 solo una vez
                       por turno, se reduce de 1 en 1 en cada turno)
            En apply effects
            if (x2_time % 2 == 0) {
                x2_count--
                x2_mul = x2_count
            }
            */
            break;

        case ATAQUE_DDOS:
            lose_hp(defender, 1500 * (attacker->attack_mult));
            break;

        case FUERZA_BRUTA:
            attacker->brute_force_counter++;
            if (attacker->brute_force_counter == 3) {
                lose_hp(defender, 10000 * (attacker->attack_mult));
                attacker->brute_force_counter = 0;
            }
            break;

        case RUZGAR:
            lose_hp(defender, 1000 * (attacker->attack_mult));
            break;

        case COLETAZO:
            for (int ch; ch < n_characters; ch++) {
                lose_hp(characters[ch], 500 * (attacker->attack_mult));
            }
            break;

        case SALTO:
            lose_hp(defender, 1500 * (attacker->attack_mult));

            // Block jump for the next turn
            attacker->jumped = true;
            break;

        case ESPINA_VENENOSA:

            // Check if target already intoxicated
            if (defender->intoxicated_counter > 0) {
                lose_hp(defender, 500 * (attacker->attack_mult));
            }
            defender->intoxicated_counter = 3;

            // Unblock jump for the next turn
            attacker->jumped = false;
            break;

        case CASO_COPIA:
            // TODO: Recibir como parametro array adicional de jugadores, para
            // poder escoger una de sus caracteristicas
            break;

        case REPROBATRON_9000:
            attacker->failed = true;
            attacker->attack_mult = 0.5 * attacker->attack_mult;
            break;

        case SUDO_RM_RF:
            lose_hp(attacker, (100 * round) * attacker->attack_mult);
            break;
    }
}

void apply_status_effects(Character* character) {

    // TODO: Mult_x2 counter handle

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
    free(character->probabilities);
    free(character->abilities);
    free(character);
}
