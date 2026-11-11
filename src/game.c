#include "game.h"

#include <math.h>
#include <string.h>
#include <stddef.h>
#include <limits.h>

#define SF_PI 3.14159265358979323846f
#define SF_WORLD_SIZE 2400.0f
#define SF_ASTEROID_COUNT 8
#define SF_PLAYER_RADIUS 12.0f
#define SF_FTL_FUEL_COST 5
#define SF_WEAPON_ENERGY_COST 8.0f
#define SF_ENERGY_REGEN_RATE 12.0f

static const float asteroid_x[SF_ASTEROID_COUNT] = {
    520.0f, 680.0f, 860.0f, 1040.0f, 1280.0f, 1510.0f, 1740.0f, 1980.0f
};

static const float asteroid_y[SF_ASTEROID_COUNT] = {
    250.0f, 620.0f, 430.0f, 890.0f, 510.0f, 1120.0f, 760.0f, 1380.0f
};

static const float asteroid_radius[SF_ASTEROID_COUNT] = {
    22.0f, 30.0f, 18.0f, 34.0f, 25.0f, 28.0f, 20.0f, 32.0f
};


static float clampf(float value, float minimum, float maximum)
{
    if (value < minimum) {
        return minimum;
    }

    if (value > maximum) {
        return maximum;
    }

    return value;
}

float sf_distance_squared(float ax, float ay, float bx, float by)
{
    const float dx = ax - bx;
    const float dy = ay - by;

    return dx * dx + dy * dy;
}

static unsigned int random_next(SfGame *game)
{
    unsigned int value = game->rng;

    value ^= value << 13;
    value ^= value >> 17;
    value ^= value << 5;

    game->rng = value;
    return value;
}

static float random_range(SfGame *game, float minimum, float maximum)
{
    const float unit = (float)(random_next(game) & 0x00FFFFFFu) / 16777215.0f;

    return minimum + (maximum - minimum) * unit;
}

const char *sf_item_name(int item)
{
    static const char *names[SF_ITEM_COUNT] = {
        "Metal",
        "Fuel",
        "Food",
        "Medicine",
        "Weapon"
    };

    if (item < 0 || item >= SF_ITEM_COUNT) {
        return "Unknown";
    }

    return names[item];
}

int sf_item_price(int item)
{
    static const int prices[SF_ITEM_COUNT] = {
        8, 14, 5, 22, 75
    };

    if (item < 0 || item >= SF_ITEM_COUNT) {
        return 0;
    }

    return prices[item];
}

static int cargo_used(const int cargo[SF_ITEM_COUNT])
{
    int total = 0;
    int item;

    for (item = 0; item < SF_ITEM_COUNT; ++item) {
        const int volume_per_item = item == SF_ITEM_WEAPON ? 2 : 1;
        total += cargo[item] * volume_per_item;
    }

    return total;
}

static void resolve_asteroid_collisions(SfShip *ship, int system_index)
{
    int i;

    if (!sf_game_system_has_asteroids(system_index)) {
        return;
    }

    for (i = 0; i < SF_ASTEROID_COUNT; ++i) {
        const float dx = ship->x - asteroid_x[i];
        const float dy = ship->y - asteroid_y[i];
        const float minimum_distance = asteroid_radius[i] + SF_PLAYER_RADIUS;
        const float distance_squared = dx * dx + dy * dy;

        if (distance_squared < minimum_distance * minimum_distance) {
            float distance = sqrtf(distance_squared);
            float normal_x = dx;
            float normal_y = dy;

            if (distance < 0.001f) {
                normal_x = 1.0f;
                normal_y = 0.0f;
                distance = 1.0f;
            }

            ship->x = asteroid_x[i] + (normal_x / distance) * minimum_distance;
            ship->y = asteroid_y[i] + (normal_y / distance) * minimum_distance;
            ship->vx *= -0.25f;
            ship->vy *= -0.25f;
        }
    }
}

static void spawn_cargo_drop(
    SfGame *game,
    float x,
    float y,
    int item,
    int quantity
)
{
    int i;

    if (quantity <= 0 || item < 0 || item >= SF_ITEM_COUNT) {
        return;
    }

    for (i = 0; i < SF_MAX_DROPS; ++i) {
        SfCargoDrop *drop = &game->drops[i];

        if (!drop->active) {
            drop->active = true;
            drop->x = x + random_range(game, -12.0f, 12.0f);
            drop->y = y + random_range(game, -12.0f, 12.0f);
            drop->vx = random_range(game, -18.0f, 18.0f);
            drop->vy = random_range(game, -18.0f, 18.0f);
            drop->item = item;
            drop->quantity = quantity;
            return;
        }
    }
}

static void destroy_npc_ship(SfGame *game, SfShip *ship)
{
    int item;

    for (item = 0; item < SF_ITEM_COUNT; ++item) {
        if (ship->cargo[item] > 0) {
            spawn_cargo_drop(
                game,
                ship->x,
                ship->y,
                item,
                ship->cargo[item]
            );

            ship->cargo[item] = 0;
        }
    }

    /* Installed weapons are valuable salvage, but may not survive destruction. */
    if ((random_next(game) % 100u) < 35u) {
        spawn_cargo_drop(game, ship->x, ship->y, SF_ITEM_WEAPON, 1);
    }

    ship->alive = false;
    game->destroyed_npcs += 1u;
}

static bool spawn_projectile(
    SfGame *game,
    float x,
    float y,
    float angle,
    bool from_player,
    float speed
)
{
    int i;

    for (i = 0; i < SF_MAX_PROJECTILES; ++i) {
        SfProjectile *projectile = &game->projectiles[i];

        if (!projectile->active) {
            projectile->active = true;
            projectile->x = x;
            projectile->y = y;
            projectile->vx = cosf(angle) * speed;
            projectile->vy = sinf(angle) * speed;
            projectile->life = 1.8f;
            projectile->from_player = from_player;
            return true;
        }
    }

    return false;
}

void sf_game_fire_player(SfGame *game)
{
    SfShip *ship;

    if (game == NULL) {
        return;
    }

    ship = &game->player;

    if (ship->fire_cooldown > 0.0f ||
        ship->energy < ship->weapon_energy_cost) {
        return;
    }

    if (!spawn_projectile(
            game,
            ship->x + cosf(ship->angle) * 18.0f,
            ship->y + sinf(ship->angle) * 18.0f,
            ship->angle,
            true,
            360.0f
        )) {
        return;
    }

    ship->energy -= ship->weapon_energy_cost;
    ship->fire_cooldown = 0.22f;
}


static void apply_ship_damage(SfShip *ship, float damage)
{
    if (damage <= 0.0f || !ship->alive) {
        return;
    }

    if (ship->armor > 0.0f) {
        const float absorbed = damage < ship->armor ? damage : ship->armor;

        ship->armor -= absorbed;
        damage -= absorbed;
    }

    if (damage > 0.0f) {
        ship->hull -= damage;
    }

    if (ship->hull <= 0.0f) {
        ship->hull = 0.0f;
        ship->alive = false;
    }
}

static void initialize_system_npcs(SfGame *game)
{
    SfShip *cargo_ship = &game->npcs[0];
    SfShip *pirate_ship = &game->npcs[1];

    memset(game->npcs, 0, sizeof(game->npcs));

    cargo_ship->alive = true;
    cargo_ship->role = SF_ROLE_CARGO;
    cargo_ship->x = 720.0f;
    cargo_ship->y = 360.0f;
    cargo_ship->angle = SF_PI;
    cargo_ship->armor = 20.0f;
    cargo_ship->max_armor = 20.0f;
    cargo_ship->hull = 65.0f;
    cargo_ship->max_hull = 65.0f;
    cargo_ship->energy = 60.0f;
    cargo_ship->max_energy = 60.0f;
    cargo_ship->weapon_energy_cost = 10.0f;
    cargo_ship->has_ftl_drive = true;
    cargo_ship->cargo[SF_ITEM_FUEL] = 60;
    cargo_ship->cargo[SF_ITEM_FOOD] = 24;
    cargo_ship->cargo[SF_ITEM_METAL] = 16;

    pirate_ship->alive = true;
    pirate_ship->role = SF_ROLE_PIRATE;
    pirate_ship->x = 910.0f;
    pirate_ship->y = 340.0f;
    pirate_ship->angle = SF_PI;
    pirate_ship->armor = 35.0f;
    pirate_ship->max_armor = 35.0f;
    pirate_ship->hull = 85.0f;
    pirate_ship->max_hull = 85.0f;
    pirate_ship->energy = 100.0f;
    pirate_ship->max_energy = 100.0f;
    pirate_ship->weapon_energy_cost = 10.0f;
    pirate_ship->has_ftl_drive = true;
    pirate_ship->cargo[SF_ITEM_FUEL] = 8;
}

bool sf_game_jump_to_system(SfGame *game, int destination_system)
{
    if (game == NULL ||
        !game->player.alive ||
        !game->player.has_ftl_drive ||
        destination_system < 0 ||
        destination_system >= SF_MAX_SYSTEMS ||
        destination_system == game->current_system ||
        game->inventory[SF_ITEM_FUEL] < SF_FTL_FUEL_COST) {
        return false;
    }

    game->inventory[SF_ITEM_FUEL] -= SF_FTL_FUEL_COST;
    game->current_system = destination_system;

    game->player.x = 300.0f;
    game->player.y = 300.0f;
    game->player.vx = 0.0f;
    game->player.vy = 0.0f;

    game->station_x = sf_game_system_station_x(destination_system);
    game->station_y = sf_game_system_station_y(destination_system);

    memset(game->npcs, 0, sizeof(game->npcs));
    initialize_system_npcs(game);
    memset(game->drops, 0, sizeof(game->drops));
    memset(game->projectiles, 0, sizeof(game->projectiles));

    return true;
}

void sf_game_init(SfGame *game)
{
    if (game == NULL) {
        return;
    }

    memset(game, 0, sizeof(*game));

    game->rng = 0x51A7F04Du;

    game->player.x = 300.0f;
    game->player.y = 300.0f;
    game->player.angle = 0.0f;
    game->player.armor = 60.0f;
    game->player.max_armor = 60.0f;
    game->player.hull = 100.0f;
    game->player.max_hull = 100.0f;
    game->player.energy = 100.0f;
    game->player.max_energy = 100.0f;
    game->player.weapon_energy_cost = SF_WEAPON_ENERGY_COST;
    game->player.has_ftl_drive = true;
    game->player.alive = true;

    game->inventory[SF_ITEM_METAL] = 12;
    game->inventory[SF_ITEM_FUEL] = 18;
    game->selected_item = SF_ITEM_METAL;
    game->selected_quantity = 5;
    game->credits = 120;
    game->current_system = 0;
    game->npc_salvage_enabled = true;
    game->station_x = sf_game_system_station_x(0);
    game->station_y = sf_game_system_station_y(0);

    initialize_system_npcs(game);
}

static void update_player(SfGame *game, const SfInput *input, float dt)
{
    SfShip *ship = &game->player;

    if (input->turn_left) {
        ship->angle -= 2.8f * dt;
    }

    if (input->turn_right) {
        ship->angle += 2.8f * dt;
    }

    if (ship->angle > SF_PI) {
        ship->angle -= 2.0f * SF_PI;
    }

    if (ship->angle < -SF_PI) {
        ship->angle += 2.0f * SF_PI;
    }

    {
        const bool boosting = input->boost;
        const float acceleration = boosting ? 340.0f : 190.0f;
        const float maximum_speed = boosting ? 270.0f : 190.0f;

        if (input->thrust) {
            ship->vx += cosf(ship->angle) * acceleration * dt;
            ship->vy += sinf(ship->angle) * acceleration * dt;
        }

        if (input->reverse) {
            ship->vx -= cosf(ship->angle) * 123.5f * dt;
            ship->vy -= sinf(ship->angle) * 123.5f * dt;
        }

        ship->vx = clampf(
            ship->vx * powf(0.985f, dt * 60.0f),
            -maximum_speed,
            maximum_speed
        );
        ship->vy = clampf(
            ship->vy * powf(0.985f, dt * 60.0f),
            -maximum_speed,
            maximum_speed
        );
    }

    ship->x = clampf(ship->x + ship->vx * dt, 24.0f, SF_WORLD_SIZE - 24.0f);
    ship->y = clampf(ship->y + ship->vy * dt, 24.0f, SF_WORLD_SIZE - 24.0f);

    resolve_asteroid_collisions(ship, game->current_system);

    if (ship->fire_cooldown > 0.0f) {
        ship->fire_cooldown -= dt;
    }

    ship->energy += SF_ENERGY_REGEN_RATE * dt;
    if (ship->energy > ship->max_energy) {
        ship->energy = ship->max_energy;
    }

    if (input->fire) {
        sf_game_fire_player(game);
    }
}

static float wrap_angle(float angle)
{
    while (angle > SF_PI) {
        angle -= 2.0f * SF_PI;
    }

    while (angle < -SF_PI) {
        angle += 2.0f * SF_PI;
    }

    return angle;
}

static void update_pirate(SfGame *game, SfShip *ship, float dt)
{
    SfShip *target = game->npcs[0].alive
        ? &game->npcs[0]
        : &game->player;

    const float dx = target->x - ship->x;
    const float dy = target->y - ship->y;
    const float distance = sqrtf(dx * dx + dy * dy);
    const float desired_angle = atan2f(dy, dx);
    const float angle_delta = wrap_angle(desired_angle - ship->angle);

    ship->angle += clampf(angle_delta, -1.7f * dt, 1.7f * dt);

    if (distance > 170.0f) {
        ship->vx += cosf(ship->angle) * 85.0f * dt;
        ship->vy += sinf(ship->angle) * 85.0f * dt;
    } else if (distance < 100.0f) {
        ship->vx -= cosf(ship->angle) * 45.0f * dt;
        ship->vy -= sinf(ship->angle) * 45.0f * dt;
    }

    if (distance < 330.0f &&
        ship->fire_cooldown <= 0.0f &&
        ship->energy >= ship->weapon_energy_cost) {
        if (spawn_projectile(
                game,
                ship->x + cosf(ship->angle) * 17.0f,
                ship->y + sinf(ship->angle) * 17.0f,
                ship->angle,
                false,
                230.0f
            )) {
            ship->energy -= ship->weapon_energy_cost;
            ship->fire_cooldown = 0.85f;
        }
    }
}

static void update_cargo_ship(SfGame *game, SfShip *ship, float dt)
{
    const float dx = game->station_x - ship->x;
    const float dy = game->station_y - ship->y;
    const float desired_angle = atan2f(dy, dx);
    const float angle_delta = wrap_angle(desired_angle - ship->angle);

    ship->angle += clampf(angle_delta, -0.8f * dt, 0.8f * dt);
    ship->vx += cosf(ship->angle) * 28.0f * dt;
    ship->vy += sinf(ship->angle) * 28.0f * dt;
}

static void update_npcs(SfGame *game, float dt)
{
    int i;

    for (i = 0; i < SF_MAX_NPCS; ++i) {
        SfShip *ship = &game->npcs[i];

        if (!ship->alive) {
            continue;
        }

        if (ship->role == SF_ROLE_PIRATE) {
            update_pirate(game, ship, dt);
        } else {
            update_cargo_ship(game, ship, dt);
        }

        ship->vx = clampf(ship->vx * powf(0.992f, dt * 60.0f), -105.0f, 105.0f);
        ship->vy = clampf(ship->vy * powf(0.992f, dt * 60.0f), -105.0f, 105.0f);

        ship->x = clampf(ship->x + ship->vx * dt, 24.0f, SF_WORLD_SIZE - 24.0f);
        ship->y = clampf(ship->y + ship->vy * dt, 24.0f, SF_WORLD_SIZE - 24.0f);

        resolve_asteroid_collisions(ship, game->current_system);

        if (ship->fire_cooldown > 0.0f) {
            ship->fire_cooldown -= dt;
        }

        ship->energy += SF_ENERGY_REGEN_RATE * dt;
        if (ship->energy > ship->max_energy) {
            ship->energy = ship->max_energy;
        }
    }
}

static void update_projectiles(SfGame *game, float dt)
{
    int i;

    for (i = 0; i < SF_MAX_PROJECTILES; ++i) {
        SfProjectile *projectile = &game->projectiles[i];

        if (!projectile->active) {
            continue;
        }

        projectile->x += projectile->vx * dt;
        projectile->y += projectile->vy * dt;
        projectile->life -= dt;

        if (projectile->life <= 0.0f) {
            projectile->active = false;
            continue;
        }

        if (projectile->from_player) {
            int j;

            for (j = 0; j < SF_MAX_NPCS; ++j) {
                SfShip *ship = &game->npcs[j];

                if (ship->alive &&
                    sf_distance_squared(
                        projectile->x,
                        projectile->y,
                        ship->x,
                        ship->y
                    ) < 22.0f * 22.0f) {
                    apply_ship_damage(ship, 24.0f);
                    projectile->active = false;

                    if (!ship->alive) {
                        destroy_npc_ship(game, ship);
                    }

                    break;
                }
            }
        } else if (sf_distance_squared(
                       projectile->x,
                       projectile->y,
                       game->player.x,
                       game->player.y
                   ) < SF_PLAYER_RADIUS * SF_PLAYER_RADIUS) {
            apply_ship_damage(&game->player, 8.0f);
            projectile->active = false;
        }
    }
}

static void update_cargo_drops(SfGame *game, float dt)
{
    int i;

    for (i = 0; i < SF_MAX_DROPS; ++i) {
        SfCargoDrop *drop = &game->drops[i];

        if (!drop->active) {
            continue;
        }

        if (drop->item < 0 || drop->item >= SF_ITEM_COUNT || drop->quantity <= 0) {
            drop->active = false;
            continue;
        }

        drop->x += drop->vx * dt;
        drop->y += drop->vy * dt;
        drop->vx *= powf(0.94f, dt * 60.0f);
        drop->vy *= powf(0.94f, dt * 60.0f);

        if (game->npc_salvage_enabled) {
            int j;

            for (j = 0; j < SF_MAX_NPCS; ++j) {
                SfShip *npc = &game->npcs[j];
                const int volume_per_item = drop->item == SF_ITEM_WEAPON ? 2 : 1;
                int added_volume;

                if (drop->quantity > 100 / volume_per_item) {
                    continue;
                }

                added_volume = drop->quantity * volume_per_item;

                if (!npc->alive) {
                    continue;
                }

                if (cargo_used(npc->cargo) + added_volume <= 100 &&
                    sf_distance_squared(
                        drop->x,
                        drop->y,
                        npc->x,
                        npc->y
                    ) < 26.0f * 26.0f) {
                    npc->cargo[drop->item] += drop->quantity;
                    drop->active = false;
                    break;
                }
            }
        }
    }
}

void sf_game_update(SfGame *game, const SfInput *input, float dt)
{
    static const SfInput idle_input = { 0 };

    if (game == NULL || !isfinite(dt) || dt <= 0.0f) {
        return;
    }

    if (dt > 0.05f) {
        dt = 0.05f;
    }

    if (input == NULL) {
        input = &idle_input;
    }

    if (game->player.alive) {
        update_player(game, input, dt);
    }

    update_npcs(game, dt);
    update_projectiles(game, dt);
    update_cargo_drops(game, dt);
}

bool sf_game_collect_nearest(SfGame *game)
{
    int best_index = -1;
    float best_distance_squared = 38.0f * 38.0f;
    int i;

    if (game == NULL || !game->player.alive) {
        return false;
    }

    for (i = 0; i < SF_MAX_DROPS; ++i) {
        SfCargoDrop *drop = &game->drops[i];
        int added_volume;
        float distance_squared;

        if (!drop->active || drop->item < 0 || drop->item >= SF_ITEM_COUNT ||
            drop->quantity <= 0) {
            continue;
        }

        if (drop->quantity > (drop->item == SF_ITEM_WEAPON ? 50 : 100)) {
            continue;
        }

        added_volume = drop->quantity * (drop->item == SF_ITEM_WEAPON ? 2 : 1);
        distance_squared = sf_distance_squared(
            game->player.x,
            game->player.y,
            drop->x,
            drop->y
        );

        if (cargo_used(game->inventory) + added_volume > 100) {
            continue;
        }

        if (distance_squared <= best_distance_squared) {
            best_index = i;
            best_distance_squared = distance_squared;
        }
    }

    if (best_index < 0) {
        return false;
    }

    {
        SfCargoDrop *drop = &game->drops[best_index];

        game->inventory[drop->item] += drop->quantity;
        drop->active = false;
    }

    return true;
}

bool sf_game_jettison_selected(SfGame *game)
{
    int item;
    int quantity;

    if (game == NULL || !game->player.alive) {
        return false;
    }

    item = game->selected_item;
    quantity = game->selected_quantity;

    if (item < 0 || item >= SF_ITEM_COUNT || item == SF_ITEM_WEAPON) {
        return false;
    }

    if (quantity <= 0 || quantity > game->inventory[item]) {
        return false;
    }

    game->inventory[item] -= quantity;

    spawn_cargo_drop(
        game,
        game->player.x,
        game->player.y,
        item,
        quantity
    );

    return true;
}

bool sf_game_sell_selected(SfGame *game)
{
    int item;
    int quantity;

    if (game == NULL || !game->player.alive) {
        return false;
    }

    item = game->selected_item;

    if (item < 0 || item >= SF_ITEM_COUNT) {
        return false;
    }

    if (sf_distance_squared(
            game->player.x,
            game->player.y,
            game->station_x,
            game->station_y
        ) > 90.0f * 90.0f) {
        return false;
    }

    quantity = game->inventory[item];

    if (quantity <= 0) {
        return false;
    }

    {
        const long long sale_value = (long long)quantity * (long long)sf_item_price(item);
        const long long new_credits = (long long)game->credits + sale_value;

        if (new_credits > INT_MAX || new_credits < 0) {
            return false;
        }

        game->credits = (int)new_credits;
    }

    game->inventory[item] = 0;

    return true;
}
