#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#pragma once


// Classes for characters & monsters
typedef enum character_class {
    HUNTER,
    MEDIC,
    HACKER,
    GREAT_JAGRUZ,
    RUZALOS,
    RUIZ
} Class;

// Attacks
typedef enum ability {
    ESTOCADA,
    CORTE_CRUZADO,
    DISTRAER,
    CURAR,
    DESTELLO_REGENERADOR,
    DESCARGA_VITAL,
    INYECCION_SQL,
    ATAQUE_DDOS,
    FUERZA_BRUTA,
    RUZGAR,
    COLETAZO,
    SALTO,
    ESPINA_VENENOSA,
    CASO_COPIA,
    REPROBATRON_9000,
    SUDO_RM_RF
} Ability;

// Model characters & monsters
typedef struct character {

    // Stats
    bool is_active;
    int current_hp;
    int max_hp;

    // Character abilities
    int n_abilities;
    Ability* abilities;
    float* probabilities;

    // Hacker duplicates attack
    int attack_mult;

    // Debuffs
    int intoxicated_counter;
    int bleeding_counter;
    bool failed;

    // Special Class: Hacker (deals 10k of damage after counter = 3)
    int brute_force_counter;

    // Special Class: Ruzalos (cannot jump two consecutive rounds)
    bool jumped;

} Character;

// Character basic methods
Character* create_character(Class type);
void lose_hp(Character* character, int hp);
void recover_hp(Character* character, int hp);

// Character interactions
Ability get_ability(Character* character, int ability_n);
void use_ability(Character* attacker, Character* defender, Ability ability);

// Destroy methods
void destroy_character(Character* character);
