#ifndef STAR_FORGE_ASSETS_H
#define STAR_FORGE_ASSETS_H
#include <SDL3/SDL.h>
#include <stdbool.h>
typedef enum { SF_ASSET_PLAYER,SF_ASSET_CARGO,SF_ASSET_PIRATE,SF_ASSET_STATION,SF_ASSET_TERRAN,SF_ASSET_GAS,SF_ASSET_MOON,SF_ASSET_ASTEROID,SF_ASSET_METAL,SF_ASSET_FUEL,SF_ASSET_FOOD,SF_ASSET_MEDICINE,SF_ASSET_WEAPON,SF_ASSET_COUNT } SfAssetId;
typedef struct { SDL_Texture *textures[SF_ASSET_COUNT]; } SfAssets;
bool sf_assets_init(SfAssets *,SDL_Renderer *);
void sf_assets_destroy(SfAssets *);
SDL_Texture *sf_assets_get(const SfAssets *,SfAssetId);
#endif
