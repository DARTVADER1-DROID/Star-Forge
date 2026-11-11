#include <SDL3/SDL.h>

#include "game.h"
#include "render.h"
#include "assets.h"
#include "ui_layout.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>


static void set_draw_color(
    SDL_Renderer *renderer,
    Uint8 red,
    Uint8 green,
    Uint8 blue
)
{
    SDL_SetRenderDrawColor(renderer, red, green, blue, 255);
}

static void draw_ship(
    SDL_Renderer *renderer,
    const SfAssets *assets,
    SfAssetId sprite,
    float x,
    float y,
    float angle,
    int camera_x,
    int camera_y,
    Uint8 red,
    Uint8 green,
    Uint8 blue
)
{
    SDL_Texture *texture = sf_assets_get(assets, sprite);
    SDL_FRect destination = {
        x - (float)camera_x - 12.0f,
        y - (float)camera_y - 12.0f,
        24.0f,
        24.0f
    };

    if (texture != NULL) {
        const double degrees = (double)angle * 57.29577951308232 + 90.0;

        (void)SDL_RenderTextureRotated(
            renderer,
            texture,
            NULL,
            &destination,
            degrees,
            NULL,
            SDL_FLIP_NONE
        );
    } else {
        static const float local_x[3] = { 15.0f, -10.0f, -7.0f };
        static const float local_y[3] = { 0.0f, -9.0f, 0.0f };
        float screen_x[3];
        float screen_y[3];
        const float cosine = cosf(angle);
        const float sine = sinf(angle);
        int i;

        for (i = 0; i < 3; ++i) {
            screen_x[i] = x + local_x[i] * cosine - local_y[i] * sine -
                (float)camera_x;
            screen_y[i] = y + local_x[i] * sine + local_y[i] * cosine -
                (float)camera_y;
        }

        set_draw_color(renderer, red, green, blue);

        for (i = 0; i < 3; ++i) {
            const int next = (i + 1) % 3;

            SDL_RenderLine(
                renderer,
                screen_x[i],
                screen_y[i],
                screen_x[next],
                screen_y[next]
            );
        }
    }
}

static void draw_asteroids(
    SDL_Renderer *renderer,
    const SfGame *game,
    const SfAssets *assets,
    int camera_x,
    int camera_y,
    int viewport_width
)
{
    if (!sf_game_system_has_asteroids(game->current_system)) {
        return;
    }
    static const int asteroid_x[8] = {
        520, 680, 860, 1040, 1280, 1510, 1740, 1980
    };

    static const int asteroid_y[8] = {
        250, 620, 430, 890, 510, 1120, 760, 1380
    };

    static const int asteroid_radius[8] = {
        22, 30, 18, 34, 25, 28, 20, 32
    };

    int i;

    for (i = 0; i < 8; ++i) {
        const int screen_x = asteroid_x[i] - camera_x;
        const int screen_y = asteroid_y[i] - camera_y;
        const int radius = asteroid_radius[i];

        if (screen_x + radius < 0 ||
            screen_x - radius >= viewport_width ||
            screen_y + radius < 0 ||
            screen_y - radius >= SF_UI_LOGICAL_HEIGHT) {
            continue;
        }

        {
            SDL_Texture *texture = sf_assets_get(assets, SF_ASSET_ASTEROID);
            SDL_FRect destination = {
                (float)screen_x - (float)radius,
                (float)screen_y - (float)radius,
                (float)radius * 2.0f,
                (float)radius * 2.0f
            };
            if (texture != NULL) {
                (void)SDL_RenderTexture(renderer, texture, NULL, &destination);
            }
        }
    }
}

static void draw_cargo_drops(
    SDL_Renderer *renderer,
    const SfGame *game,
    const SfAssets *assets,
    int camera_x,
    int camera_y
)
{
    int i;

    for (i = 0; i < SF_MAX_DROPS; ++i) {
        const SfCargoDrop *drop = &game->drops[i];
        SDL_FRect rectangle;

        if (!drop->active || drop->quantity <= 0) {
            continue;
        }

        if (drop->item >= 0 && drop->item < SF_ITEM_COUNT) {
            static const SfAssetId salvage_ids[SF_ITEM_COUNT] = {
                SF_ASSET_METAL, SF_ASSET_FUEL, SF_ASSET_FOOD,
                SF_ASSET_MEDICINE, SF_ASSET_WEAPON
            };
            SDL_Texture *texture = sf_assets_get(assets, salvage_ids[drop->item]);
            SDL_FRect sprite = {
                drop->x - (float)camera_x - 5.0f,
                drop->y - (float)camera_y - 5.0f,
                10.0f,
                10.0f
            };
            if (texture != NULL) {
                (void)SDL_RenderTexture(renderer, texture, NULL, &sprite);
                continue;
            }
        }

        rectangle = (SDL_FRect) {
            drop->x - (float)camera_x - 3.0f,
            drop->y - (float)camera_y - 3.0f,
            6.0f,
            6.0f
        };

        SDL_RenderFillRect(renderer, &rectangle);
    }
}

static void draw_projectiles(
    SDL_Renderer *renderer,
    const SfGame *game,
    int camera_x,
    int camera_y
)
{
    int i;

    for (i = 0; i < SF_MAX_PROJECTILES; ++i) {
        const SfProjectile *projectile = &game->projectiles[i];

        if (!projectile->active) {
            continue;
        }

        if (projectile->from_player) {
            set_draw_color(renderer, 100, 220, 80);
        } else {
            set_draw_color(renderer, 255, 90, 80);
        }

        SDL_RenderPoint(
            renderer,
            projectile->x - (float)camera_x,
            projectile->y - (float)camera_y
        );
    }
}

static void draw_npcs(
    SDL_Renderer *renderer,
    const SfGame *game,
    const SfAssets *assets,
    int camera_x,
    int camera_y
)
{
    int i;

    for (i = 0; i < SF_MAX_NPCS; ++i) {
        const SfShip *ship = &game->npcs[i];
        const Uint8 red = ship->role == SF_ROLE_PIRATE ? 235 : 90;
        const Uint8 green = ship->role == SF_ROLE_PIRATE ? 80 : 190;
        const Uint8 blue = ship->role == SF_ROLE_PIRATE ? 85 : 220;
        const float health_ratio = ship->max_hull > 0.0f
            ? ship->hull / ship->max_hull
            : 0.0f;

        if (!ship->alive) {
            continue;
        }

        draw_ship(
            renderer,
            assets,
            ship->role == SF_ROLE_PIRATE
                ? SF_ASSET_PIRATE
                : (ship->role == SF_ROLE_PATROL ? SF_ASSET_PATROL : SF_ASSET_CARGO),
            ship->x,
            ship->y,
            ship->angle,
            camera_x,
            camera_y,
            red,
            green,
            blue
        );

        set_draw_color(renderer, 180, 60, 70);

        SDL_RenderLine(
            renderer,
            ship->x - (float)camera_x - 10.0f,
            ship->y - (float)camera_y - 13.0f,
            ship->x - (float)camera_x - 10.0f + 20.0f * health_ratio,
            ship->y - (float)camera_y - 13.0f
        );
    }
}

static void draw_hud(SDL_Renderer *renderer, const SfGame *game)
{
    char text[96];

    (void)snprintf(
        text,
        sizeof(text),
        "%s  HULL %.0f  ARM %.0f  ENERGY %.0f",
        sf_game_system_name(game->current_system),
        game->player.hull,
        game->player.armor,
        game->player.energy
    );

    set_draw_color(renderer, 205, 220, 238);
    SDL_RenderDebugText(renderer, 5.0f, 5.0f, text);
}


static void draw_filled_circle(
    SDL_Renderer *renderer,
    float center_x,
    float center_y,
    int radius
)
{
    int y;

    for (y = -radius; y <= radius; ++y) {
        const float span = sqrtf((float)(radius * radius - y * y));

        SDL_RenderLine(
            renderer,
            center_x - span,
            center_y + (float)y,
            center_x + span,
            center_y + (float)y
        );
    }
}

static void draw_system_bodies(
    SDL_Renderer *renderer,
    const SfGame *game,
    const SfAssets *assets,
    int camera_x,
    int camera_y
)
{
    static const float planet_x[4] = { 720.0f, 1420.0f, 1850.0f, 980.0f };
    static const float planet_y[4] = { 620.0f, 1180.0f, 540.0f, 1760.0f };
    static const int planet_radius[4] = { 34, 48, 27, 40 };
    const int planet_count = sf_game_system_planet_count(game->current_system);
    const int moon_count = sf_game_system_moon_count(game->current_system);
    int i;

    for (i = 0; i < planet_count && i < 4; ++i) {
        const float x = planet_x[i] - (float)camera_x;
        const float y = planet_y[i] - (float)camera_y;

        {
            SDL_Texture *texture = sf_assets_get(
                assets,
                (i % 2) == 0 ? SF_ASSET_TERRAN : SF_ASSET_GAS
            );
            SDL_FRect destination = {
                x - (float)planet_radius[i],
                y - (float)planet_radius[i],
                (float)planet_radius[i] * 2.0f,
                (float)planet_radius[i] * 2.0f
            };
            if (texture != NULL) {
                (void)SDL_RenderTexture(renderer, texture, NULL, &destination);
            } else {
                set_draw_color(renderer, 85, 100, 145);
                draw_filled_circle(renderer, x, y, planet_radius[i]);
            }
        }
    }

    for (i = 0; i < moon_count && i < 7; ++i) {
        const int parent = i % (planet_count > 0 ? planet_count : 1);
        const float angle = (float)i * 1.37f;
        const float orbit = (float)planet_radius[parent] + 18.0f + (float)(i % 3) * 7.0f;
        const float center_x = planet_x[parent] - (float)camera_x;
        const float center_y = planet_y[parent] - (float)camera_y;
        const float moon_x = center_x + cosf(angle) * orbit;
        const float moon_y = center_y + sinf(angle) * orbit;

        {
            SDL_Texture *texture = sf_assets_get(assets, SF_ASSET_MOON);
            SDL_FRect destination = {
                moon_x - 6.0f,
                moon_y - 6.0f,
                12.0f,
                12.0f
            };

            if (texture != NULL) {
                (void)SDL_RenderTexture(renderer, texture, NULL, &destination);
            } else {
                set_draw_color(renderer, 155, 155, 165);
                draw_filled_circle(renderer, moon_x, moon_y, 6);
            }
        }
    }
}

void sf_render_world(
    SDL_Renderer *renderer,
    const SfGame *game,
    const SfAssets *assets,
    int camera_x,
    int camera_y,
    bool draw_player,
    int viewport_width
)
{
    SDL_FRect station_rectangle;
    int i;

    set_draw_color(renderer, 5, 8, 19);
    SDL_RenderClear(renderer);

    /* A deterministic star field keeps the prototype asset-free. */
    for (i = 0; i < 90; ++i) {
        const int world_x = (i * 173 + 71) % 2400;
        const int world_y = (i * 97 + 211) % 2400;
        const int screen_x = world_x - camera_x;
        const int screen_y = world_y - camera_y;

        if (screen_x >= 0 && screen_x < viewport_width &&
            screen_y >= 0 && screen_y < SF_UI_LOGICAL_HEIGHT) {
            set_draw_color(
                renderer,
                (Uint8)(70 + (i % 4) * 30),
                (Uint8)(80 + (i % 3) * 25),
                150
            );

            SDL_RenderPoint(
                renderer,
                (float)screen_x,
                (float)screen_y
            );
        }
    }

    draw_system_bodies(renderer, game, assets, camera_x, camera_y);
    draw_asteroids(renderer, game, assets, camera_x, camera_y, viewport_width);

    {
        SDL_Texture *texture = sf_assets_get(assets, SF_ASSET_STATION);

        station_rectangle = (SDL_FRect) {
            game->station_x - (float)camera_x - 16.0f,
            game->station_y - (float)camera_y - 16.0f,
            32.0f,
            32.0f
        };

        if (texture != NULL) {
            (void)SDL_RenderTexture(
                renderer,
                texture,
                NULL,
                &station_rectangle
            );
        } else {
            set_draw_color(renderer, 55, 110, 150);
            SDL_RenderRect(renderer, &station_rectangle);
        }
    }

    draw_cargo_drops(renderer, game, assets, camera_x, camera_y);
    draw_projectiles(renderer, game, camera_x, camera_y);
    draw_npcs(renderer, game, assets, camera_x, camera_y);

    if (draw_player && game->player.alive) {
        draw_ship(
            renderer,
            assets,
            SF_ASSET_PLAYER,
            game->player.x,
            game->player.y,
            game->player.angle,
            camera_x,
            camera_y,
            100,
            235,
            150
        );
    }

    draw_hud(renderer, game);
}

void sf_render_inventory_panel(
    SDL_Renderer *renderer,
    const SfGame *game,
    int viewport_width
)
{
    static const char *const labels[SF_ITEM_COUNT] = {
        "METAL", "FUEL", "FOOD", "MED", "WEAPON"
    };
    const float panel_offset = ((float)viewport_width - 320.0f) * 0.5f;
    SDL_FRect panel = { 40.0f + panel_offset, 45.0f, 240.0f, 88.0f };
    char text[96];
    const int selected_item = game->selected_item >= 0 &&
        game->selected_item < SF_ITEM_COUNT ? game->selected_item : SF_ITEM_METAL;
    int i;

    set_draw_color(renderer, 12, 20, 38);
    SDL_RenderFillRect(renderer, &panel);
    set_draw_color(renderer, 100, 180, 220);
    SDL_RenderRect(renderer, &panel);

    set_draw_color(renderer, 240, 240, 245);
    SDL_RenderDebugText(renderer, 50.0f + panel_offset, 52.0f, "CARGO HOLD");
    SDL_RenderDebugText(renderer, 250.0f + panel_offset, 52.0f, "X");

    for (i = 0; i < SF_ITEM_COUNT; ++i) {
        SDL_FRect item_button = {
            48.0f + panel_offset + (float)i * 45.0f,
            64.0f,
            42.0f,
            17.0f
        };

        set_draw_color(
            renderer,
            selected_item == i ? 55 : 30,
            selected_item == i ? 105 : 55,
            selected_item == i ? 155 : 85
        );
        SDL_RenderFillRect(renderer, &item_button);
        set_draw_color(renderer, 230, 235, 245);
        SDL_RenderDebugText(
            renderer,
            item_button.x + 2.0f,
            item_button.y + 4.0f,
            labels[i]
        );
    }

    (void)snprintf(
        text,
        sizeof(text),
        "QTY %d   HOLD %d",
        game->selected_quantity,
        game->inventory[selected_item]
    );
    set_draw_color(renderer, 240, 240, 245);
    SDL_RenderDebugText(renderer, 50.0f + panel_offset, 87.0f, text);
    SDL_RenderDebugText(renderer, 52.0f + panel_offset, 101.0f, "- QTY");
    SDL_RenderDebugText(renderer, 115.0f + panel_offset, 101.0f, "+ QTY");
    SDL_RenderDebugText(
        renderer,
        190.0f + panel_offset,
        101.0f,
        game->npc_salvage_enabled ? "NPC LOOT ON" : "NPC LOOT OFF"
    );
    SDL_RenderDebugText(renderer, 52.0f + panel_offset, 117.0f, "JETTISON");
    SDL_RenderDebugText(renderer, 178.0f + panel_offset, 117.0f, "SELL AT STATION");
}
