#include "galaxy.h"

#include <math.h>

typedef struct {
    float x;
    float y;
} SfGalaxyNode;

static const SfGalaxyNode nodes[SF_MAX_SYSTEMS] = {
    { 62.0f, 76.0f },
    { 139.0f, 43.0f },
    { 143.0f, 124.0f },
    { 245.0f, 82.0f }
};



int sf_galaxy_hit_test(float x, float y)
{
    int i;

    for (i = 0; i < SF_MAX_SYSTEMS; ++i) {
        const float dx = x - nodes[i].x;
        const float dy = y - nodes[i].y;

        if (dx * dx + dy * dy <= 17.0f * 17.0f) {
            return i;
        }
    }

    return -1;
}

static void draw_disc(SDL_Renderer *renderer, float x, float y, int radius)
{
    int row;

    for (row = -radius; row <= radius; ++row) {
        const float half_width = sqrtf((float)(radius * radius - row * row));
        SDL_RenderLine(renderer, x - half_width, y + (float)row,
                       x + half_width, y + (float)row);
    }
}

void sf_galaxy_render(SDL_Renderer *renderer, const SfGame *game, int selected_system)
{
    int i;
    int j;

    (void)SDL_SetRenderDrawColor(renderer, 4, 8, 20, 255);
    (void)SDL_RenderClear(renderer);

    (void)SDL_SetRenderDrawColor(renderer, 110, 145, 190, 255);
    SDL_RenderDebugText(renderer, 9.0f, 7.0f, "GALAXY NAVIGATION");
    SDL_RenderDebugText(renderer, 9.0f, 19.0f, "Select a connected system");

    for (i = 0; i < SF_MAX_SYSTEMS; ++i) {
        for (j = i + 1; j < SF_MAX_SYSTEMS; ++j) {
            if (sf_game_systems_connected(i, j)) {
                (void)SDL_SetRenderDrawColor(renderer, 55, 93, 130, 255);
                SDL_RenderLine(renderer, nodes[i].x, nodes[i].y,
                               nodes[j].x, nodes[j].y);
            }
        }
    }

    for (i = 0; i < SF_MAX_SYSTEMS; ++i) {
        const bool current = i == game->current_system;
        const bool reachable = sf_game_systems_connected(game->current_system, i);
        const bool selected = i == selected_system;
        char label[48];

        if (selected) {
            (void)SDL_SetRenderDrawColor(renderer, 255, 210, 95, 255);
        } else if (current) {
            (void)SDL_SetRenderDrawColor(renderer, 100, 235, 150, 255);
        } else if (reachable) {
            (void)SDL_SetRenderDrawColor(renderer, 100, 175, 245, 255);
        } else {
            (void)SDL_SetRenderDrawColor(renderer, 75, 82, 100, 255);
        }

        draw_disc(renderer, nodes[i].x, nodes[i].y, selected ? 8 : 6);

        (void)SDL_snprintf(label, sizeof(label), "%s%s",
                           sf_game_system_name(i), current ? " *" : "");
        SDL_RenderDebugText(renderer, nodes[i].x - 18.0f,
                            nodes[i].y + 12.0f, label);
    }

    (void)SDL_SetRenderDrawColor(renderer, 210, 225, 245, 255);
    SDL_RenderDebugText(renderer, 9.0f, 157.0f, "GREEN: CURRENT  BLUE: REACHABLE");
    SDL_RenderDebugText(renderer, 9.0f, 169.0f, "Tap a blue node, then JUMP. X: CLOSE");

    (void)SDL_SetRenderDrawColor(renderer, 32, 65, 100, 255);
    {
        SDL_FRect jump = { 264.0f, 143.0f, 47.0f, 25.0f };
        SDL_FRect close = { 264.0f, 8.0f, 47.0f, 22.0f };
        SDL_RenderFillRect(renderer, &jump);
        SDL_RenderFillRect(renderer, &close);
    }
    (void)SDL_SetRenderDrawColor(renderer, 245, 245, 255, 255);
    SDL_RenderDebugText(renderer, 273.0f, 151.0f, "JUMP");
    SDL_RenderDebugText(renderer, 280.0f, 15.0f, "X");
}
