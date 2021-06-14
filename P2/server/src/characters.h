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

// Define buff struct for sequential buffing or debuffing
struct buff;
typedef struct buff Buff;
struct buff {
    int rounds;
    double multiplier;
    Buff* next_buff;
};

// Model characters & monsters
struct character;
typedef struct character Character;
struct character {

    // Stats
    bool is_monster;
    bool is_active;
    int current_hp;
    int max_hp;
    
    // Define class name and type
    char* class_name;
    Class type;

    // Character abilities
    int n_abilities;
    Ability* abilities;
    double* probabilities;
    char* ability_names[3];
    int enemy_target[3];
    int selected_skill_id;

    // Damage counters
    int intoxicated_counter;
    int bleeding_counter;

    // Buff and Debuff
    Buff* buffs;
    int n_buffs;
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

// Monster abilities and target selection
int get_random_character_id(int n_characters);
int get_random_ability_id(Character* monster);

// Character interactions
Ability get_ability(Character* character, int ability_id);
void use_ability(Character* attacker, Character* defender, Ability ability, int n_characters, Character** characters, int round);
double get_character_multiplier(Character* character);
void apply_status_effects(Character** characters, int n_characters);

// Destroy methods
void destroy_character(Character* character);

// Aux methods
void add_buff(Character* character);
void copy_ability(Character* attacker, int n_characters, Character** characters, int round);
Class get_random_monster();
