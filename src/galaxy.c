#include "galaxy.h"

#include <math.h>

typedef struct {
    float x;
    float y;
} SfGalaxyNode;

static const SfGalaxyNode base_nodes[SF_MAX_SYSTEMS] = {
    { 62.0f, 76.0f },
    { 139.0f, 43.0f },
    { 143.0f, 124.0f },
    { 245.0f, 82.0f }
};

static SfGalaxyNode node_position(int index, int viewport_width)
{
    const float horizontal_scale = ((float)viewport_width - 32.0f) / 288.0f;

    return (SfGalaxyNode) {
        16.0f + (base_nodes[index].x - 16.0f) * horizontal_scale,
        base_nodes[index].y
    };
}



int sf_galaxy_hit_test(float x, float y, int viewport_width)
{
    int i;

    for (i = 0; i < SF_MAX_SYSTEMS; ++i) {
        const SfGalaxyNode node = node_position(i, viewport_width);
        const float dx = x - node.x;
        const float dy = y - node.y;

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

void sf_galaxy_render(
    SDL_Renderer *renderer,
    const SfGame *game,
    int selected_system,
    int viewport_width
)
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
                const SfGalaxyNode from = node_position(i, viewport_width);
                const SfGalaxyNode to = node_position(j, viewport_width);
                SDL_RenderLine(renderer, from.x, from.y, to.x, to.y);
            }
        }
    }

    for (i = 0; i < SF_MAX_SYSTEMS; ++i) {
        const SfGalaxyNode node = node_position(i, viewport_width);
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

        draw_disc(renderer, node.x, node.y, selected ? 8 : 6);

        (void)SDL_snprintf(label, sizeof(label), "%s%s",
                           sf_game_system_name(i), current ? " *" : "");
        SDL_RenderDebugText(renderer, node.x - 18.0f,
                            node.y + 12.0f, label);
    }

    (void)SDL_SetRenderDrawColor(renderer, 210, 225, 245, 255);
    if (!game->player.alive) {
        SDL_RenderDebugText(renderer, 9.0f, 139.0f, "SHIP DESTROYED");
    } else if (!game->player.has_ftl_drive) {
        SDL_RenderDebugText(renderer, 9.0f, 139.0f, "FTL DRIVE REQUIRED");
    } else if (game->inventory[SF_ITEM_FUEL] < 5) {
        SDL_RenderDebugText(renderer, 9.0f, 139.0f, "NEED 5 FUEL TO JUMP");
    } else if (selected_system >= 0 && selected_system < SF_MAX_SYSTEMS) {
        char detail[72];
        (void)SDL_snprintf(
            detail, sizeof(detail), "%s  |  FUEL: %d (5 REQUIRED)",
            sf_game_system_name(selected_system),
            game->inventory[SF_ITEM_FUEL]
        );
        SDL_RenderDebugText(renderer, 9.0f, 139.0f, detail);
    } else {
        SDL_RenderDebugText(renderer, 9.0f, 139.0f, "Select a highlighted connected system");
    }
    SDL_RenderDebugText(renderer, 9.0f, 157.0f, "GREEN: CURRENT  BLUE: REACHABLE");
    SDL_RenderDebugText(renderer, 9.0f, 169.0f, "Tap a blue node, then JUMP. X: CLOSE");

    (void)SDL_SetRenderDrawColor(renderer, 32, 65, 100, 255);
    {
        SDL_FRect jump = { (float)viewport_width - 64.0f, 143.0f, 56.0f, 25.0f };
        SDL_FRect close = { (float)viewport_width - 64.0f, 8.0f, 56.0f, 28.0f };
        SDL_RenderFillRect(renderer, &jump);
        SDL_RenderFillRect(renderer, &close);
    }
    (void)SDL_SetRenderDrawColor(renderer, 245, 245, 255, 255);
    SDL_RenderDebugText(renderer, (float)viewport_width - 50.0f, 151.0f, "JUMP");
    SDL_RenderDebugText(renderer, (float)viewport_width - 39.0f, 17.0f, "CLOSE");
}
