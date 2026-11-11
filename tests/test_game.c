#include "../src/game.h"
#include <assert.h>
#include <stdio.h>

int main(void)
{
    SfGame game;
    SfInput idle = { false, false, false, false, false, false };
    bool fuel_drop_found = false;

    sf_game_init(&game);
    assert(game.player.alive);
    assert(game.boost_energy == 100.0f);
    assert(game.inventory[SF_ITEM_METAL] == 12);
    assert(game.npcs[0].role == SF_ROLE_CARGO);
    assert(game.npcs[1].role == SF_ROLE_PIRATE);

    /* Boost consumes energy and increases acceleration. */
    {
        SfInput boost_input = { true, false, false, false, false, true };

        sf_game_update(&game, &boost_input, 1.0f / 60.0f);
        assert(game.boost_energy < 100.0f);
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
    game.npcs[0].hp = 24.0f;
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

    puts("Star-Forge simulation tests passed.");
    return 0;
}
