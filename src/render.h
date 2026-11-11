#ifndef STAR_FORGE_RENDER_H
#define STAR_FORGE_RENDER_H

#include <SDL3/SDL.h>

#include "game.h"

void sf_render_world(
    SDL_Renderer *renderer,
    const SfGame *game,
    int camera_x,
    int camera_y
);

void sf_render_hover_info(
    SDL_Renderer *renderer,
    const SfGame *game,
    float pointer_x,
    float pointer_y,
    int camera_x,
    int camera_y
);

void sf_render_inventory_panel(
    SDL_Renderer *renderer,
    const SfGame *game
);

#endif
