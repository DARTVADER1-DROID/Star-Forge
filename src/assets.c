#include "assets.h"
#include <limits.h>
#include <string.h>
#define STB_IMAGE_IMPLEMENTATION
#include ""stb_image.h""
static const char *const paths[SF_ASSET_COUNT]={
"assets/ships/ship_player_runner.png","assets/ships/ship_cargo_hauler.png","assets/ships/ship_pirate_interceptor.png",
"assets/stations/station_ring_hub.png","assets/planets/planet_terran_64.png","assets/planets/planet_gas_96.png",
"assets/planets/planet_moon_32.png","assets/asteroids/asteroid_large_01.png","assets/salvage/salvage_metal.png",
"assets/salvage/salvage_fuel.png","assets/salvage/salvage_food.png","assets/salvage/salvage_medicine.png",
"assets/salvage/salvage_weapon.png",\n"assets/ui/ui_btn_fire_up.png",\n"assets/ui/ui_btn_fire_down.png",\n"assets/ui/ui_btn_boost_up.png",\n"assets/ui/ui_btn_boost_down.png",\n"assets/ui/ui_btn_loot_up.png",\n"assets/ui/ui_btn_loot_down.png",\n"assets/ui/ui_joystick_base.png",\n"assets/ui/ui_joystick_knob.png"};
static SDL_Texture *load_one(SDL_Renderer *r,const char *path){
size_t size=0;void *file=SDL_LoadFile(path,&size);int w=0,h=0,n=0;unsigned char *pixels;SDL_Surface *surface;SDL_Texture *texture;
if(!file||size>(size_t)INT_MAX){SDL_free(file);SDL_Log("Missing sprite %s",path);return NULL;}
pixels=stbi_load_from_memory((const stbi_uc*)file,(int)size,&w,&h,&n,STBI_rgb_alpha);SDL_free(file);
if(!pixels||w<=0||h<=0){SDL_Log("Invalid sprite %s",path);stbi_image_free(pixels);return NULL;}
surface=SDL_CreateSurfaceFrom(w,h,SDL_PIXELFORMAT_RGBA32,pixels,w*4);
if(!surface){stbi_image_free(pixels);return NULL;}
texture=SDL_CreateTextureFromSurface(r,surface);SDL_DestroySurface(surface);stbi_image_free(pixels);
if(texture)SDL_SetTextureScaleMode(texture,SDL_SCALEMODE_NEAREST);return texture;}
bool sf_assets_init(SfAssets *a,SDL_Renderer *r){int count=0;memset(a,0,sizeof(*a));for(int i=0;i<SF_ASSET_COUNT;i++){a->textures[i]=load_one(r,paths[i]);if(a->textures[i])count++;}SDL_Log("Loaded %d/%d sprites",count,SF_ASSET_COUNT);return count>0;}
void sf_assets_destroy(SfAssets *a){for(int i=0;i<SF_ASSET_COUNT;i++){if(a->textures[i])SDL_DestroyTexture(a->textures[i]);a->textures[i]=NULL;}}
SDL_Texture *sf_assets_get(const SfAssets *a,SfAssetId id){if(!a||id<0||id>=SF_ASSET_COUNT)return NULL;return a->textures[id];}
