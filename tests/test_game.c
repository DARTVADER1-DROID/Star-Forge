#include "../src/game.h"
#include <assert.h>
#include <stdio.h>
int main(void){SfGame g;SfInput idle={false,false,false,false,false};sf_game_init(&g);assert(g.player.alive);assert(g.inventory[SF_ITEM_METAL]==12);assert(g.npcs[0].role==SF_ROLE_CARGO);assert(g.npcs[1].role==SF_ROLE_PIRATE);g.selected_item=SF_ITEM_METAL;g.selected_quantity=5;assert(sf_game_jettison_selected(&g));assert(g.inventory[SF_ITEM_METAL]==7);assert(sf_game_collect_nearest(&g));assert(g.inventory[SF_ITEM_METAL]==12);g.player.x=g.station_x;g.player.y=g.station_y;assert(sf_game_sell_selected(&g));assert(g.inventory[SF_ITEM_METAL]==0);assert(g.credits==216);sf_game_update(&g,&idle,1.0f/60.0f);puts("Star-Forge simulation tests passed.");return 0;}
