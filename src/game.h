#ifndef STAR_FORGE_GAME_H
#define STAR_FORGE_GAME_H

#include <stdbool.h>

#define SF_MAX_NPCS 8
#define SF_MAX_DROPS 64
#define SF_MAX_PROJECTILES 96
#define SF_ITEM_COUNT 5

typedef enum {
    SF_ITEM_METAL,
    SF_ITEM_FUEL,
    SF_ITEM_FOOD,
    SF_ITEM_MEDICINE,
    SF_ITEM_WEAPON
} SfItem;

typedef enum {
    SF_ROLE_CARGO,
    SF_ROLE_PIRATE,
    SF_ROLE_PATROL
} SfNpcRole;

typedef struct {
    float x;
    float y;
    float vx;
    float vy;
    float angle;
    float hp;
    float max_hp;
    float fire_cooldown;
    int cargo[SF_ITEM_COUNT];
    SfNpcRole role;
    bool alive;
} SfShip;

typedef struct {
    float x;
    float y;
    float vx;
    float vy;
    float life;
    bool active;
    bool from_player;
} SfProjectile;

typedef struct {
    float x;
    float y;
    float vx;
    float vy;
    int item;
    int quantity;
    bool active;
} SfCargoDrop;

typedef struct {
    bool thrust;
    bool reverse;
    bool turn_left;
    bool turn_right;
    bool fire;
} SfInput;

typedef struct {
    SfShip player;
    SfShip npcs[SF_MAX_NPCS];
    SfCargoDrop drops[SF_MAX_DROPS];
    SfProjectile projectiles[SF_MAX_PROJECTILES];
    int inventory[SF_ITEM_COUNT];
    int selected_item;
    int selected_quantity;
    int credits;
    bool npc_salvage_enabled;
    float station_x;
    float station_y;
    unsigned int rng;
    unsigned int destroyed_npcs;
} SfGame;

void sf_game_init(SfGame *game);
void sf_game_update(SfGame *game, const SfInput *input, float dt);
void sf_game_fire_player(SfGame *game);

bool sf_game_collect_nearest(SfGame *game);
bool sf_game_jettison_selected(SfGame *game);
bool sf_game_sell_selected(SfGame *game);

float sf_distance_squared(float ax, float ay, float bx, float by);
const char *sf_item_name(int item);
int sf_item_price(int item);

#endif
