#ifndef STAR_FORGE_UI_LAYOUT_H
#define STAR_FORGE_UI_LAYOUT_H

#include <SDL3/SDL.h>
#include <stdbool.h>

#define SF_UI_LOGICAL_HEIGHT 180
#define SF_UI_MIN_LOGICAL_WIDTH 320
#define SF_UI_MAX_LOGICAL_WIDTH 480

typedef struct {
    int viewport_width;
    SDL_FRect fire;
    SDL_FRect boost;
    SDL_FRect loot;
    SDL_FRect ftl;
    SDL_FRect inventory;
    float joystick_center_x;
    float joystick_center_y;
    float joystick_radius;
} SfUiLayout;

static inline bool sf_ui_contains(const SDL_FRect *rect, float x, float y)
{
    return rect != NULL &&
        x >= rect->x && x <= rect->x + rect->w &&
        y >= rect->y && y <= rect->y + rect->h;
}

static inline void sf_ui_layout_init(SfUiLayout *layout, int viewport_width)
{
    const float button_size = 28.0f;
    const float button_gap = 6.0f;
    const float right_margin = 8.0f;
    const float left_column_x = (float)viewport_width -
        right_margin - button_size * 2.0f - button_gap;
    const float right_column_x = (float)viewport_width -
        right_margin - button_size;
    const float top_row_y = 110.0f;
    const float bottom_row_y = 144.0f;

    if (layout == NULL) {
        return;
    }

    layout->viewport_width = viewport_width;
    layout->fire = (SDL_FRect) {
        right_column_x, bottom_row_y, button_size, button_size
    };
    layout->boost = (SDL_FRect) {
        left_column_x, bottom_row_y, button_size, button_size
    };
    layout->loot = (SDL_FRect) {
        right_column_x, top_row_y, button_size, button_size
    };
    layout->ftl = (SDL_FRect) {
        left_column_x, top_row_y, button_size, button_size
    };
    layout->inventory = (SDL_FRect) {
        ((float)viewport_width - 34.0f) * 0.5f,
        bottom_row_y,
        34.0f,
        button_size
    };
    layout->joystick_center_x = 36.0f;
    layout->joystick_center_y = 146.0f;
    layout->joystick_radius = 22.0f;
}

#endif
