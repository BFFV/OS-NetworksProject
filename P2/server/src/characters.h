#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include "utils.h"
#pragma once


// Classes for characters & monsters
typedef enum character_class {
    HUNTER,
    MEDIC,
    HACKER,
    GREAT_JAGRUZ,
    RUZALOS,
    RUIZ,
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
    bool is_monster;    // True is character is a monster
    bool is_active;     // True is character is alive
    int current_hp;     // Current health
    int max_hp;         // Maximum health

    // Define class name and type
    char* class_name;   // Class name or monster name
    Class type;         // Class enum
    char* name;

    // Character abilities
    int n_abilities;
    Ability* abilities;
    double* probabilities;  // (Monsters) Contains the prob of each ability
    int enemy_target[3];    // Tells if the ability in pos x attacks an enemy
    int selected_skill_id;  // The next ability to be used.

    // Damage counters
    int intoxicated_counter;
    int bleeding_counter;

    // Buff and Debuff
    Buff* buffs;
    int n_buffs;
    int failed_counter;

    // Special Class: Hacker (deals 10k of damage after counter = 3)
    int brute_force_counter;

    // Monster attribute: next_attack, when hunter distract the monster
    int next_defender_id;

    // Special Class: Ruzalos (cannot jump two consecutive rounds)
    bool jumped;

};

// -------- Main Functions ---------

// Character basic methods
Character* create_character(Class type);

// Destroy methods
void destroy_character(Character* character);

// Deals damage to character
void lose_hp(Character* character, int hp);

// Heal character
void recover_hp(Character* character, int hp);

// Use a selected ability
char* use_ability(int attacker_id, int defender_id, Character** characters, int n_characters, Character* monster, int rounds, Ability ability);

// Apply/Updates long term effects
int apply_status_effects(Character** characters, int n_characters);


// ---------- Helpers -------------------

// Monster abilities and target selection
int get_random_character_id(int active_players);
int get_random_ability_id(Character* monster);

// Select a random monster to fight
Class get_random_monster();

// Abilities interactions
Ability get_ability(Character* character, int ability_id);
char* get_ability_name(Ability ability);

// Calculates the damage modifier on character
double get_character_multiplier(Character* character);

// -------------- Attack Functions ---------------

char* estocada(Character* attacker, Character* defender);

char* corte_cruzado(Character* attacker, Character* defender);

char* distraer(Character* attacker, Character* defender, int attacker_id);

char* curar(Character* healer, Character* defender);

char* destello_regenerador(Character* attacker, Character* defender, Character** characters, int n_characters);

char* descarga_vital(Character* attacker, Character* defender);

char* inyeccion_sql(Character* attacker, Character* objective);

char* ataque_ddos(Character* attacker, Character* defender);

char* brute_force_attack(Character* attacker, Character* defender);

char* ruzgar(Character* attacker, Character* defender);

char* coletazo(Character* attacker, Character** characters, int n_characters);

char* salto(Character* attacker, Character* defender);

char* espina_venenosa(Character* attacker, Character* defender);

char* caso_copia(Character* attacker, Character** characters, int n_characters, int rounds);

char* reprobaton_9000(Character* attacker, Character* defender);

char* sudo_rm_rf(Character* attacker, Character** characters, int n_characters, int rounds);