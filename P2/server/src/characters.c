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
            character->probabilities = malloc(sizeof(float) * 1);

            character->abilities = malloc(sizeof(Ability) * 3);
            character->abilities[0] = ESTOCADA;
            character->abilities[1] = CORTE_CRUZADO;
            character->abilities[2] = DISTRAER;

            character->max_hp = 5000;
            character->current_hp = 5000;

        case MEDIC:
            character->probabilities = malloc(sizeof(float) * 1);

            character->abilities = malloc(sizeof(Ability) * 3);
            character->abilities[0] = CURAR;
            character->abilities[1] = DESTELLO_REGENERADOR;
            character->abilities[2] = DESCARGA_VITAL;

            character->max_hp = 3000;
            character->current_hp = 3000;

        case HACKER:
            character->probabilities = malloc(sizeof(float) * 1);

            character->abilities = malloc(sizeof(Ability) * 3);
            character->abilities[0] = INYECCION_SQL;
            character->abilities[1] = ATAQUE_DDOS;
            character->abilities[2] = FUERZA_BRUTA;

            character->max_hp = 2500;
            character->current_hp = 2500;

        case GREAT_JAGRUZ:
            character->probabilities = malloc(sizeof(float) * 2);
            character->probabilities[0] = 0.5;
            character->probabilities[1] = 0.5;

            character->abilities = malloc(sizeof(Ability) * 2);
            character->abilities[0] = RUZALOS;
            character->abilities[1] = COLETAZO;

            character->max_hp = 10000;
            character->current_hp = 10000;

        case RUZALOS:
            character->probabilities = malloc(sizeof(float) * 2);
            character->probabilities[0] = 0.4;
            character->probabilities[1] = 0.6;

            character->abilities = malloc(sizeof(Ability) * 2);
            character->abilities[0] = SALTO;
            character->abilities[1] = ESPINA_VENENOSA;

            character->max_hp = 20000;
            character->current_hp = 20000;

        case RUIZ:
            character->probabilities = malloc(sizeof(float) * 3);
            character->probabilities[0] = 0.4;
            character->probabilities[1] = 0.2;
            character->probabilities[2] = 0.4;

            character->abilities = malloc(sizeof(Ability) * 3);
            character->abilities[0] = CASO_COPIA;
            character->abilities[1] = REPROBATRON_9000;
            character->abilities[2] = SUDO_RM_RF;
            character->max_hp = 25000;
            character->current_hp = 25000;
    }
    return character;
}

// Damage character
void lose_hp(Character* character, int hp) {
    character->current_hp -= hp;
    if (character->current_hp < 0) {
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

// Get chosen ability from character
Ability get_ability(Character* character, int ability_n) {
    // TODO:
    return ESTOCADA;
}

// Use character ability
void use_ability(Character* attacker, Character* defender, Ability ability) {
    // TODO:
}

// Destroy character
void destroy_character(Character* character) {
    free(character->probabilities);
    free(character->abilities);
    free(character);
}
