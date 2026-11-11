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

typedef struct {
    SDL_FingerID joystick_id;
    SDL_FingerID fire_id;
    SDL_FingerID boost_id;
    bool joystick_active;
    bool fire_active;
    bool boost_active;
    float joystick_x;
    float joystick_y;
    bool mouse_joystick_active;
} SfTouchState;

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

static void update_touch_event(
    const SDL_TouchFingerEvent *finger,
    SfTouchState *touch,
    SfGame *game
)
{
    const float x = finger->x * (float)SF_LOGICAL_WIDTH;
    const float y = finger->y * (float)SF_LOGICAL_HEIGHT;

    if (finger->type == SDL_EVENT_FINGER_DOWN) {
        if (*inventory_open) {\n            handle_ui_tap(game, inventory_open, x, y);\n            return;\n        }\n\n        if (x >= 160.0f && x <= 207.0f && y >= 143.0f) {\n            handle_ui_tap(game, inventory_open, x, y);\n        } else if (x < 155.0f && !touch->joystick_active) {
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
        } else if (x > 175.0f && x <= 245.0f && y > 85.0f && y <= 125.0f) {
            (void)sf_game_jump_to_system(
                game,
                (game->current_system + 1) % SF_MAX_SYSTEMS
            );
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
    const SfGame *game,
    const SfAssets *assets
)
{
    SDL_FRect fire = { 268.0f, 125.0f, 43.0f, 43.0f };
    SDL_FRect boost = { 270.0f, 69.0f, 38.0f, 38.0f };
    SDL_FRect loot = { 211.0f, 145.0f, 38.0f, 27.0f };
    SDL_FRect ftl = { 211.0f, 96.0f, 38.0f, 27.0f };
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

    {
        SDL_Texture *base = sf_assets_get(assets, SF_ASSET_JOYSTICK_BASE);
        SDL_Texture *knob = sf_assets_get(assets, SF_ASSET_JOYSTICK_KNOB);
        SDL_Texture *fire_texture = sf_assets_get(
            assets,
            touch->fire_active ? SF_ASSET_FIRE_DOWN : SF_ASSET_FIRE_UP
        );
        SDL_Texture *boost_texture = sf_assets_get(
            assets,
            touch->boost_active ? SF_ASSET_BOOST_DOWN : SF_ASSET_BOOST_UP
        );
        SDL_Texture *loot_texture = sf_assets_get(assets, SF_ASSET_LOOT_UP);
        SDL_FRect base_rect = { 16.0f, 100.0f, 64.0f, 64.0f };
        SDL_FRect knob_rect = { knob_x - 13.0f, knob_y - 13.0f, 26.0f, 26.0f };

        if (base) (void)SDL_RenderTexture(renderer, base, NULL, &base_rect);
        else draw_touch_ring(renderer, 48.0f, 132.0f, 31.0f);
        if (knob) (void)SDL_RenderTexture(renderer, knob, NULL, &knob_rect);
        else draw_touch_ring(renderer, knob_x, knob_y, 12.0f);
        if (fire_texture) (void)SDL_RenderTexture(renderer, fire_texture, NULL, &fire);
        else { set_draw_color(renderer, 30, 70, 115); SDL_RenderFillRect(renderer, &fire); }
        if (boost_texture) (void)SDL_RenderTexture(renderer, boost_texture, NULL, &boost);
        else { set_draw_color(renderer, 30, 70, 115); SDL_RenderFillRect(renderer, &boost); }
        if (loot_texture) (void)SDL_RenderTexture(renderer, loot_texture, NULL, &loot);
        else { set_draw_color(renderer, 30, 70, 115); SDL_RenderFillRect(renderer, &loot); }
        set_draw_color(renderer, 30, 70, 115);
        SDL_RenderFillRect(renderer, &ftl);
    }

    set_draw_color(renderer, 245, 245, 255);
    SDL_RenderDebugText(renderer, 276.0f, 141.0f, "FIRE");
    SDL_RenderDebugText(renderer, 275.0f, 82.0f, "BOOST");
    SDL_RenderDebugText(renderer, 218.0f, 154.0f, "LOOT");
    SDL_RenderDebugText(renderer, 218.0f, 105.0f, "FTL");\n    SDL_RenderDebugText(renderer, 165.0f, 154.0f, "CARGO");

    {
        char energy_text[24];

        (void)snprintf(
            energy_text,
            sizeof(energy_text),
            "ENERGY %.0f%%",
            game->player.energy
        );
        SDL_RenderDebugText(renderer, 5.0f, 41.0f, energy_text);
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

            if (event.type == SDL_EVENT_FINGER_DOWN ||
                event.type == SDL_EVENT_FINGER_MOTION ||
                event.type == SDL_EVENT_FINGER_UP) {
                update_touch_event(&event.tfinger, &touch, &game, &inventory_open);
            }

            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
                event.type == SDL_EVENT_MOUSE_BUTTON_UP ||
                event.type == SDL_EVENT_MOUSE_MOTION) {
                float pointer_x = 0.0f;
                float pointer_y = 0.0f;
                int width = SF_WINDOW_WIDTH;
                int height = SF_WINDOW_HEIGHT;
                bool pressed = event.type != SDL_EVENT_MOUSE_BUTTON_UP;

                SDL_GetWindowSize(window, &width, &height);
                if (event.type == SDL_EVENT_MOUSE_MOTION) {
                    pointer_x = event.motion.x;
                    pointer_y = event.motion.y;
                    pressed = (event.motion.state & SDL_BUTTON_LMASK) != 0;
                } else {
                    pointer_x = event.button.x;
                    pointer_y = event.button.y;
                }

                if (width > 0 && height > 0) {
                    pointer_x *= (float)SF_LOGICAL_WIDTH / (float)width;
                    pointer_y *= (float)SF_LOGICAL_HEIGHT / (float)height;
                }

                if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&\n                    (inventory_open || (pointer_x >= 160.0f && pointer_x <= 207.0f &&\n                                        pointer_y >= 143.0f))) {\n                    handle_ui_tap(&game, &inventory_open, pointer_x, pointer_y);\n                } else if (pointer_x < 155.0f) {
                    touch.mouse_joystick_active = pressed;
                    touch.joystick_active = pressed;
                    touch.joystick_x = pointer_x;
                    touch.joystick_y = pointer_y;
                } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                    if (pointer_x > 268.0f && pointer_y > 125.0f) {
                        touch.fire_active = true;
                    } else if (pointer_x > 268.0f && pointer_y <= 110.0f) {
                        touch.boost_active = true;
                    } else if (pointer_x > 205.0f && pointer_x <= 255.0f &&
                               pointer_y > 140.0f) {
                        (void)sf_game_collect_nearest(&game);
                    } else if (pointer_x > 205.0f && pointer_x <= 255.0f &&
                               pointer_y > 90.0f && pointer_y <= 130.0f) {
                        (void)sf_game_jump_to_system(
                            &game,
                            (game.current_system + 1) % SF_MAX_SYSTEMS
                        );
                    }
                } else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
                    touch.fire_active = false;
                    touch.boost_active = false;
                    if (touch.mouse_joystick_active) {
                        touch.mouse_joystick_active = false;
                        touch.joystick_active = false;
                    }
                }
            }
        }

        input.fire = touch.fire_active;
        input.boost = touch.boost_active;

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

        sf_render_world(renderer, &game, &assets, camera_x, camera_y);

        sf_render_hover_info(
            renderer,
            &game,
            mouse_x,
            mouse_y,
            camera_x,
            camera_y
        );

        if (inventory_open) {
            sf_render_inventory_panel(renderer, &game);
        }

        draw_touch_controls(renderer, &touch, &game, &assets);

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
