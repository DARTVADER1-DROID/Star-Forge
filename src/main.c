#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "game.h"
#include "render.h"
#include "assets.h"
#include "galaxy.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#define SF_WINDOW_WIDTH 960
#define SF_WINDOW_HEIGHT 540
#define SF_LOGICAL_WIDTH 320
#define SF_LOGICAL_HEIGHT 180
#define SF_FIXED_STEP (1.0 / 60.0)

static bool galaxy_map_open;
static int galaxy_selected_system = -1;
static int ftl_destination = -1;
static float ftl_transition_timer;
static float arrival_effect_timer;

typedef struct {
    SDL_FingerID joystick_id;
    SDL_FingerID fire_id;
    SDL_FingerID boost_id;
    bool joystick_active;
    bool fire_active;
    bool boost_active;
    float joystick_origin_x;
    float joystick_origin_y;
    float joystick_x;
    float joystick_y;
    bool mouse_joystick_active;
} SfTouchState;

static void set_draw_color(
    SDL_Renderer *renderer,
    Uint8 red,
    Uint8 green,
    Uint8 blue
)
{
    (void)SDL_SetRenderDrawColor(renderer, red, green, blue, 255);
}

static void handle_ui_tap(
    SfGame *game,
    bool *inventory_open,
    float x,
    float y
)
{
    if (game == NULL || inventory_open == NULL) {
        return;
    }

    if (!*inventory_open) {
        if (x >= 160.0f && x <= 207.0f && y >= 143.0f && y <= 174.0f) {
            *inventory_open = true;
        }
        return;
    }

    if (x >= 235.0f && x <= 260.0f && y >= 48.0f && y <= 68.0f) {
        *inventory_open = false;
        return;
    }

    if (y >= 65.0f && y <= 82.0f && x >= 48.0f && x < 273.0f) {
        const int item = (int)((x - 48.0f) / 45.0f);

        if (item >= 0 && item < SF_ITEM_COUNT) {
            game->selected_item = item;
            if (game->selected_quantity > game->inventory[item]) {
                game->selected_quantity = game->inventory[item];
            }
            if (game->selected_quantity < 1) {
                game->selected_quantity = 1;
            }
        }
        return;
    }

    if (y >= 86.0f && y <= 102.0f) {
        if (x >= 52.0f && x <= 105.0f && game->selected_quantity > 1) {
            game->selected_quantity--;
        } else if (x >= 110.0f && x <= 165.0f &&
                   game->selected_quantity < 999) {
            game->selected_quantity++;
        } else if (x >= 185.0f && x <= 270.0f) {
            game->npc_salvage_enabled = !game->npc_salvage_enabled;
        }
        return;
    }

    if (y >= 108.0f && y <= 128.0f) {
        if (x >= 50.0f && x <= 150.0f) {
            (void)sf_game_jettison_selected(game);
        } else if (x >= 160.0f && x <= 270.0f) {
            (void)sf_game_sell_selected(game);
        }
    }
}

static bool window_to_logical(
    SDL_Renderer *renderer,
    float window_x,
    float window_y,
    float *logical_x,
    float *logical_y
)
{
    return SDL_RenderCoordinatesFromWindow(
        renderer,
        window_x,
        window_y,
        logical_x,
        logical_y
    );
}

static void update_touch_event(
    const SDL_TouchFingerEvent *finger,
    SfTouchState *touch,
    SfGame *game,
    bool *inventory_open,
    SDL_Window *window,
    SDL_Renderer *renderer
)
{
    int window_width = 0;
    int window_height = 0;
    float x = 0.0f;
    float y = 0.0f;

    SDL_GetWindowSize(window, &window_width, &window_height);
    if (window_width <= 0 || window_height <= 0 ||
        !window_to_logical(
            renderer,
            finger->x * (float)window_width,
            finger->y * (float)window_height,
            &x,
            &y
        )) {
        return;
    }

    if (galaxy_map_open) {
        if (finger->type == SDL_EVENT_FINGER_DOWN) {
            if (x >= 264.0f && y >= 8.0f && y <= 30.0f) {
                galaxy_map_open = false;
                galaxy_selected_system = -1;
            } else if (x >= 260.0f && y >= 140.0f) {
                if (galaxy_selected_system >= 0 &&
                    sf_game_systems_connected(game->current_system, galaxy_selected_system) &&
                    game->player.has_ftl_drive && game->inventory[SF_ITEM_FUEL] >= 5) {
                    ftl_destination = galaxy_selected_system;
                    ftl_transition_timer = 1.45f;
                    galaxy_map_open = false;
                    galaxy_selected_system = -1;
                }
            } else {
                const int node = sf_galaxy_hit_test(x, y);
                if (node >= 0 && sf_game_systems_connected(game->current_system, node)) {
                    galaxy_selected_system = node;
                }
            }
        }
        return;
    }

    if (finger->type == SDL_EVENT_FINGER_DOWN) {
        if (*inventory_open) {
            handle_ui_tap(game, inventory_open, x, y);
            return;
        }

        if (x >= 257.0f && x <= 281.0f && y >= 116.0f && y <= 140.0f) {
            galaxy_map_open = true;
            galaxy_selected_system = -1;
        } else if (x >= 160.0f && x <= 207.0f && y >= 143.0f) {
            handle_ui_tap(game, inventory_open, x, y);
        } else if (x < (float)SF_LOGICAL_WIDTH * 0.48f &&
                   !touch->joystick_active) {
            touch->joystick_active = true;
            touch->joystick_id = finger->fingerID;
            touch->joystick_origin_x = x;
            touch->joystick_origin_y = y;
            touch->joystick_x = x;
            touch->joystick_y = y;
        } else if (x >= 286.0f && y >= 145.0f && !touch->fire_active) {
            touch->fire_active = true;
            touch->fire_id = finger->fingerID;
        } else if (x >= 257.0f && x < 286.0f && y >= 145.0f &&
                   !touch->boost_active) {
            touch->boost_active = true;
            touch->boost_id = finger->fingerID;
        } else if (x >= 286.0f && y >= 116.0f && y < 145.0f) {
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

    (void)SDL_SetRenderDrawColor(renderer, 105, 165, 225, 255);

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
    const SfGame *game,
    const SfAssets *assets
)
{
    SDL_FRect buttons[4] = {
        { 286.0f, 145.0f, 24.0f, 24.0f },
        { 257.0f, 145.0f, 24.0f, 24.0f },
        { 286.0f, 116.0f, 24.0f, 24.0f },
        { 257.0f, 116.0f, 24.0f, 24.0f }
    };
    const float center_x = touch->joystick_active ? touch->joystick_origin_x : 29.0f;
    const float center_y = touch->joystick_active ? touch->joystick_origin_y : 148.0f;
    float knob_x = center_x;
    float knob_y = center_y;
    int i;

    if (touch->joystick_active) {
        float dx = touch->joystick_x - center_x;
        float dy = touch->joystick_y - center_y;
        const float length = sqrtf(dx * dx + dy * dy);
        if (length > 14.0f) {
            dx *= 14.0f / length;
            dy *= 14.0f / length;
        }
        knob_x += dx;
        knob_y += dy;
    }

    {
        SDL_Texture *base = sf_assets_get(assets, SF_ASSET_JOYSTICK_BASE);
        SDL_Texture *knob = sf_assets_get(assets, SF_ASSET_JOYSTICK_KNOB);
        SDL_FRect base_rect = { center_x - 18.0f, center_y - 18.0f, 36.0f, 36.0f };
        SDL_FRect knob_rect = { knob_x - 7.0f, knob_y - 7.0f, 14.0f, 14.0f };
        if (base != NULL) (void)SDL_RenderTexture(renderer, base, NULL, &base_rect);
        else draw_touch_ring(renderer, center_x, center_y, 17.0f);
        if (knob != NULL) (void)SDL_RenderTexture(renderer, knob, NULL, &knob_rect);
        else draw_touch_ring(renderer, knob_x, knob_y, 6.0f);
    }

    for (i = 0; i < 4; ++i) {
        (void)SDL_SetRenderDrawColor(renderer, 27, 48, 72, 225);
        SDL_RenderFillRect(renderer, &buttons[i]);
        (void)SDL_SetRenderDrawColor(renderer, 105, 155, 205, 255);
        SDL_RenderRect(renderer, &buttons[i]);
    }

    (void)SDL_SetRenderDrawColor(renderer, 245, 248, 255, 255);
    SDL_RenderDebugText(renderer, 293.0f, 153.0f, "F");
    SDL_RenderDebugText(renderer, 264.0f, 153.0f, "B");
    SDL_RenderDebugText(renderer, 293.0f, 124.0f, "L");
    SDL_RenderDebugText(renderer, 259.0f, 124.0f, "FTL");

    {
        char status[96];
        (void)snprintf(status, sizeof(status), "%s H%.0f A%.0f E%.0f",
                       sf_game_system_name(game->current_system),
                       game->player.hull, game->player.armor, game->player.energy);
        (void)SDL_SetRenderDrawColor(renderer, 220, 232, 245, 255);
        SDL_RenderDebugText(renderer, 5.0f, 5.0f, status);
    }
}

int main(int argc, char **argv)
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SfGame game;
    SfAssets assets;
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
    (void)sf_assets_init(&assets, renderer);

    performance_frequency = SDL_GetPerformanceFrequency();
    previous_counter = SDL_GetPerformanceCounter();

    SDL_Log("Star-Forge: entering main loop");

    while (running) {
        SDL_Event event;
        SfInput input = { 0 };
        Uint64 current_counter;
        double frame_time;
        int window_width = SF_WINDOW_WIDTH;
        int window_height = SF_WINDOW_HEIGHT;
        int camera_x;
        int camera_y;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }

            if (event.type == SDL_EVENT_FINGER_DOWN ||
                event.type == SDL_EVENT_FINGER_MOTION ||
                event.type == SDL_EVENT_FINGER_UP) {
                update_touch_event(
                    &event.tfinger,
                    &touch,
                    &game,
                    &inventory_open,
                    window,
                    renderer
                );
            }


        }

        if (ftl_transition_timer > 0.0f) {
            ftl_transition_timer -= (float)SF_FIXED_STEP;
            if (ftl_transition_timer <= 0.0f && ftl_destination >= 0) {
                (void)sf_game_jump_to_system(&game, ftl_destination);
                ftl_destination = -1;
                arrival_effect_timer = 1.0f;
            }
        }
        if (arrival_effect_timer > 0.0f) {
            arrival_effect_timer = fmaxf(0.0f, arrival_effect_timer - (float)SF_FIXED_STEP);
        }

        input.fire = touch.fire_active;
        input.boost = touch.boost_active;

        if (touch.joystick_active) {
            const float dx = touch.joystick_x - touch.joystick_origin_x;
            const float dy = touch.joystick_y - touch.joystick_origin_y;
            const float distance = sqrtf(dx * dx + dy * dy);
            const float maximum_radius = 14.0f
            const float normalized_distance = fminf(
                distance / maximum_radius,
                1.0f
            );

            if (distance > 0.5f) {
                const float dead_zone = 0.12f;
                input.steering_x = dx / distance;
                input.steering_y = dy / distance;
                input.steering_strength = normalized_distance <= dead_zone
                    ? 0.0f
                    : (normalized_distance - dead_zone) / (1.0f - dead_zone);
            }
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
            (void)window_to_logical(
                renderer,
                mouse_x,
                mouse_y,
                &mouse_x,
                &mouse_y
            );
        }

        if (galaxy_map_open) {
            sf_galaxy_render(renderer, &game, galaxy_selected_system);
        } else {
            sf_render_world(renderer, &game, &assets, camera_x, camera_y);
            if (ftl_transition_timer > 0.0f) {
                const float pulse = 1.0f - ftl_transition_timer / 1.45f;
                (void)SDL_SetRenderDrawColor(renderer, 120, 190, 255, 210);
                draw_touch_ring(renderer, 160.0f, 90.0f, 12.0f + pulse * 95.0f);
                SDL_RenderDebugText(renderer, 132.0f, 86.0f, "FTL TRANSIT");
            } else if (arrival_effect_timer > 0.0f) {
                const float pulse = 1.0f - arrival_effect_timer;
                (void)SDL_SetRenderDrawColor(renderer, 145, 220, 255, 210);
                draw_touch_ring(renderer, 160.0f, 90.0f, 8.0f + pulse * 55.0f);
            }
        }

        if (inventory_open && !galaxy_map_open) {
            sf_render_inventory_panel(renderer, &game);
        }

        if (!galaxy_map_open) {
            draw_touch_controls(renderer, &touch, &game, &assets);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(1);
    }

    SDL_Log("Star-Forge: shutting down normally");
    sf_assets_destroy(&assets);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
