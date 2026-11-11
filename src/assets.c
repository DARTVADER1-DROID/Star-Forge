#include "assets.h"
#include <limits.h>
#include <string.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
static const char *const paths[SF_ASSET_COUNT]={
"assets/ships/ship_player_runner.png","assets/ships/ship_cargo_hauler.png","assets/ships/ship_pirate_interceptor.png",
"assets/stations/station_ring_hub.png","assets/planets/planet_terran_64.png","assets/planets/planet_gas_96.png",
"assets/planets/planet_moon_32.png","assets/asteroids/asteroid_large_01.png","assets/salvage/salvage_metal.png",
"assets/salvage/salvage_fuel.png","assets/salvage/salvage_food.png","assets/salvage/salvage_medicine.png",
"assets/salvage/salvage_weapon.png",
"assets/ui/ui_btn_fire_up.png",
"assets/ui/ui_btn_fire_down.png",
"assets/ui/ui_btn_boost_up.png",
"assets/ui/ui_btn_boost_down.png",
"assets/ui/ui_btn_loot_up.png",
"assets/ui/ui_btn_loot_down.png",
"assets/ui/ui_joystick_base.png",
"assets/ui/ui_joystick_knob.png"};
static SDL_Texture *load_one(SDL_Renderer *r,const char *path){
size_t size=0;void *file=SDL_LoadFile(path,&size);int w=0,h=0,n=0;unsigned char *pixels;SDL_Surface *surface;SDL_Texture *texture;
if(!file||size>(size_t)INT_MAX){SDL_free(file);SDL_Log("Missing sprite %s",path);return NULL;}
pixels=stbi_load_from_memory((const stbi_uc*)file,(int)size,&w,&h,&n,STBI_rgb_alpha);SDL_free(file);
if(!pixels||w<=0||h<=0){SDL_Log("Invalid sprite %s",path);stbi_image_free(pixels);return NULL;}
surface=SDL_CreateSurfaceFrom(w,h,SDL_PIXELFORMAT_RGBA32,pixels,w*4);
if(!surface){stbi_image_free(pixels);return NULL;}
texture=SDL_CreateTextureFromSurface(r,surface);SDL_DestroySurface(surface);stbi_image_free(pixels);
if (texture == NULL) {
        SDL_Log("Could not create texture for %s: %s", path, SDL_GetError());
        return NULL;
    }

    if (!SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST)) {
        SDL_Log("Could not set pixel-art scaling for %s: %s", path, SDL_GetError());
    }

    return texture;}
bool sf_assets_init(SfAssets *assets, SDL_Renderer *renderer)
{
    int loaded = 0;
    int i;

    if (assets == NULL || renderer == NULL) {
        return false;
    }

    memset(assets, 0, sizeof(*assets));

    for (i = 0; i < SF_ASSET_COUNT; ++i) {
        assets->textures[i] = load_one(renderer, paths[i]);
        if (assets->textures[i] != NULL) {
            ++loaded;
        }
    }

    SDL_Log("Loaded %d/%d sprites", loaded, SF_ASSET_COUNT);
    return loaded == SF_ASSET_COUNT;
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
