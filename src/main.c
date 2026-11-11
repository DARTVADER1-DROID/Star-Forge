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


typedef struct {
    SDL_FingerID joystick_id;
    SDL_FingerID fire_id;
    SDL_FingerID boost_id;
    bool joystick_active;
    bool fire_active;
    bool boost_active;
    float joystick_x;
    float joystick_y;
} SfTouchState;

static void update_touch_event(
    const SDL_TouchFingerEvent *finger,
    SfTouchState *touch,
    SfGame *game
)
{
    const float x = finger->x * (float)SF_LOGICAL_WIDTH;
    const float y = finger->y * (float)SF_LOGICAL_HEIGHT;

    if (finger->type == SDL_EVENT_FINGER_DOWN) {
        if (x < 155.0f && !touch->joystick_active) {
            touch->joystick_active = true;
            touch->joystick_id = finger->fingerID;
            touch->joystick_x = x;
            touch->joystick_y = y;
        } else if (x > 245.0f && y > 105.0f && !touch->fire_active) {
            touch->fire_active = true;
            touch->fire_id = finger->fingerID;
        } else if (x > 245.0f && y <= 105.0f && !touch->boost_active) {
            touch->boost_active = true;
            touch->boost_id = finger->fingerID;
        } else if (x > 175.0f && x <= 245.0f && y > 125.0f) {
            (void)sf_game_collect_nearest(game);
        }

        return;
    }

    if (finger->type == SDL_EVENT_FINGER_MOTION &&
        touch->joystick_active &&
        finger->fingerID == touch->joystick_id) {
        touch->joystick_x = x;
        touch->joystick_y = y;
        return;
    }

    if (finger->type == SDL_EVENT_FINGER_UP) {
        if (touch->joystick_active && finger->fingerID == touch->joystick_id) {
            touch->joystick_active = false;
        }

        if (touch->fire_active && finger->fingerID == touch->fire_id) {
            touch->fire_active = false;
        }

        if (touch->boost_active && finger->fingerID == touch->boost_id) {
            touch->boost_active = false;
        }
    }
}

static void draw_touch_ring(SDL_Renderer *renderer, float center_x, float center_y, float radius)
{
    const int segments = 24;
    int i;

    set_draw_color(renderer, 105, 165, 225);

    for (i = 0; i < segments; ++i) {
        const float a0 = (float)i * 6.28318530718f / (float)segments;
        const float a1 = (float)(i + 1) * 6.28318530718f / (float)segments;

        SDL_RenderLine(
            renderer,
            center_x + cosf(a0) * radius,
            center_y + sinf(a0) * radius,
            center_x + cosf(a1) * radius,
            center_y + sinf(a1) * radius
        );
    }
}

static void draw_touch_controls(
    SDL_Renderer *renderer,
    const SfTouchState *touch,
    const SfGame *game
)
{
    SDL_FRect fire = { 268.0f, 125.0f, 43.0f, 43.0f };
    SDL_FRect boost = { 270.0f, 69.0f, 38.0f, 38.0f };
    SDL_FRect loot = { 211.0f, 145.0f, 38.0f, 27.0f };
    float knob_x = 48.0f;
    float knob_y = 132.0f;

    if (touch->joystick_active) {
        float dx = touch->joystick_x - 48.0f;
        float dy = touch->joystick_y - 132.0f;
        const float length = sqrtf(dx * dx + dy * dy);

        if (length > 25.0f) {
            dx *= 25.0f / length;
            dy *= 25.0f / length;
        }

        knob_x += dx;
        knob_y += dy;
    }

    draw_touch_ring(renderer, 48.0f, 132.0f, 31.0f);
    draw_touch_ring(renderer, knob_x, knob_y, 12.0f);

    set_draw_color(renderer, 30, 70, 115);
    SDL_RenderFillRect(renderer, &fire);
    SDL_RenderFillRect(renderer, &boost);
    SDL_RenderFillRect(renderer, &loot);

    set_draw_color(renderer, 125, 195, 245);
    SDL_RenderRect(renderer, &fire);
    SDL_RenderRect(renderer, &boost);
    SDL_RenderRect(renderer, &loot);

    set_draw_color(renderer, 245, 245, 255);
    SDL_RenderDebugText(renderer, 276.0f, 141.0f, "FIRE");
    SDL_RenderDebugText(renderer, 275.0f, 82.0f, "BOOST");
    SDL_RenderDebugText(renderer, 218.0f, 154.0f, "LOOT");

    {
        char energy_text[24];

        (void)snprintf(
            energy_text,
            sizeof(energy_text),
            "BOOST %.0f%%",
            game->boost_energy
        );
        SDL_RenderDebugText(renderer, 5.0f, 41.0f, energy_text);
    }
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
    SfTouchState touch = { 0 };
    Uint64 previous_counter;
    Uint64 performance_frequency;
    double accumulator = 0.0;

    (void)argc;
    (void)argv;

    SDL_Log("Star-Forge: starting SDL video subsystem");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("FATAL: SDL video initialization failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Log("Star-Forge: SDL video initialized");

    window = SDL_CreateWindow(
        "Star-Forge v0.1",
        SF_WINDOW_WIDTH,
        SF_WINDOW_HEIGHT,
        SDL_WINDOW_RESIZABLE
    );

    if (window == NULL) {
        SDL_Log("FATAL: window creation failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Log("Star-Forge: window created");

    renderer = SDL_CreateRenderer(window, NULL);

    if (renderer == NULL) {
        SDL_Log("Default renderer unavailable: %s", SDL_GetError());
        SDL_ClearError();
        renderer = SDL_CreateRenderer(window, "software");
    }

    if (renderer == NULL) {
        SDL_Log("FATAL: no SDL renderer could be created: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Log("Star-Forge: renderer created");

    (void)SDL_SetRenderLogicalPresentation(
        renderer,
        SF_LOGICAL_WIDTH,
        SF_LOGICAL_HEIGHT,
        SDL_LOGICAL_PRESENTATION_LETTERBOX
    );

    sf_game_init(&game);
    SDL_Log("Star-Forge: game simulation initialized");

    performance_frequency = SDL_GetPerformanceFrequency();
    previous_counter = SDL_GetPerformanceCounter();

    SDL_Log("Star-Forge: entering main loop");

    while (running) {
        SDL_Event event;
        SfInput input = { 0 };
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

            if (event.type == SDL_EVENT_FINGER_DOWN ||
                event.type == SDL_EVENT_FINGER_MOTION ||
                event.type == SDL_EVENT_FINGER_UP) {
                update_touch_event(&event.tfinger, &touch, &game);
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

        input.fire = keyboard_state[SDL_SCANCODE_SPACE] || touch.fire_active;
        input.boost = touch.boost_active || keyboard_state[SDL_SCANCODE_LSHIFT];

        if (touch.joystick_active) {
            const float dx = touch.joystick_x - 48.0f;
            const float dy = touch.joystick_y - 132.0f;

            input.turn_left = input.turn_left || dx < -8.0f;
            input.turn_right = input.turn_right || dx > 8.0f;
            input.thrust = input.thrust || dy < -8.0f;
            input.reverse = input.reverse || dy > 8.0f;
        }

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

#ifdef __ANDROID__
        draw_touch_controls(renderer, &touch, &game);
#endif

        SDL_RenderPresent(renderer);
        SDL_Delay(1);
    }

    SDL_Log("Star-Forge: shutting down normally");
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
