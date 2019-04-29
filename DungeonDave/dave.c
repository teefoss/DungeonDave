//
//  main.c
//  DungeonDave
//
//  Created by Thomas Foster on 2/14/19.
//  Copyright © 2019 Thomas Foster. All rights reserved.
//

#include <stdio.h>
#include "dave.h"
#include "light.h"

#define EDITOR 1

int tics;
int originx, originy; // visible upperleft corner
int maxx, maxy; // visible lower right corner


// GLOBAL CONSTANTS

const uint8_t *key;
const SDL_Rect maprect = { 0, 0, MAP_W, MAP_H };
const SDL_Rect statusrect = { STATUS_X, STATUS_Y, STATUS_W, STATUS_H };


// GLOBAL VARIABLES

texture_t 	objtextures[NUMOBJECTS];
texture_t	tiletextures[NUMTILES];

object_t	*player, *freeobj;
int			lastobj; // keep track of index of last object, for iterating etc.

gamestate_t	gamestate;
leveldata_t current; // the current level data
boolean		levelloaded = false;
int 		level;
boolean 	playdone = false;

const char *classnames[NUMOBJECTS] = { // for debug
	"nothing","playerclass","fireballclass","batclass"
};



void Quit (char *error)
{
	V_ShutDown();
	if (error && *error)
	{
		puts(error);
		puts("\n");
		exit(1);
	}
	exit (0);
}


boolean Blink (int rate)
{
	return SDL_GetTicks() % rate*2 < rate;
}



//
// UpdateOrigin X / Y
// Adjust the origin for screen scrolling
//
void UpdateOriginX ()
{
	originx = player->x - MAP_W/2;
	bound(originx, 0, current.width*TILESIZE-MAP_W);
	maxx = originx + MAP_W;
}

void UpdateOriginY ()
{
	originy = player->y - MAP_H/2;
	bound(originy, 0, current.height*TILESIZE-MAP_H);
	maxy = originy + MAP_H;
}




void DrawMeter (int x, int y, int maxval, int curval)
{
	int width = 48;
	
	SDL_Rect bgr = { x, y, 48, 8 };
	SDL_Rect meter = { x, y, width*curval/maxval, 8 };
	
	V_SetColor(GRAY);
	SDL_RenderFillRect(renderer, &bgr);
	V_SetColor(GREEN);
	SDL_RenderFillRect(renderer, &meter);
}






void Refresh ()
{
	int 		i,x,y;
	tile_t 		*tile;
	object_t 	*obj;

	
	// MAP AREA
	
	SDL_RenderSetViewport(renderer, &maprect);

	V_Clear(BLACK);
	
	// tile map
	for (y=0 ; y<current.height ; y++) {
		for (x=0 ; x<current.width ; x++)
		{
			if (drawx(x) > maxx ||
				drawy(y) > maxy ||
				drawx(x) < -TILESIZE ||
				drawy(y) < -TILESIZE)
			{
				continue; // tile is offscreen
			}
			
			tile = &current.tilemap[y][x];
			if (tile->tileclass) {
				V_RenderTexture(&tiletextures[tile->tileclass], drawx(x), drawy(y));
			}
		}
	}
	
	// game objects
	for (i=0,obj=current.objects ; i<=lastobj ; i++,obj++)
	{
		if (obj->objclass && OnScreen(obj))
			Render(obj);
	}
	
	// render the light map
	for (y=0 ; y<current.height ; y++) {
		for (x=0 ; x<current.width ; x++)
		{
			if (drawx(x) > maxx ||
				drawy(y) > maxy ||
				drawx(x) < -TILESIZE ||
				drawy(y) < -TILESIZE)
			{
				continue; // tile is offscreen
			}
			
			tile = &current.tilemap[y][x];
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, tile->light);
			SDL_Rect r = { drawx(x), drawy(y), TILESIZE, TILESIZE };
			SDL_RenderFillRect(renderer, &r);
			tile->light = DEFAULT_BRIGHTNESS; // reset
		}
	}
	
	// STATUS
	
	SDL_RenderSetViewport(renderer, &statusrect);
	
	cls();
	textcolor(BRIGHTWHITE);
	printxy(0, 0, "Health ");
	printintxy(14, 0, player->hp);
	DrawMeter(7*8, 0, player->def->maxhp, player->hp);
	
	V_Draw();
}



void ReadInput (SDL_Keycode keycode)
{
	switch (keycode) {
		case SDLK_ESCAPE:
			Quit(NULL);
			break;
		default:
			break;
	}
}



void DebugKeys ()
{
	SDL_PumpEvents();
	
	if (key[SDL_SCANCODE_P]) {
		printf("Player x: %d, y: %d\n", player->x, player->y);
		printf("Player left:   %d\n", player->left);
		printf("Player top:    %d\n", player->top);
		printf("Player right:  %d\n", player->right);
		printf("Player bottom: %d\n", player->bottom);
	}
}





void CheckKeys ()
{
	SDL_PumpEvents();
	
	// hold down e and return to start editor
	if (key[SDL_SCANCODE_E] && key[SDL_SCANCODE_RETURN]) {
		gamestate = editor;
		return;
	}
	
	// PLAYER MOVEMENT

	if (key[SDL_SCANCODE_W])
		player->ymove = -player->def->speed;
	if (key[SDL_SCANCODE_S])
		player->ymove = player->def->speed;
	if (key[SDL_SCANCODE_A])
		player->xmove = -player->def->speed;
	if (key[SDL_SCANCODE_D])
		player->xmove = player->def->speed;
	
	// PLAYER SHOTS
	
	if (key[SDL_SCANCODE_UP] && !player->tics1) {
		SpawnFireball(north);
	}
	if (key[SDL_SCANCODE_DOWN] && !player->tics1) {
		SpawnFireball(south);
	}
	if (key[SDL_SCANCODE_LEFT] && !player->tics1) {
		SpawnFireball(west);
	}
	if (key[SDL_SCANCODE_RIGHT] && !player->tics1) {
		SpawnFireball(east);
	}
}




void PlayLoop ()
{
	object_t *obj,*check;
	SDL_Event ev;
	int i,j;
	
	if (!player)
		Quit("Error! Player is NULL. Level missing player start?");
	
	do
	{
		V_StartFrame();
		
		while (SDL_PollEvent(&ev))
		{
			if (ev.type == SDL_QUIT)
				Quit(NULL);
			else if (ev.type == SDL_KEYDOWN)
				ReadInput(ev.key.keysym.sym);
		}
		
		CheckKeys();
		DebugKeys();
		
		// process all thinkers and handle any collisions
		for (i=0,obj=current.objects ; i<=lastobj ; i++,obj++)
		{
			if (!obj->objclass)
				continue; // removed objectd
		
			// set to active if on-screen
			if (!obj->active && OnScreen(obj))
			{
//				obj->needstoreact = true;
				obj->active = yes;
			}
			
			if (obj->def->think)
				obj->def->think(obj);
			
			// go through other objects
			for (j=i+1,check=&current.objects[j] ; j<=lastobj ; j++,check++)
			{
				if (!check->objclass)
					continue;
				
				if (check->active // if overlap
					&& obj->right > check->left
					&& obj->left < check->right
					&& obj->top < check->bottom
					&& obj->bottom > check->top)
				{
					if (obj->def->contact)
						obj->def->contact(obj,check);
					if (check->def->contact)
						check->def->contact(check,obj);
				}
				if (!obj->objclass)
					break; // contact removed object
			}
			
			if (obj->active == removable)
				RemoveObject(obj);
			if (obj->active && !OnScreen(obj))
				obj->active = no;
		}
		
		// react to whatever happened
		for (i=0,obj=current.objects ; i<lastobj ; i++,obj++)
		{
			if (!obj->objclass)
				continue;
//			if (obj->needstoreact && obj->def->react)
//				obj->def->react(obj);
//			if (obj->active == removable)
//				RemoveObject(obj);
		}
		
		Refresh();
		tics++;
				
		V_LimitFR(60);
	} while (gamestate == play);
}




//
// InitObject
// fully initialize an object after it has been loaded from file
// When loaded, it will have objclass, tilex, and tiley set
//
void InitObject (object_t *obj, objclass_t cl)
{
	memset(obj, 0, sizeof(object_t));
	obj->objclass = cl;
	obj->def = &objdefs[cl]; // get object's def
	obj->hp = obj->def->maxhp;
	obj->active = true; // TODO (fix later) set everything to active for now
}


// assume object has already been inited with InitObject
void SetObjectPosition (object_t *obj, int x, int y)
{
	obj->x = x; //draw coords
	obj->y = y;
	obj->oldx = obj->x;
	obj->oldy = obj->y;
	
	obj->left = obj->x; // init hit box
	obj->top = obj->y;
	obj->right = obj->x + obj->def->w;
	obj->bottom = obj->y + obj->def->h;
	
	obj->tilex = (obj->x + obj->def->w/2) / TILESIZE; // in game, tile refers to
	obj->tiley = (obj->y + obj->def->h/2) / TILESIZE; // where the obj center is
}



#pragma mark -

int main ()
{
	V_InitVideo(WINDOW_W, WINDOW_H, 3, "Dungeon Dave");
	key = SDL_GetKeyboardState(NULL);
	
	if (EDITOR)
		gamestate = editor;
	else
		gamestate = play;
	
	InitObjDefs();
	InitTileDefs();
	
	// load object textures
	objtextures[playerclass] 	= V_LoadTexture("graphics/dave.png");
	objtextures[batclass] 		= V_LoadTexture("graphics/bat.png");
	objtextures[fireballclass] 	= V_LoadTexture("graphics/fball.png");
	objtextures[fbexplodeclass] = V_LoadTexture("graphics/fbexplode.png");
	objtextures[snakeclass]		= V_LoadTexture("graphics/snake.png");
	objtextures[smhealthclass] 	= V_LoadTexture("graphics/smallhealth.png");
	objtextures[bighealthclass] = V_LoadTexture("graphics/bighealth.png");
	objtextures[torchclass]		= V_LoadTexture("graphics/torch.png");

	// load tile textures
	tiletextures[floorclass]	= V_LoadTexture("graphics/graytile.png");
	tiletextures[wallclass] 	= V_LoadTexture("graphics/graywall.png");
	
	while (1)
	{
		switch (gamestate)
		{
			case title:
				break;
				
			case play:
				LoadLevel(&current, filepath);
				InitLighting();
				PlayLoop();
				break;
				
			case gameover:
				break;
				
			case editor:
				if (!LoadLevel(&current, filepath))
					NewLevel(filepath, 30, 30, 1);
				EditorLoop();
				break;
				
			default:
				break;
		}
	}
}
