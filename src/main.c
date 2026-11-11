#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "game.h"
#include "render.h"
#include "assets.h"
#include "galaxy.h"
#include "ui_layout.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#define SF_WINDOW_WIDTH 960
#define SF_WINDOW_HEIGHT 540
static int logical_width = SF_UI_MIN_LOGICAL_WIDTH;
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
} SfTouchState;

static void handle_ui_tap(
    SfGame *game,
    bool *inventory_open,
    float x,
    float y,
    int viewport_width
)
{
    if (game == NULL || inventory_open == NULL) {
        return;
    }

    if (!*inventory_open) {
        return;
    }

    x -= ((float)viewport_width - 320.0f) * 0.5f;

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

static void release_touch_controls(SfTouchState *touch)
{
    if (touch == NULL) {
        return;
    }

    touch->joystick_active = false;
    touch->fire_active = false;
    touch->boost_active = false;
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
    SDL_Renderer *renderer,
    const SfUiLayout *layout
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
        return;
    }

    if (finger->type == SDL_EVENT_FINGER_DOWN &&
        (x < 0.0f || x > (float)layout->viewport_width ||
         y < 0.0f || y > (float)SF_UI_LOGICAL_HEIGHT)) {
        return;
    }

    if (ftl_transition_timer > 0.0f) {
        if (finger->type == SDL_EVENT_FINGER_UP) {
            if (touch->joystick_active && finger->fingerID == touch->joystick_id) touch->joystick_active = false;
            if (touch->fire_active && finger->fingerID == touch->fire_id) touch->fire_active = false;
            if (touch->boost_active && finger->fingerID == touch->boost_id) touch->boost_active = false;
        }
        return;
    }

    if (galaxy_map_open) {
        if (finger->type == SDL_EVENT_FINGER_DOWN) {
            if (x >= (float)layout->viewport_width - 64.0f && y >= 8.0f && y <= 36.0f) {
                galaxy_map_open = false;
                galaxy_selected_system = -1;
            } else if (x >= (float)layout->viewport_width - 64.0f && y >= 140.0f) {
                if (galaxy_selected_system >= 0 &&
                    sf_game_systems_connected(game->current_system, galaxy_selected_system) &&
                    game->player.alive && game->player.has_ftl_drive &&
                    game->inventory[SF_ITEM_FUEL] >= 5) {
                    ftl_destination = galaxy_selected_system;
                    ftl_transition_timer = 1.45f;
                    galaxy_map_open = false;
                    galaxy_selected_system = -1;
                }
            } else {
                const int node = sf_galaxy_hit_test(x, y, layout->viewport_width);
                if (node >= 0 && sf_game_systems_connected(game->current_system, node)) {
                    galaxy_selected_system = node;
                }
            }
        }
        return;
    }

    if (finger->type == SDL_EVENT_FINGER_DOWN) {
        if (*inventory_open) {
            handle_ui_tap(game, inventory_open, x, y, layout->viewport_width);
            return;
        }

        if (sf_ui_contains(&layout->ftl, x, y)) {
            galaxy_map_open = true;
            galaxy_selected_system = -1;
            release_touch_controls(touch);
        } else if (sf_ui_contains(&layout->inventory, x, y)) {
            *inventory_open = true;
            release_touch_controls(touch);
        } else if (x < layout->inventory.x - 8.0f &&
                   y >= (float)SF_UI_LOGICAL_HEIGHT * 0.45f &&
                   !touch->joystick_active) {
            touch->joystick_active = true;
            touch->joystick_id = finger->fingerID;
            touch->joystick_origin_x = x;
            touch->joystick_origin_y = y;
            touch->joystick_x = x;
            touch->joystick_y = y;
        } else if (sf_ui_contains(&layout->fire, x, y) && !touch->fire_active) {
            touch->fire_active = true;
            touch->fire_id = finger->fingerID;
        } else if (sf_ui_contains(&layout->boost, x, y) && !touch->boost_active) {
            touch->boost_active = true;
            touch->boost_id = finger->fingerID;
        } else if (sf_ui_contains(&layout->loot, x, y)) {
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
    const SfAssets *assets,
    const SfUiLayout *layout
)
{
    const SDL_FRect *buttons[4] = {
        &layout->fire, &layout->boost, &layout->loot, &layout->ftl
    };
    const float center_x = touch->joystick_active ? touch->joystick_origin_x : layout->joystick_center_x;
    const float center_y = touch->joystick_active ? touch->joystick_origin_y : layout->joystick_center_y;
    float knob_x = center_x;
    float knob_y = center_y;
    int i;

    if (touch->joystick_active) {
        float dx = touch->joystick_x - center_x;
        float dy = touch->joystick_y - center_y;
        const float length = sqrtf(dx * dx + dy * dy);
        if (length > layout->joystick_radius) {
            dx *= layout->joystick_radius / length;
            dy *= layout->joystick_radius / length;
        }
        knob_x += dx;
        knob_y += dy;
    }

    {
        SDL_Texture *base = sf_assets_get(assets, SF_ASSET_JOYSTICK_BASE);
        SDL_Texture *knob = sf_assets_get(assets, SF_ASSET_JOYSTICK_KNOB);
        const float diameter = layout->joystick_radius * 2.0f;
        SDL_FRect base_rect = { center_x - layout->joystick_radius, center_y - layout->joystick_radius, diameter, diameter };
        const float knob_radius = layout->joystick_radius * 0.38f;
        SDL_FRect knob_rect = { knob_x - knob_radius, knob_y - knob_radius, knob_radius * 2.0f, knob_radius * 2.0f };
        if (base != NULL) (void)SDL_RenderTexture(renderer, base, NULL, &base_rect);
        else draw_touch_ring(renderer, center_x, center_y, layout->joystick_radius - 1.0f);
        if (knob != NULL) (void)SDL_RenderTexture(renderer, knob, NULL, &knob_rect);
        else draw_touch_ring(renderer, knob_x, knob_y, knob_radius - 1.0f);
    }

    for (i = 0; i < 4; ++i) {
        (void)SDL_SetRenderDrawColor(renderer, 27, 48, 72, 225);
        SDL_RenderFillRect(renderer, buttons[i]);
        (void)SDL_SetRenderDrawColor(renderer, 105, 155, 205, 255);
        SDL_RenderRect(renderer, buttons[i]);
    }

    (void)SDL_SetRenderDrawColor(renderer, 245, 248, 255, 255);
    SDL_RenderDebugText(renderer, buttons[0]->x + 10.0f, buttons[0]->y + 10.0f, "F");
    SDL_RenderDebugText(renderer, buttons[1]->x + 8.0f, buttons[1]->y + 10.0f, "B");
    SDL_RenderDebugText(renderer, buttons[2]->x + 8.0f, buttons[2]->y + 10.0f, "L");
    SDL_RenderDebugText(renderer, buttons[3]->x + 4.0f, buttons[3]->y + 10.0f, "FTL");

    (void)SDL_SetRenderDrawColor(renderer, 27, 48, 72, 225);
    SDL_RenderFillRect(renderer, &layout->inventory);
    (void)SDL_SetRenderDrawColor(renderer, 225, 235, 245, 255);
    SDL_RenderRect(renderer, &layout->inventory);
    SDL_RenderDebugText(renderer, layout->inventory.x + 9.0f, layout->inventory.y + 10.0f, "BAG");

}

int main(int argc, char **argv)
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SfGame game;
    SfAssets assets;
    bool running = true;
    bool inventory_open = false;
    bool app_active = true;
    SfTouchState touch = { 0 };
    SfUiLayout layout;
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

    {
        int output_width = 0;
        int output_height = 0;
        if (SDL_GetRenderOutputSize(renderer, &output_width, &output_height) &&
            output_width > 0 && output_height > 0) {
            logical_width = (int)lroundf(
                (float)output_width * (float)SF_UI_LOGICAL_HEIGHT /
                (float)output_height
            );
        }
        if (logical_width < SF_UI_MIN_LOGICAL_WIDTH) logical_width = SF_UI_MIN_LOGICAL_WIDTH;
        if (logical_width > SF_UI_MAX_LOGICAL_WIDTH) logical_width = SF_UI_MAX_LOGICAL_WIDTH;
        (void)SDL_SetRenderLogicalPresentation(
            renderer, logical_width, SF_UI_LOGICAL_HEIGHT,
            SDL_LOGICAL_PRESENTATION_LETTERBOX
        );
        sf_ui_layout_init(&layout, logical_width);
    }

    sf_game_init(&game);
    SDL_Log("Star-Forge: game simulation initialized");
    (void)sf_assets_init(&assets, renderer);

    performance_frequency = SDL_GetPerformanceFrequency();
    if (performance_frequency == 0) {
        SDL_Log("FATAL: performance counter frequency is unavailable");
        sf_assets_destroy(&assets);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    previous_counter = SDL_GetPerformanceCounter();

    SDL_Log("Star-Forge: entering main loop");

    while (running) {
        SDL_Event event;
        SfInput input = { 0 };
        Uint64 current_counter;
        double frame_time;
        int camera_x;
        int camera_y;
        int output_width = 0;
        int output_height = 0;

        if (SDL_GetRenderOutputSize(renderer, &output_width, &output_height) &&
            output_width > 0 && output_height > 0) {
            int next_width = (int)lroundf(
                (float)output_width * (float)SF_UI_LOGICAL_HEIGHT /
                (float)output_height
            );
            if (next_width < SF_UI_MIN_LOGICAL_WIDTH) next_width = SF_UI_MIN_LOGICAL_WIDTH;
            if (next_width > SF_UI_MAX_LOGICAL_WIDTH) next_width = SF_UI_MAX_LOGICAL_WIDTH;
            if (next_width != logical_width) {
                logical_width = next_width;
                release_touch_controls(&touch);
                (void)SDL_SetRenderLogicalPresentation(
                    renderer, logical_width, SF_UI_LOGICAL_HEIGHT,
                    SDL_LOGICAL_PRESENTATION_LETTERBOX
                );
                sf_ui_layout_init(&layout, logical_width);
            }
        }

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }

            if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST ||
                event.type == SDL_EVENT_WINDOW_HIDDEN) {
                app_active = false;
                release_touch_controls(&touch);
            }

            if (event.type == SDL_EVENT_WINDOW_FOCUS_GAINED ||
                event.type == SDL_EVENT_WINDOW_SHOWN) {
                app_active = true;
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
                    renderer,
                    &layout
                );
            }


        }

        input.fire = touch.fire_active;
        input.boost = touch.boost_active;

        if (touch.joystick_active) {
            const float dx = touch.joystick_x - touch.joystick_origin_x;
            const float dy = touch.joystick_y - touch.joystick_origin_y;
            const float distance = sqrtf(dx * dx + dy * dy);
            const float maximum_radius = layout.joystick_radius;
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
            if (app_active && !galaxy_map_open && !inventory_open) {
                if (ftl_transition_timer > 0.0f) {
                    ftl_transition_timer -= (float)SF_FIXED_STEP;
                    if (ftl_transition_timer <= 0.0f && ftl_destination >= 0) {
                        if (sf_game_jump_to_system(&game, ftl_destination)) {
                            arrival_effect_timer = 1.0f;
                        }
                        ftl_destination = -1;
                    }
                } else {
                    sf_game_update(&game, &input, (float)SF_FIXED_STEP);
                }

                if (arrival_effect_timer > 0.0f) {
                    arrival_effect_timer = fmaxf(
                        0.0f,
                        arrival_effect_timer - (float)SF_FIXED_STEP
                    );
                }
            }
            accumulator -= SF_FIXED_STEP;
        }

        camera_x = (int)(game.player.x - (float)logical_width * 0.5f);
        camera_y = (int)(game.player.y - (float)SF_UI_LOGICAL_HEIGHT * 0.5f);



        if (galaxy_map_open) {
            sf_galaxy_render(renderer, &game, galaxy_selected_system, logical_width);
        } else {
            sf_render_world(
                renderer, &game, &assets, camera_x, camera_y,
                ftl_transition_timer <= 0.0f, logical_width
            );
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
            sf_render_inventory_panel(renderer, &game, logical_width);
        }

        if (!galaxy_map_open && !inventory_open) {
            draw_touch_controls(renderer, &touch, &assets, &layout);
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
