#include "../src/game.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>

int main(void)
{
    SfGame game;
    SfInput idle = { false, false, false, false, false, false };
    bool fuel_drop_found = false;

    sf_game_init(&game);
    assert(game.player.alive);
    assert(game.player.armor == 60.0f);
    assert(game.player.hull == 100.0f);
    assert(game.player.energy == 100.0f);
    assert(game.current_system == 0);
    assert(game.inventory[SF_ITEM_METAL] == 12);
    assert(game.npcs[0].role == SF_ROLE_CARGO);
    assert(game.npcs[1].role == SF_ROLE_PIRATE);
    assert(game.npcs[0].cargo[SF_ITEM_FUEL] +
           game.npcs[0].cargo[SF_ITEM_FOOD] +
           game.npcs[0].cargo[SF_ITEM_METAL] == 100);

    /* Boost increases acceleration without consuming ship energy. */
    {
        SfInput boost_input = { true, false, false, false, false, true };

        sf_game_update(&game, &boost_input, 1.0f / 60.0f);
        assert(game.player.energy == 100.0f);
        assert(game.player.vx > 4.0f);
    }

    game.selected_item = SF_ITEM_METAL;
    game.selected_quantity = 5;
    assert(sf_game_jettison_selected(&game));
    assert(game.inventory[SF_ITEM_METAL] == 7);
    assert(sf_game_collect_nearest(&game));
    assert(game.inventory[SF_ITEM_METAL] == 12);

    game.player.x = game.station_x;
    game.player.y = game.station_y;
    assert(sf_game_sell_selected(&game));
    assert(game.inventory[SF_ITEM_METAL] == 0);
    assert(game.credits == 216);

    /* A ship overlapping an asteroid is pushed clear of its collision radius. */
    sf_game_init(&game);
    game.player.x = 520.0f;
    game.player.y = 250.0f;
    sf_game_update(&game, &idle, 1.0f / 60.0f);
    assert(sf_distance_squared(game.player.x, game.player.y, 520.0f, 250.0f) >= 34.0f * 34.0f);

    /* Destroying a cargo NPC creates physical drops from its actual manifest. */
    sf_game_init(&game);
    game.player.x = 600.0f;
    game.player.y = 360.0f;
    game.player.angle = 0.0f;
    game.npcs[0].x = 630.0f;
    game.npcs[0].y = 360.0f;
    game.npcs[0].armor = 0.0f;
    game.npcs[0].hull = 24.0f;
    sf_game_fire_player(&game);
    sf_game_update(&game, &idle, 0.05f);
    assert(!game.npcs[0].alive);
    assert(game.destroyed_npcs == 1u);
    for (int i = 0; i < SF_MAX_DROPS; ++i) {
        if (game.drops[i].active && game.drops[i].item == SF_ITEM_FUEL &&
            game.drops[i].quantity == 60) {
            fuel_drop_found = true;
        }
    }
    assert(fuel_drop_found);

    /* Armour absorbs damage before hull; overflow reaches the hull. */
    sf_game_init(&game);
    game.player.armor = 5.0f;
    game.player.hull = 20.0f;
    game.projectiles[0].active = true;
    game.projectiles[0].from_player = false;
    game.projectiles[0].x = game.player.x;
    game.projectiles[0].y = game.player.y;
    game.projectiles[0].life = 1.0f;
    sf_game_update(&game, &idle, 1.0f / 60.0f);
    assert(game.player.armor == 0.0f);
    assert(game.player.hull == 17.0f);

    /* Weapons consume energy; thrust and turning do not. */
    sf_game_init(&game);
    game.player.energy = 8.0f;
    sf_game_fire_player(&game);
    assert(game.player.energy == 0.0f);
    assert(game.projectiles[0].active);
    game.player.energy = 7.0f;
    game.player.fire_cooldown = 0.0f;
    sf_game_fire_player(&game);
    assert(!game.projectiles[1].active);
    {
        SfInput move_input = { true, false, true, false, false, false };
        const float energy_before = game.player.energy;
        sf_game_update(&game, &move_input, 1.0f / 60.0f);
        assert(game.player.energy >= energy_before);
    }

    /* Inter-system travel requires an FTL drive and five fuel units. */
    sf_game_init(&game);
    assert(sf_game_jump_to_system(&game, 1));
    assert(game.current_system == 1);
    assert(game.npcs[0].alive);
    assert(game.npcs[1].alive);
    assert(game.inventory[SF_ITEM_FUEL] == 13);
    assert(sf_game_system_planet_count(1) == 2);
    assert(sf_game_system_moon_count(1) == 3);
    assert(!sf_game_system_has_asteroids(1));
    assert(!sf_game_jump_to_system(&game, 1));
    game.player.has_ftl_drive = false;
    assert(!sf_game_jump_to_system(&game, 2));
    game.player.has_ftl_drive = true;
    game.inventory[SF_ITEM_FUEL] = 4;
    assert(!sf_game_jump_to_system(&game, 2));

    /* Public simulation entry points safely reject null and invalid time inputs. */
    {
        const float old_x = game.player.x;
        const float old_y = game.player.y;

        sf_game_update(NULL, &idle, 1.0f / 60.0f);
        sf_game_update(&game, &idle, NAN);
        sf_game_update(&game, &idle, INFINITY);
        sf_game_update(&game, &idle, -1.0f);
        sf_game_init(NULL);
        sf_game_fire_player(NULL);

        assert(game.player.x == old_x);
        assert(game.player.y == old_y);
        assert(!sf_game_collect_nearest(NULL));
        assert(!sf_game_jettison_selected(NULL));
        assert(!sf_game_sell_selected(NULL));
        assert(!sf_game_jump_to_system(NULL, 1));
    }

    puts("Star-Forge simulation tests passed.");
    return 0;
}
