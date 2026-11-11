#include <SDL3/SDL.h>

#include "game.h"
#include "render.h"
#include "assets.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#define SF_WINDOW_WIDTH 960
#define SF_WINDOW_HEIGHT 540
#define SF_LOGICAL_WIDTH 320
#define SF_LOGICAL_HEIGHT 180
#define SF_FIXED_STEP (1.0 / 60.0)

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
    SDL_Texture *texture=sf_assets_get(assets,sprite);
    SDL_FRect dst={x-(float)camera_x-12.0f,y-(float)camera_y-12.0f,24.0f,24.0f};
    if(texture){(void)SDL_RenderTextureRotated(renderer,texture,NULL,&dst,(double)angle*57.29577951308232+90.0,NULL,SDL_FLIP_NONE);}
    else {static const float px[3]={15,-10,-7},py[3]={0,-9,0};float sx[3],sy[3],c=cosf(angle),sn=sinf(angle);for(int i=0;i<3;i++){sx[i]=x+px[i]*c-py[i]*sn-(float)camera_x;sy[i]=y+px[i]*sn+py[i]*c-(float)camera_y;}set_draw_color(renderer,red,green,blue);for(int i=0;i<3;i++){int j=(i+1)%3;SDL_RenderLine(renderer,sx[i],sy[i],sx[j],sy[j]);}}
}

static void draw_asteroids(
    SDL_Renderer *renderer,
    const SfGame *game,
    int camera_x,
    int camera_y
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
        int y;

        if (screen_x + radius < 0 ||
            screen_x - radius >= SF_LOGICAL_WIDTH ||
            screen_y + radius < 0 ||
            screen_y - radius >= SF_LOGICAL_HEIGHT) {
            continue;
        }

        set_draw_color(renderer, 75, 85, 105);

        for (y = -radius; y <= radius; ++y) {
            const int half_width = (int)sqrt(
                (double)(radius * radius - y * y)
            );

            SDL_RenderLine(
                renderer,
                (float)(screen_x - half_width),
                (float)(screen_y + y),
                (float)(screen_x + half_width),
                (float)(screen_y + y)
            );
        }

        set_draw_color(renderer, 105, 115, 135);
        SDL_RenderPoint(
            renderer,
            (float)(screen_x - 4),
            (float)(screen_y - 3)
        );
    }
}

static void draw_cargo_drops(
    SDL_Renderer *renderer,
    const SfGame *game,
    int camera_x,
    int camera_y
)
{
    int i;

    for (i = 0; i < SF_MAX_DROPS; ++i) {
        const SfCargoDrop *drop = &game->drops[i];
        SDL_FRect rectangle;

        if (!drop->active) {
            continue;
        }

        switch (drop->item) {
            case SF_ITEM_METAL:
                set_draw_color(renderer, 160, 170, 180);
                break;

            case SF_ITEM_FUEL:
                set_draw_color(renderer, 80, 190, 230);
                break;

            case SF_ITEM_FOOD:
                set_draw_color(renderer, 110, 190, 90);
                break;

            case SF_ITEM_MEDICINE:
                set_draw_color(renderer, 220, 220, 240);
                break;

            default:
                set_draw_color(renderer, 245, 190, 65);
                break;
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
            ship->role == SF_ROLE_PIRATE ? SF_ASSET_PIRATE : SF_ASSET_CARGO,
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
    char text[160];

    set_draw_color(renderer, 220, 230, 245);

    (void)snprintf(
        text,
        sizeof(text),
        "STAR-FORGE  SYS %s  ARM %.0f HULL %.0f EN %.0f  CR %d",
        sf_game_system_name(game->current_system),
        game->player.armor,
        game->player.hull,
        game->player.energy,
        game->credits
    );

    SDL_RenderDebugText(renderer, 5.0f, 5.0f, text);

    (void)snprintf(
        text,
        sizeof(text),
        "METAL %d FUEL %d FOOD %d MED %d WEAPON %d",
        game->inventory[SF_ITEM_METAL],
        game->inventory[SF_ITEM_FUEL],
        game->inventory[SF_ITEM_FOOD],
        game->inventory[SF_ITEM_MEDICINE],
        game->inventory[SF_ITEM_WEAPON]
    );

    SDL_RenderDebugText(renderer, 5.0f, 17.0f, text);

    (void)snprintf(
        text,
        sizeof(text),
        "SELECT %s x%d | NPC SALVAGE %s",
        sf_item_name(game->selected_item),
        game->selected_quantity,
        game->npc_salvage_enabled ? "ON" : "OFF"
    );

    SDL_RenderDebugText(renderer, 5.0f, 29.0f, text);

    SDL_RenderDebugText(
        renderer,
        5.0f,
        153.0f,
        "W/S thrust A/D turn SPACE fire E loot J jettison"
    );

    SDL_RenderDebugText(
        renderer,
        5.0f,
        165.0f,
        "1-5 item +/- qty T sell F3 salvage I cargo G FTL jump"
    );
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

        set_draw_color(
            renderer,
            (Uint8)(75 + i * 24),
            (Uint8)(90 + i * 17),
            (Uint8)(135 + i * 18)
        );
        draw_filled_circle(renderer, x, y, planet_radius[i]);

        set_draw_color(renderer, 150, 165, 190);
        SDL_RenderLine(
            renderer,
            x - (float)planet_radius[i] * 0.7f,
            y - (float)planet_radius[i] * 0.25f,
            x + (float)planet_radius[i] * 0.55f,
            y - (float)planet_radius[i] * 0.25f
        );
    }

    for (i = 0; i < moon_count && i < 7; ++i) {
        const int parent = i % (planet_count > 0 ? planet_count : 1);
        const float angle = (float)i * 1.37f;
        const float orbit = (float)planet_radius[parent] + 18.0f + (float)(i % 3) * 7.0f;
        const float center_x = planet_x[parent] - (float)camera_x;
        const float center_y = planet_y[parent] - (float)camera_y;
        const float moon_x = center_x + cosf(angle) * orbit;
        const float moon_y = center_y + sinf(angle) * orbit;

        set_draw_color(renderer, 155, 155, 165);
        draw_filled_circle(renderer, moon_x, moon_y, 5 + (i % 3));
    }
}

void sf_render_world(
    SDL_Renderer *renderer,
    const SfGame *game,
    const SfAssets *assets,
    int camera_x,
    int camera_y
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

        if (screen_x >= 0 && screen_x < SF_LOGICAL_WIDTH &&
            screen_y >= 0 && screen_y < SF_LOGICAL_HEIGHT) {
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

    draw_system_bodies(renderer, game, camera_x, camera_y);
    draw_asteroids(renderer, game, camera_x, camera_y);

    {SDL_Texture *t=sf_assets_get(assets,SF_ASSET_STATION);station_rectangle=(SDL_FRect){game->station_x-(float)camera_x-16.0f,game->station_y-(float)camera_y-16.0f,32.0f,32.0f};if(t)(void)SDL_RenderTexture(renderer,t,NULL,&station_rectangle);else{set_draw_color(renderer,55,110,150);SDL_RenderRect(renderer,&station_rectangle);}}

    draw_cargo_drops(renderer, game, camera_x, camera_y);
    draw_projectiles(renderer, game, camera_x, camera_y);
    draw_npcs(renderer, game, assets, camera_x, camera_y);

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

    draw_hud(renderer, game);
}

void sf_render_hover_info(
    SDL_Renderer *renderer,
    const SfGame *game,
    float mouse_x,
    float mouse_y,
    int camera_x,
    int camera_y
)
{
    int i;

    for (i = 0; i < SF_MAX_DROPS; ++i) {
        const SfCargoDrop *drop = &game->drops[i];
        char text[96];

        if (!drop->active) {
            continue;
        }

        if (sf_distance_squared(
                mouse_x + (float)camera_x,
                mouse_y + (float)camera_y,
                drop->x,
                drop->y
            ) < 18.0f * 18.0f) {
            (void)snprintf(
                text,
                sizeof(text),
                "%s x%d | E: collect",
                sf_item_name(drop->item),
                drop->quantity
            );

            set_draw_color(renderer, 255, 235, 150);
            SDL_RenderDebugText(renderer, 5.0f, 43.0f, text);
            return;
        }
    }
}

void sf_render_inventory_panel(
    SDL_Renderer *renderer,
    const SfGame *game
)
{
    SDL_FRect panel = { 62.0f, 52.0f, 196.0f, 76.0f };
    char text[96];

    set_draw_color(renderer, 20, 28, 48);
    SDL_RenderFillRect(renderer, &panel);

    set_draw_color(renderer, 100, 180, 220);
    SDL_RenderRect(renderer, &panel);

    set_draw_color(renderer, 240, 240, 245);
    SDL_RenderDebugText(renderer, 72.0f, 61.0f, "CARGO HOLD (I to close)");

    (void)snprintf(
        text,
        sizeof(text),
        "Selected: %s   Quantity: %d",
        sf_item_name(game->selected_item),
        game->selected_quantity
    );

    SDL_RenderDebugText(renderer, 72.0f, 76.0f, text);
    SDL_RenderDebugText(
        renderer,
        72.0f,
        91.0f,
        "J jettison  +/- quantity  T sell at station"
    );
    SDL_RenderDebugText(renderer, 72.0f, 106.0f, "E collect nearby salvage");
}


