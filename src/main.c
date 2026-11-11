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
    const SfGame *game
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

    draw_touch_ring(renderer, 48.0f, 132.0f, 31.0f);
    draw_touch_ring(renderer, knob_x, knob_y, 12.0f);

    set_draw_color(renderer, 30, 70, 115);
    SDL_RenderFillRect(renderer, &fire);
    SDL_RenderFillRect(renderer, &boost);
    SDL_RenderFillRect(renderer, &loot);
    SDL_RenderFillRect(renderer, &ftl);

    set_draw_color(renderer, 125, 195, 245);
    SDL_RenderRect(renderer, &fire);
    SDL_RenderRect(renderer, &boost);
    SDL_RenderRect(renderer, &loot);
    SDL_RenderRect(renderer, &ftl);

    set_draw_color(renderer, 245, 245, 255);
    SDL_RenderDebugText(renderer, 276.0f, 141.0f, "FIRE");
    SDL_RenderDebugText(renderer, 275.0f, 82.0f, "BOOST");
    SDL_RenderDebugText(renderer, 218.0f, 154.0f, "LOOT");
    SDL_RenderDebugText(renderer, 218.0f, 105.0f, "FTL");

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
                update_touch_event(&event.tfinger, &touch, &game);
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

        draw_world(renderer, &game, camera_x, camera_y);

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
