#include "assets.h"

#include <limits.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#define SF_ASSET_PIXEL_BUDGET (8u * 1024u * 1024u)
#include "stb_image.h"

static const char *const asset_paths[SF_ASSET_COUNT] = {
    "assets/ships/ship_player_runner.png",
    "assets/ships/ship_cargo_hauler.png",
    "assets/ships/ship_pirate_interceptor.png",
    "assets/ships/ship_military_destroyer.png",
    "assets/stations/station_ring_hub.png",
    "assets/planets/planet_terran_64.png",
    "assets/planets/planet_gas_96.png",
    "assets/planets/planet_moon_32.png",
    "assets/asteroids/asteroid_large_01.png",
    "assets/salvage/salvage_metal.png",
    "assets/salvage/salvage_fuel.png",
    "assets/salvage/salvage_food.png",
    "assets/salvage/salvage_medicine.png",
    "assets/salvage/salvage_weapon.png",
    "assets/ui/ui_btn_fire_up.png",
    "assets/ui/ui_btn_fire_down.png",
    "assets/ui/ui_btn_boost_up.png",
    "assets/ui/ui_btn_boost_down.png",
    "assets/ui/ui_btn_loot_up.png",
    "assets/ui/ui_btn_loot_down.png",
    "assets/ui/ui_joystick_base.png",
    "assets/ui/ui_joystick_knob.png"
};

static SDL_Texture *load_texture(
    SDL_Renderer *renderer,
    const char *path,
    size_t *remaining_pixel_budget
)
{
    size_t file_size = 0;
    void *file_data;
    unsigned char *pixels;
    int width = 0;
    int height = 0;
    int source_channels = 0;
    SDL_Surface *surface;
    SDL_Texture *texture;
    size_t pixel_count;

    if (remaining_pixel_budget == NULL || *remaining_pixel_budget == 0) {
        SDL_Log("Skipped sprite '%s': texture memory budget exhausted", path);
        return NULL;
    }

    file_data = SDL_LoadFile(path, &file_size);

    if (file_data == NULL || file_size == 0 || file_size > (size_t)INT_MAX) {
        SDL_free(file_data);
        SDL_Log("Could not read sprite '%s': %s", path, SDL_GetError());
        return NULL;
    }

    if (!stbi_info_from_memory(
            (const stbi_uc *)file_data,
            (int)file_size,
            &width,
            &height,
            &source_channels
        ) ||
        width <= 0 || height <= 0 ||
        width > 2048 || height > 2048 ||
        (size_t)width * (size_t)height > 4u * 1024u * 1024u) {
        SDL_Log("Rejected invalid or oversized sprite '%s'", path);
        SDL_free(file_data);
        return NULL;
    }

    pixel_count = (size_t)width * (size_t)height;
    if (pixel_count > *remaining_pixel_budget) {
        SDL_Log("Skipped sprite '%s': texture memory budget would be exceeded", path);
        SDL_free(file_data);
        return NULL;
    }

    pixels = stbi_load_from_memory(
        (const stbi_uc *)file_data,
        (int)file_size,
        &width,
        &height,
        &source_channels,
        STBI_rgb_alpha
    );

    SDL_free(file_data);

    if (pixels == NULL) {
        SDL_Log("Could not decode sprite '%s'", path);
        return NULL;
    }

    surface = SDL_CreateSurfaceFrom(
        width,
        height,
        SDL_PIXELFORMAT_RGBA32,
        pixels,
        width * 4
    );

    if (surface == NULL) {
        SDL_Log("Could not create surface for '%s': %s", path, SDL_GetError());
        stbi_image_free(pixels);
        return NULL;
    }

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
    stbi_image_free(pixels);

    if (texture == NULL) {
        SDL_Log("Could not create texture for '%s': %s", path, SDL_GetError());
        return NULL;
    }

    if (!SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST)) {
        SDL_Log("Could not set pixel-art scaling for '%s': %s", path, SDL_GetError());
    }

    *remaining_pixel_budget -= pixel_count;
    return texture;
}

bool sf_assets_init(SfAssets *assets, SDL_Renderer *renderer)
{
    int loaded_count = 0;
    size_t remaining_pixel_budget = SF_ASSET_PIXEL_BUDGET;
    int i;

    if (assets == NULL || renderer == NULL) {
        return false;
    }

    memset(assets, 0, sizeof(*assets));

    for (i = 0; i < SF_ASSET_COUNT; ++i) {
        assets->textures[i] = load_texture(
            renderer, asset_paths[i], &remaining_pixel_budget
        );

        if (assets->textures[i] != NULL) {
            ++loaded_count;
        }
    }

    SDL_Log(
        "Loaded %d of %d game sprites; texture budget remaining: %zu MiB",
        loaded_count,
        SF_ASSET_COUNT,
        remaining_pixel_budget / (1024u * 1024u)
    );

    return loaded_count == SF_ASSET_COUNT;
}

void sf_assets_destroy(SfAssets *assets)
{
    int i;

    if (assets == NULL) {
        return;
    }

    for (i = 0; i < SF_ASSET_COUNT; ++i) {
        if (assets->textures[i] != NULL) {
            SDL_DestroyTexture(assets->textures[i]);
            assets->textures[i] = NULL;
        }
    }
}

SDL_Texture *sf_assets_get(const SfAssets *assets, SfAssetId id)
{
    if (assets == NULL || id < 0 || id >= SF_ASSET_COUNT) {
        return NULL;
    }

    return assets->textures[id];
}
