#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
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
struct character;
typedef struct character Character;
struct character {

    // Stats
    bool is_monster;
    bool is_active;
    int current_hp;
    int max_hp;

    // Character abilities
    int n_abilities;
    Ability* abilities;
    double* probabilities;

    // Hacker duplicates attack
    double attack_mult;

    // Debuffs
    int intoxicated_counter;
    int bleeding_counter;
    // int x2_attack_counter[128];
    // int n_buffs;
    bool failed;

    // Special Class: Hacker (deals 10k of damage after counter = 3)
    int brute_force_counter;

    // Monster attribute: next_attack, when hunter distract the monster
    Character* next_defender;

    // Special Class: Ruzalos (cannot jump two consecutive rounds)
    bool jumped;

};

// Character basic methods
Character* create_character(Class type);
void lose_hp(Character* character, int hp);
void recover_hp(Character* character, int hp);

// Character interactions
Ability get_random_ability(Character* character);
Ability get_ability(Character* character, int ability_n);
void use_ability(Character* attacker, Character* defender, Ability ability, int n_characters, Character** characters, int round);
void apply_status_effects(Character* character);

// Destroy methods
void destroy_character(Character* character);
