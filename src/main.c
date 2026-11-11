#include <SDL3/SDL.h>

#include "game.h"

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
    static const float local_x[3] = { 15.0f, -10.0f, -7.0f };
    static const float local_y[3] = { 0.0f, -9.0f, 0.0f };
    float screen_x[3];
    float screen_y[3];
    const float cosine = cosf(angle);
    const float sine = sinf(angle);
    int i;

    for (i = 0; i < 3; ++i) {
        const float rotated_x = local_x[i] * cosine - local_y[i] * sine;
        const float rotated_y = local_x[i] * sine + local_y[i] * cosine;

        screen_x[i] = x + rotated_x - (float)camera_x;
        screen_y[i] = y + rotated_y - (float)camera_y;
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

static void draw_asteroids(
    SDL_Renderer *renderer,
    int camera_x,
    int camera_y
)
{
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
        const float health_ratio = ship->max_hp > 0.0f
            ? ship->hp / ship->max_hp
            : 0.0f;

        if (!ship->alive) {
            continue;
        }

        draw_ship(
            renderer,
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
        "STAR-FORGE v0.1  HP %.0f  CREDITS %d",
        game->player.hp,
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
        "1-5 item +/- qty T sell near station F3 NPC salvage I cargo"
    );
}

static void draw_world(
    SDL_Renderer *renderer,
    const SfGame *game,
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

    draw_asteroids(renderer, camera_x, camera_y);

    set_draw_color(renderer, 55, 110, 150);

    station_rectangle = (SDL_FRect) {
        game->station_x - (float)camera_x - 9.0f,
        game->station_y - (float)camera_y - 9.0f,
        18.0f,
        18.0f
    };

    SDL_RenderRect(renderer, &station_rectangle);

    draw_cargo_drops(renderer, game, camera_x, camera_y);
    draw_projectiles(renderer, game, camera_x, camera_y);
    draw_npcs(renderer, game, camera_x, camera_y);

    draw_ship(
        renderer,
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

static void draw_hover_info(
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

static void draw_inventory_panel(
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

static void handle_key_press(
    SDL_Scancode key,
    SfGame *game,
    bool *running,
    bool *inventory_open
)
{
    switch (key) {
        case SDL_SCANCODE_ESCAPE:
            *running = false;
            break;

        case SDL_SCANCODE_1:
            game->selected_item = SF_ITEM_METAL;
            break;

        case SDL_SCANCODE_2:
            game->selected_item = SF_ITEM_FUEL;
            break;

        case SDL_SCANCODE_3:
            game->selected_item = SF_ITEM_FOOD;
            break;

        case SDL_SCANCODE_4:
            game->selected_item = SF_ITEM_MEDICINE;
            break;

        case SDL_SCANCODE_5:
            game->selected_item = SF_ITEM_WEAPON;
            break;

        case SDL_SCANCODE_J:
            (void)sf_game_jettison_selected(game);
            break;

        case SDL_SCANCODE_E:
            (void)sf_game_collect_nearest(game);
            break;

        case SDL_SCANCODE_T:
            (void)sf_game_sell_selected(game);
            break;

        case SDL_SCANCODE_F3:
            game->npc_salvage_enabled = !game->npc_salvage_enabled;
            break;

        case SDL_SCANCODE_EQUALS:
        case SDL_SCANCODE_KP_PLUS:
            game->selected_quantity += 1;
            break;

        case SDL_SCANCODE_MINUS:
        case SDL_SCANCODE_KP_MINUS:
            if (game->selected_quantity > 1) {
                game->selected_quantity -= 1;
            }
            break;

        case SDL_SCANCODE_I:
            *inventory_open = !*inventory_open;
            break;

        default:
            break;
    }
}

int main(int argc, char **argv)
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SfGame game;
    bool running = true;
    bool inventory_open = false;
    Uint64 previous_counter;
    Uint64 performance_frequency;
    double accumulator = 0.0;

    (void)argc;
    (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        SDL_Log("SDL initialization failed: %s", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow(
        "Star-Forge v0.1",
        SF_WINDOW_WIDTH,
        SF_WINDOW_HEIGHT,
        SDL_WINDOW_RESIZABLE
    );

    if (window == NULL) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, NULL);

    if (renderer == NULL) {
        SDL_Log("Renderer creation failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    (void)SDL_SetRenderLogicalPresentation(
        renderer,
        SF_LOGICAL_WIDTH,
        SF_LOGICAL_HEIGHT,
        SDL_LOGICAL_PRESENTATION_LETTERBOX
    );

    sf_game_init(&game);

    performance_frequency = SDL_GetPerformanceFrequency();
    previous_counter = SDL_GetPerformanceCounter();

    while (running) {
        SDL_Event event;
        SfInput input = { false, false, false, false, false };
        const bool *keyboard_state;
        Uint64 current_counter;
        double frame_time;
        float mouse_x = 0.0f;
        float mouse_y = 0.0f;
        int window_width = SF_WINDOW_WIDTH;
        int window_height = SF_WINDOW_HEIGHT;
        int camera_x;
        int camera_y;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }

            if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
                handle_key_press(
                    event.key.scancode,
                    &game,
                    &running,
                    &inventory_open
                );
            }
        }

        keyboard_state = SDL_GetKeyboardState(NULL);

        input.thrust =
            keyboard_state[SDL_SCANCODE_W] ||
            keyboard_state[SDL_SCANCODE_UP];

        input.reverse =
            keyboard_state[SDL_SCANCODE_S] ||
            keyboard_state[SDL_SCANCODE_DOWN];

        input.turn_left =
            keyboard_state[SDL_SCANCODE_A] ||
            keyboard_state[SDL_SCANCODE_LEFT];

        input.turn_right =
            keyboard_state[SDL_SCANCODE_D] ||
            keyboard_state[SDL_SCANCODE_RIGHT];

        input.fire = keyboard_state[SDL_SCANCODE_SPACE];

        current_counter = SDL_GetPerformanceCounter();

        frame_time = (double)(current_counter - previous_counter) /
            (double)performance_frequency;

        previous_counter = current_counter;

        if (frame_time > 0.25) {
            frame_time = 0.25;
        }

        accumulator += frame_time;

        while (accumulator >= SF_FIXED_STEP) {
            sf_game_update(&game, &input, (float)SF_FIXED_STEP);
            accumulator -= SF_FIXED_STEP;
        }

        camera_x = (int)(game.player.x - (float)SF_LOGICAL_WIDTH * 0.5f);
        camera_y = (int)(game.player.y - (float)SF_LOGICAL_HEIGHT * 0.5f);

        (void)SDL_GetMouseState(&mouse_x, &mouse_y);
        SDL_GetWindowSize(window, &window_width, &window_height);

        if (window_width > 0 && window_height > 0) {
            mouse_x = mouse_x * (float)SF_LOGICAL_WIDTH / (float)window_width;
            mouse_y = mouse_y * (float)SF_LOGICAL_HEIGHT / (float)window_height;
        }

        draw_world(renderer, &game, camera_x, camera_y);

        draw_hover_info(
            renderer,
            &game,
            mouse_x,
            mouse_y,
            camera_x,
            camera_y
        );

        if (inventory_open) {
            draw_inventory_panel(renderer, &game);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(1);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
