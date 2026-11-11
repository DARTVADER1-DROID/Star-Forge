#ifndef STAR_FORGE_RENDER_H
#define STAR_FORGE_RENDER_H

#include <SDL3/SDL.h>

#include "game.h"
#include "assets.h"

void sf_render_world(
    SDL_Renderer *renderer,
    const SfGame *game,
    const SfAssets *assets,
    int camera_x,
    int camera_y,
    bool draw_player,
    int viewport_width
);

void sf_render_inventory_panel(
    SDL_Renderer *renderer,
    const SfGame *game,
    int viewport_width
);

#endif
