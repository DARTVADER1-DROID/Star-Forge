#include "game.h"
#include <math.h>
#include <string.h>
#define PI 3.14159265358979323846f
#define WORLD 2400.0f
static float clampf(float v,float lo,float hi){return v<lo?lo:(v>hi?hi:v);}
float sf_distance_squared(float ax,float ay,float bx,float by){float x=ax-bx,y=ay-by;return x*x+y*y;}
static unsigned int rnd(SfGame*g){unsigned int x=g->rng;x^=x<<13;x^=x>>17;x^=x<<5;g->rng=x;return x;}
static float rangef(SfGame*g,float a,float b){return a+(b-a)*(float)(rnd(g)&0xffffffu)/16777215.0f;}
const char *sf_item_name(int i){static const char*n[SF_ITEM_COUNT]={"Metal","Fuel","Food","Medicine","Weapon"};return i>=0&&i<SF_ITEM_COUNT?n[i]:"Unknown";}
int sf_item_price(int i){static const int p[SF_ITEM_COUNT]={8,14,5,22,75};return i>=0&&i<SF_ITEM_COUNT?p[i]:0;}
static void spawn_drop(SfGame*g,float x,float y,int item,int qty){
 if(qty<=0)return;
 for(int i=0;i<SF_MAX_DROPS;i++)if(!g->drops[i].active){SfCargoDrop*d=&g->drops[i];d->active=true;d->x=x+rangef(g,-12,12);d->y=y+rangef(g,-12,12);d->vx=rangef(g,-18,18);d->vy=rangef(g,-18,18);d->item=item;d->quantity=qty;return;}
}
static void destroy_npc(SfGame*g,SfShip*s){
 for(int i=0;i<SF_ITEM_COUNT;i++){if(s->cargo[i]>0){spawn_drop(g,s->x,s->y,i,s->cargo[i]);s->cargo[i]=0;}}
 if(rnd(g)%100u<35u)spawn_drop(g,s->x,s->y,SF_ITEM_WEAPON,1);
 s->alive=false;g->destroyed_npcs++;
}
static void spawn_projectile(SfGame*g,float x,float y,float a,bool player,float speed){
 for(int i=0;i<SF_MAX_PROJECTILES;i++)if(!g->projectiles[i].active){SfProjectile*p=&g->projectiles[i];p->active=true;p->x=x;p->y=y;p->vx=cosf(a)*speed;p->vy=sinf(a)*speed;p->life=1.8f;p->from_player=player;return;}
}
void sf_game_fire_player(SfGame*g){SfShip*s=&g->player;if(s->fire_cooldown>0)return;spawn_projectile(g,s->x+cosf(s->angle)*18,s->y+sinf(s->angle)*18,s->angle,true,360);s->fire_cooldown=.22f;}
void sf_game_init(SfGame*g){
 memset(g,0,sizeof(*g));g->rng=0x51A7F04Du;g->player.x=300;g->player.y=300;g->player.hp=g->player.max_hp=100;g->player.alive=true;
 g->inventory[SF_ITEM_METAL]=12;g->inventory[SF_ITEM_FUEL]=18;g->selected_item=SF_ITEM_METAL;g->selected_quantity=5;g->credits=120;g->npc_salvage_enabled=true;g->station_x=220;g->station_y=180;
 SfShip*c=&g->npcs[0];c->alive=true;c->role=SF_ROLE_CARGO;c->x=720;c->y=360;c->angle=PI;c->hp=c->max_hp=65;c->cargo[SF_ITEM_FUEL]=60;c->cargo[SF_ITEM_FOOD]=24;c->cargo[SF_ITEM_METAL]=18;
 SfShip*p=&g->npcs[1];p->alive=true;p->role=SF_ROLE_PIRATE;p->x=910;p->y=340;p->angle=PI;p->hp=p->max_hp=85;p->cargo[SF_ITEM_FUEL]=8;
}
static void update_player(SfGame*g,const SfInput*in,float dt){
 SfShip*s=&g->player;if(in->turn_left)s->angle-=2.8f*dt;if(in->turn_right)s->angle+=2.8f*dt;
 if(s->angle>PI)s->angle-=2*PI;if(s->angle< -PI)s->angle+=2*PI;
 if(in->thrust){s->vx+=cosf(s->angle)*190*dt;s->vy+=sinf(s->angle)*190*dt;}
 if(in->reverse){s->vx-=cosf(s->angle)*123.5f*dt;s->vy-=sinf(s->angle)*123.5f*dt;}
 s->vx=clampf(s->vx*powf(.985f,dt*60),-190,190);s->vy=clampf(s->vy*powf(.985f,dt*60),-190,190);
 s->x=clampf(s->x+s->vx*dt,24,WORLD-24);s->y=clampf(s->y+s->vy*dt,24,WORLD-24);
 if(s->fire_cooldown>0)s->fire_cooldown-=dt;if(in->fire)sf_game_fire_player(g);
}
static void update_npcs(SfGame*g,float dt){
 for(int i=0;i<SF_MAX_NPCS;i++){SfShip*s=&g->npcs[i];if(!s->alive)continue;
  if(s->role==SF_ROLE_PIRATE){SfShip*t=g->npcs[0].alive?&g->npcs[0]:&g->player;float dx=t->x-s->x,dy=t->y-s->y,d=sqrtf(dx*dx+dy*dy),want=atan2f(dy,dx),delta=want-s->angle;
   while(delta>PI)delta-=2*PI;while(delta< -PI)delta+=2*PI;s->angle+=clampf(delta,-1.7f*dt,1.7f*dt);
   if(d>170){s->vx+=cosf(s->angle)*85*dt;s->vy+=sinf(s->angle)*85*dt;}else if(d<100){s->vx-=cosf(s->angle)*45*dt;s->vy-=sinf(s->angle)*45*dt;}
   if(d<330&&s->fire_cooldown<=0){spawn_projectile(g,s->x+cosf(s->angle)*17,s->y+sinf(s->angle)*17,s->angle,false,230);s->fire_cooldown=.85f;}
  }else{float dx=g->station_x-s->x,dy=g->station_y-s->y,want=atan2f(dy,dx),delta=want-s->angle;while(delta>PI)delta-=2*PI;while(delta< -PI)delta+=2*PI;s->angle+=clampf(delta,-.8f*dt,.8f*dt);s->vx+=cosf(s->angle)*28*dt;s->vy+=sinf(s->angle)*28*dt;}
  s->vx=clampf(s->vx*powf(.992f,dt*60),-105,105);s->vy=clampf(s->vy*powf(.992f,dt*60),-105,105);s->x=clampf(s->x+s->vx*dt,24,WORLD-24);s->y=clampf(s->y+s->vy*dt,24,WORLD-24);if(s->fire_cooldown>0)s->fire_cooldown-=dt;
 }
}
static void update_projectiles(SfGame*g,float dt){
 for(int i=0;i<SF_MAX_PROJECTILES;i++){SfProjectile*p=&g->projectiles[i];if(!p->active)continue;p->x+=p->vx*dt;p->y+=p->vy*dt;p->life-=dt;if(p->life<=0){p->active=false;continue;}
  if(p->from_player){for(int j=0;j<SF_MAX_NPCS;j++){SfShip*s=&g->npcs[j];if(s->alive&&sf_distance_squared(p->x,p->y,s->x,s->y)<22*22){s->hp-=24;p->active=false;if(s->hp<=0)destroy_npc(g,s);break;}}}
  else if(sf_distance_squared(p->x,p->y,g->player.x,g->player.y)<13*13){g->player.hp-=8;p->active=false;if(g->player.hp<0)g->player.hp=0;}
 }
}
static void update_drops(SfGame*g,float dt){
 for(int i=0;i<SF_MAX_DROPS;i++){SfCargoDrop*d=&g->drops[i];if(!d->active)continue;d->x+=d->vx*dt;d->y+=d->vy*dt;d->vx*=powf(.94f,dt*60);d->vy*=powf(.94f,dt*60);
  if(g->npc_salvage_enabled)for(int j=0;j<SF_MAX_NPCS;j++){SfShip*n=&g->npcs[j];int total=0;if(!n->alive)continue;for(int k=0;k<SF_ITEM_COUNT;k++)total+=n->cargo[k];if(total<100&&sf_distance_squared(d->x,d->y,n->x,n->y)<26*26){n->cargo[d->item]+=d->quantity;d->active=false;break;}}
 }
}
void sf_game_update(SfGame*g,const SfInput*in,float dt){if(dt<=0)return;if(dt>.05f)dt=.05f;if(g->player.hp>0)update_player(g,in,dt);update_npcs(g,dt);update_projectiles(g,dt);update_drops(g,dt);}
bool sf_game_collect_nearest(SfGame*g){
 int best=-1;float best_d=38*38;for(int i=0;i<SF_MAX_DROPS;i++){SfCargoDrop*d=&g->drops[i];int total=0;if(!d->active)continue;for(int k=0;k<SF_ITEM_COUNT;k++)total+=g->inventory[k];if(total+d->quantity>100)continue;float dist=sf_distance_squared(g->player.x,g->player.y,d->x,d->y);if(dist<=best_d){best=i;best_d=dist;}}
 if(best<0)return false;SfCargoDrop*d=&g->drops[best];g->inventory[d->item]+=d->quantity;d->active=false;return true;
}
bool sf_game_jettison_selected(SfGame*g){int i=g->selected_item,q=g->selected_quantity;if(i<0||i>=SF_ITEM_COUNT||i==SF_ITEM_WEAPON||q<=0||q>g->inventory[i])return false;g->inventory[i]-=q;spawn_drop(g,g->player.x,g->player.y,i,q);return true;}
bool sf_game_sell_selected(SfGame*g){int i=g->selected_item;if(i<0||i>=SF_ITEM_COUNT||sf_distance_squared(g->player.x,g->player.y,g->station_x,g->station_y)>90*90||g->inventory[i]<=0)return false;g->credits+=g->inventory[i]*sf_item_price(i);g->inventory[i]=0;return true;}
