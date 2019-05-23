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
#include "player.h"

#define EDITOR 1

int 	tics;
float 	originx, originy; // visible upperleft corner
float 	maxx, maxy; // visible lower right corner


// GLOBAL CONSTANTS

const uint8_t *key;
const SDL_Rect maprect = { 0, 0, MAP_W, MAP_H };
const SDL_Rect statusrect = { STATUS_X, STATUS_Y, STATUS_W, STATUS_H };
const SDL_Rect *full = NULL;


// GLOBAL VARIABLES

SDL_Texture *objtextures[NUMOBJECTS];
SDL_Texture	*tiletextures[NUMTILES];

//object_t	*player; //*freeobj;
//int			lastobj; // keep track of index of last object, for iterating etc.

gamestate_t	gamestate;

leveldata_t current; // the current level data OLD
tile_t		tilemap[MAPMAX][MAPMAX];

boolean		levelloaded = false;
int 		level;
boolean 	playdone = false;

const char *classnames[NUMOBJECTS] = { // for debug
	"OT_NOTHING","OT_PLAYER","OT_FIREBALL","OT_BAT"
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




//
// UpdateOrigin X / Y
// Adjust the origin for screen scrolling
//
void UpdateOriginX ()
{
	originx = player.obj->x - MAP_W/2;
	bound(originx, 0, current.width*TILESIZE-MAP_W);
	maxx = originx + MAP_W;
}

void UpdateOriginY ()
{
	originy = player.obj->y - MAP_H/2;
	bound(originy, 0, current.height*TILESIZE-MAP_H);
	maxy = originy + MAP_H;
}

void UpdateOrigin ()
{
	UpdateOriginX();
	UpdateOriginY();
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
	int 		x,y;
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
			
			tile = &tilemap[y][x];
			if (tile->type) {
				SDL_Rect dst = { drawx(x), drawy(y), TILESIZE, TILESIZE };
				//V_RenderTexture(&tiletextures[tile->tileclass], drawx(x), drawy(y));
				SDL_RenderCopy(renderer, tiletextures[tile->type], NULL, &dst);
			}
		}
	}
	
	// game objects
	obj = first;
	do
	{
		if (obj->type && OnScreen(obj))
			RenderObject(obj);
		obj = obj->next;
	} while (obj);
	
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
			
			tile = &tilemap[y][x];
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
	printintxy(14, 0, player.obj->health);
	DrawMeter(7*8, 0, player.obj->info->maxhp, player.obj->health);
	
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
	
	if (key[SDL_SCANCODE_P])
	{
		printf("Player x: %f, y: %f\n", player.obj->x, player.obj->y);
		printf("Player left:   %f\n", player.obj->left);
		printf("Player top:    %f\n", player.obj->top);
		printf("Player right:  %f\n", player.obj->right);
		printf("Player bottom: %f\n", player.obj->bottom);
	}
	
	if (key[SDL_SCANCODE_O])
	{
		object_t *obj = first;
		do
		{
			printf("=================\n");
			printf("type:     %d\n", obj->type);
			printf("x, y:    (%d, %d)\n", (int)obj->x, (int)obj->y);
			printf("src xy:  (%d, %d)\n", obj->srcrect.x, obj->srcrect.y);
			obj = obj->next;
		} while (obj);
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
		player.obj->dy = -player.obj->info->speed;
	if (key[SDL_SCANCODE_S])
		player.obj->dy = player.obj->info->speed;
	if (key[SDL_SCANCODE_A])
		player.obj->dx = -player.obj->info->speed;
	if (key[SDL_SCANCODE_D])
		player.obj->dx = player.obj->info->speed;
	
	// PLAYER SHOTS
	
	if (key[SDL_SCANCODE_UP] && !player.cooldown) {
		SpawnFireball(DI_NORTH);
	}
	if (key[SDL_SCANCODE_DOWN] && !player.cooldown) {
		SpawnFireball(DI_SOUTH);
	}
	if (key[SDL_SCANCODE_LEFT] && !player.cooldown) {
		SpawnFireball(DI_WEST);
	}
	if (key[SDL_SCANCODE_RIGHT] && !player.cooldown) {
		SpawnFireball(DI_EAST);
	}
}




void PlayLoop ()
{
	object_t *obj,*check;
	SDL_Event ev;
	
	if ( !player.obj )
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
		obj = first;
		do
		{
			ObjectThinker(obj);
			
			// check for collisions
			check = obj->next;
			while ( check )
			{
				if ( OnScreen(check) )
				{
					if (obj->right > check->left &&
						obj->left < check->right &&
						obj->top < check->bottom &&
						obj->bottom > check->top)
					{
						// overlapping, make contact
						if (obj->info->contact)
							obj->info->contact(obj,check);
						if (check->info->contact)
							check->info->contact(check,obj);
					}
					if ( !obj )
						break; // contact removed obj
				}
				check = check->next;
			}
			
			// set to active if on-screen
//			if (!obj->active && OnScreen(obj))
//				obj->active = yes;
//
//			else if (obj->active && !OnScreen(obj))
//				obj->active = no;
			
			obj = obj->next;
		} while (obj);
		
		Refresh();
		tics++;
				
		V_LimitFR();
		
	} while (gamestate == play);
}




void SetObjectPosition (object_t *obj, int x, int y)
{
	obj->x = x; //draw coords
	obj->y = y;
	obj->oldx = obj->x;
	obj->oldy = obj->y;
	
	obj->left = obj->x; // set hit box
	obj->top = obj->y;
	obj->right = obj->x + obj->info->width;
	obj->bottom = obj->y + obj->info->height;
	
	// in game, tile refers to where the obj center is
	obj->tilex = (obj->x + obj->info->width/2) / TILESIZE;
	obj->tiley = (obj->y + obj->info->height/2) / TILESIZE;
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
		
	// load object textures
	objtextures[OT_PLAYER] 	= V_LoadTexture("graphics/dave.png");
	objtextures[OT_BAT] 		= V_LoadTexture("graphics/bat.png");
	objtextures[OT_FIREBALL] 	= V_LoadTexture("graphics/fball.png");
	objtextures[OT_FBEXPLOSION] = V_LoadTexture("graphics/fbexplode.png");
	objtextures[OT_SNAKE]		= V_LoadTexture("graphics/snake.png");
	objtextures[OT_SMALLHEALTH] 	= V_LoadTexture("graphics/smallhealth.png");
	objtextures[OT_BIGHEALTH] = V_LoadTexture("graphics/bighealth.png");
	objtextures[OT_TORCH]		= V_LoadTexture("graphics/torch.png");

	// load tile textures
	tiletextures[TT_FLOOR]	= V_LoadTexture("graphics/graytile.png");
	tiletextures[TT_WALL] 	= V_LoadTexture("graphics/graywall.png");
	
	if (!LoadLevel(&current, filepath))
		NewLevel(filepath, 30, 30, 1);
	
	while (1)
	{
		
		switch (gamestate)
		{
			case title:
				break;
				
			case play:
				PlayLoop();
				break;
				
			case gameover:
				break;
				
			case editor:
				EditorLoop();
				break;
				
			default:
				break;
		}
	}
}
