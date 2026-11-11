#ifndef STAR_FORGE_GALAXY_H
#define STAR_FORGE_GALAXY_H

#include <stdbool.h>
#include <SDL3/SDL.h>
#include "game.h"

bool sf_galaxy_is_connected(int from_system, int to_system);
int sf_galaxy_hit_test(float x, float y);
void sf_galaxy_render(SDL_Renderer *renderer, const SfGame *game, int selected_system);

#endif
