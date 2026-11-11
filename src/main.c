#include <SDL3/SDL.h>
#include "game.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#define WW 960
#define WH 540
#define LW 320
#define LH 180
#define STEP (1.0/60.0)
static void color(SDL_Renderer*r,Uint8 a,Uint8 b,Uint8 c){SDL_SetRenderDrawColor(r,a,b,c,255);}
static void ship_draw(SDL_Renderer*r,float x,float y,float a,int cx,int cy,Uint8 cr,Uint8 cg,Uint8 cb){
 float px[3]={15,-10,-7},py[3]={0,-9,0},sx[3],sy[3],co=cosf(a),si=sinf(a);
 for(int i=0;i<3;i++){sx[i]=x+px[i]*co-py[i]*si-cx;sy[i]=y+px[i]*si+py[i]*co-cy;}
 color(r,cr,cg,cb);for(int i=0;i<3;i++)SDL_RenderLine(r,sx[i],sy[i],sx[(i+1)%3],sy[(i+1)%3]);
}
static void draw_asteroids(SDL_Renderer*r,int cx,int cy){
 static const int ax[8]={520,680,860,1040,1280,1510,1740,1980};
 static const int ay[8]={250,620,430,890,510,1120,760,1380};
 static const int ar[8]={22,30,18,34,25,28,20,32};
 for(int i=0;i<8;i++){int sx=ax[i]-cx,sy=ay[i]-cy;if(sx+ar[i]<0||sx-ar[i]>=LW||sy+ar[i]<0||sy-ar[i]>=LH)continue;color(r,75,85,105);for(int yy=-ar[i];yy<=ar[i];yy++){int half=(int)sqrt((double)(ar[i]*ar[i]-yy*yy));SDL_RenderLine(r,(float)(sx-half),(float)(sy+yy),(float)(sx+half),(float)(sy+yy));}color(r,105,115,135);SDL_RenderPoint(r,(float)(sx-4),(float)(sy-3));}
}
static void draw(SDL_Renderer*r,const SfGame*g,int cx,int cy){
 SDL_FRect q;char t[160];color(r,5,8,19);SDL_RenderClear(r);
 for(int i=0;i<90;i++){int x=(i*173+71)%2400-cx,y=(i*97+211)%2400-cy;if(x>=0&&x<LW&&y>=0&&y<LH){color(r,(Uint8)(70+i%4*30),(Uint8)(80+i%3*25),150);SDL_RenderPoint(r,(float)x,(float)y);}}
 draw_asteroids(r,cx,cy);\n color(r,55,110,150);q=(SDL_FRect){g->station_x-cx-9,g->station_y-cy-9,18,18};SDL_RenderRect(r,&q);
 for(int i=0;i<SF_MAX_DROPS;i++){const SfCargoDrop*d=&g->drops[i];if(!d->active)continue;switch(d->item){case SF_ITEM_METAL:color(r,160,170,180);break;case SF_ITEM_FUEL:color(r,80,190,230);break;case SF_ITEM_FOOD:color(r,110,190,90);break;case SF_ITEM_MEDICINE:color(r,220,220,240);break;default:color(r,245,190,65);break;}q=(SDL_FRect){d->x-cx-3,d->y-cy-3,6,6};SDL_RenderFillRect(r,&q);}
 for(int i=0;i<SF_MAX_PROJECTILES;i++){const SfProjectile*p=&g->projectiles[i];if(p->active){color(r,p->from_player?100:255,p->from_player?220:90,80);SDL_RenderPoint(r,p->x-cx,p->y-cy);}}
 for(int i=0;i<SF_MAX_NPCS;i++){const SfShip*s=&g->npcs[i];if(!s->alive)continue;ship_draw(r,s->x,s->y,s->angle,cx,cy,s->role==SF_ROLE_PIRATE?235:90,s->role==SF_ROLE_PIRATE?80:190,s->role==SF_ROLE_PIRATE?85:220);color(r,180,60,70);SDL_RenderLine(r,s->x-cx-10,s->y-cy-13,s->x-cx-10+20*s->hp/s->max_hp,s->y-cy-13);}
 ship_draw(r,g->player.x,g->player.y,g->player.angle,cx,cy,100,235,150);color(r,220,230,245);
 snprintf(t,sizeof(t),"STAR-FORGE v0.1  HP %.0f  CREDITS %d",g->player.hp,g->credits);SDL_RenderDebugText(r,5,5,t);
 snprintf(t,sizeof(t),"METAL %d FUEL %d FOOD %d MED %d WEAPON %d",g->inventory[0],g->inventory[1],g->inventory[2],g->inventory[3],g->inventory[4]);SDL_RenderDebugText(r,5,17,t);
 snprintf(t,sizeof(t),"SELECT %s x%d | NPC SALVAGE %s",sf_item_name(g->selected_item),g->selected_quantity,g->npc_salvage_enabled?"ON":"OFF");SDL_RenderDebugText(r,5,29,t);
 SDL_RenderDebugText(r,5,153,"W/S thrust A/D turn SPACE fire E loot J jettison");SDL_RenderDebugText(r,5,165,"1-5 item +/- qty T sell near station F3 NPC salvage I cargo");
}
static void hover(SDL_Renderer*r,const SfGame*g,float mx,float my,int cx,int cy){
 for(int i=0;i<SF_MAX_DROPS;i++){const SfCargoDrop*d=&g->drops[i];char t[90];if(d->active&&sf_distance_squared(mx+cx,my+cy,d->x,d->y)<18*18){snprintf(t,sizeof(t),"%s x%d | E: collect",sf_item_name(d->item),d->quantity);color(r,255,235,150);SDL_RenderDebugText(r,5,43,t);return;}}
}
int main(int argc,char**argv){
 (void)argc;(void)argv;SDL_Window*w=NULL;SDL_Renderer*r=NULL;SfGame g;bool run=true,inventory=false;Uint64 prev,frequency;double acc=0;
 if(!SDL_Init(SDL_INIT_VIDEO|SDL_INIT_GAMEPAD)){SDL_Log("SDL init: %s",SDL_GetError());return 1;}
 w=SDL_CreateWindow("Star-Forge v0.1",WW,WH,SDL_WINDOW_RESIZABLE);if(!w){SDL_Quit();return 1;}
 r=SDL_CreateRenderer(w,NULL);if(!r){SDL_DestroyWindow(w);SDL_Quit();return 1;}
 (void)SDL_SetRenderLogicalPresentation(r,LW,LH,SDL_LOGICAL_PRESENTATION_LETTERBOX);sf_game_init(&g);frequency=SDL_GetPerformanceFrequency();prev=SDL_GetPerformanceCounter();
 while(run){SDL_Event e;SfInput in={false,false,false,false,false};Uint64 now;double frame;const bool*keys;float mx=0.0f,my=0.0f,lx=0.0f,ly=0.0f;int winw=WW,winh=WH;int cx=(int)(g.player.x-LW*.5f),cy=(int)(g.player.y-LH*.5f);
  while(SDL_PollEvent(&e)){if(e.type==SDL_EVENT_QUIT)run=false;if(e.type==SDL_EVENT_KEY_DOWN&&!e.key.repeat)switch(e.key.scancode){
   case SDL_SCANCODE_ESCAPE:run=false;break;case SDL_SCANCODE_1:g.selected_item=SF_ITEM_METAL;break;case SDL_SCANCODE_2:g.selected_item=SF_ITEM_FUEL;break;case SDL_SCANCODE_3:g.selected_item=SF_ITEM_FOOD;break;case SDL_SCANCODE_4:g.selected_item=SF_ITEM_MEDICINE;break;case SDL_SCANCODE_5:g.selected_item=SF_ITEM_WEAPON;break;
   case SDL_SCANCODE_J:(void)sf_game_jettison_selected(&g);break;case SDL_SCANCODE_E:(void)sf_game_collect_nearest(&g);break;case SDL_SCANCODE_T:(void)sf_game_sell_selected(&g);break;case SDL_SCANCODE_F3:g.npc_salvage_enabled=!g.npc_salvage_enabled;break;case SDL_SCANCODE_EQUALS:case SDL_SCANCODE_KP_PLUS:g.selected_quantity++;break;case SDL_SCANCODE_MINUS:case SDL_SCANCODE_KP_MINUS:if(g.selected_quantity>1)g.selected_quantity--;break;case SDL_SCANCODE_I:inventory=!inventory;break;default:break;}}
  keys=SDL_GetKeyboardState(NULL);in.thrust=keys[SDL_SCANCODE_W]||keys[SDL_SCANCODE_UP];in.reverse=keys[SDL_SCANCODE_S]||keys[SDL_SCANCODE_DOWN];in.turn_left=keys[SDL_SCANCODE_A]||keys[SDL_SCANCODE_LEFT];in.turn_right=keys[SDL_SCANCODE_D]||keys[SDL_SCANCODE_RIGHT];in.fire=keys[SDL_SCANCODE_SPACE];
  now=SDL_GetPerformanceCounter();frame=(double)(now-prev)/(double)frequency;prev=now;if(frame>.25)frame=.25;acc+=frame;while(acc>=STEP){sf_game_update(&g,&in,(float)STEP);acc-=STEP;}
  SDL_GetMouseState(&mx,&my);SDL_GetWindowSize(w,&winw,&winh);if(winw>0&&winh>0){lx=(float)mx*LW/winw;ly=(float)my*LH/winh;}
  draw(r,&g,cx,cy);hover(r,&g,lx,ly,cx,cy);
  if(inventory){SDL_FRect p={62,52,196,76};char text[96];color(r,20,28,48);SDL_RenderFillRect(r,&p);color(r,100,180,220);SDL_RenderRect(r,&p);color(r,240,240,245);SDL_RenderDebugText(r,72,61,"CARGO HOLD (I to close)");snprintf(text,sizeof(text),"Selected: %s   Quantity: %d",sf_item_name(g.selected_item),g.selected_quantity);SDL_RenderDebugText(r,72,76,text);SDL_RenderDebugText(r,72,91,"J jettison  +/- quantity  T sell at station");SDL_RenderDebugText(r,72,106,"E collect nearby salvage");}
  SDL_RenderPresent(r);SDL_Delay(1);
 }
 SDL_DestroyRenderer(r);SDL_DestroyWindow(w);SDL_Quit();return 0;
}
