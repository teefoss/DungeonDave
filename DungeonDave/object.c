//
//  object.c
//  DungeonDave
//
//  Created by Thomas Foster on 3/13/19.
//  Copyright © 2019 Thomas Foster. All rights reserved.
//

#include <stdio.h>
#include "dave.h"


object_t NewObject (objclass_t class, int x, int y)
{
	object_t new;
	
	memset(&new, 0, sizeof(object_t));
	
	new.objclass = class;
	new.def = &objdefs[class];
	SetObjectX(&new, x);
	SetObjectY(&new, y);
	new.active = yes;
	
	return new;
}

void NewObject1 (objclass_t class, int x, int y)
{
	object_t *new;
	
	new = malloc(sizeof(object_t));
	if (!new)
		Quit("NewObject: Error. Could not malloc object_t!");
	
	new->objclass = class;
	new->def = &objdefs[class];
}

void AddObject (object_t *obj)
{
	int i;
	
	//	*freeobj = *obj; //copy new object to free spot
	memcpy(freeobj, obj, sizeof(object_t));
	current.objectcount++;
	
	// set freeobj to next free spot
	for (i=0 ; i<MAXOBJ ; i++)
	{
		if (current.objects[i].objclass == nothing) {
			freeobj = &current.objects[i];
			if (i > lastobj) // update lastobj index
				lastobj = i - 1;
			break;
		}
	}
}


void RemoveObject (object_t *obj)
{
	obj->objclass = nothing;
	freeobj = obj;
}


//
// SetObjectX / SetObjectY
// Updates the object's x/y coordinate
// and adjusts the hitbox and tile position
//
void SetObjectX (object_t *obj, int x)
{
	if (x == obj->x) return;
	
	obj->x = x;
	obj->left = x;		// update hit box
	obj->right = x + obj->def->w;
	obj->tilex = (obj->x + obj->def->w/2) / TILESIZE;
}

void SetObjectY (object_t *obj, int y)
{
	if (y == obj->y) return;
	
	obj->y = y;
	obj->top = y; // update hit box
	obj->bottom = y + obj->def->h;
	obj->tiley = (obj->y + obj->def->h/2) / TILESIZE;
}

boolean ClipToSolidTileHoriz (object_t *obj)
{
	tile_t *UL,*UR,*LL,*LR;
	
	// current tile under each of the object's corners
	
	// if tile is solid, reset to previous position
	UL = &current.tilemap[tilecoord(obj->y)][tilecoord(obj->x)];
	UR = &current.tilemap[tilecoord(obj->y)][tilecoord(obj->right-1)];
	LL = &current.tilemap[tilecoord(obj->bottom-1)][tilecoord(obj->x)];
	LR = &current.tilemap[tilecoord(obj->bottom-1)][tilecoord(obj->right-1)];
	
	if (obj->xmove < 0 && (UL->def->solid || LL->def->solid)) {
		SetObjectX(obj, obj->oldx);
		return true;
	} else if (obj->xmove > 0 && (UR->def->solid || LR->def->solid)) {
		SetObjectX(obj, obj->oldx);
		return true;
	}
	return false;
}

boolean ClipToSolidTileVert (object_t *obj)
{
	tile_t *UL,*UR,*LL,*LR;
	
	UL = &current.tilemap[tilecoord(obj->y)][tilecoord(obj->x)];
	UR = &current.tilemap[tilecoord(obj->y)][tilecoord(obj->right-1)];
	LL = &current.tilemap[tilecoord(obj->bottom-1)][tilecoord(obj->x)];
	LR = &current.tilemap[tilecoord(obj->bottom-1)][tilecoord(obj->right-1)];
	
	if (obj->ymove < 0 && (UL->def->solid || UR->def->solid)) {
		SetObjectY(obj, obj->oldy);
		return true;
	}
	else if (obj->ymove > 0 && (LL->def->solid || LR->def->solid)) {
		SetObjectY(obj, obj->oldy);
		return true;
	}
	return false;
}


boolean OnScreen (object_t *obj)
{
	return (obj->bottom >= originy && obj->right >= originx &&
			obj->top <= originy + maxy && obj->left <= originx + maxx);
}


//
//	Render
//	Render the given object, adjusting the src rect for
//	objects that using a sprite sheet
//
void Render (object_t *obj)
{
	if (obj->objclass == nothing)
		Quit("Render: Error! free/removed object");
	
	SDL_Rect dst = { obj->x-originx, obj->y-originy, obj->def->w, obj->def->h };
	SDL_Rect src = { 0, 0, obj->def->w, obj->def->h };
	int flip = 0;

	// objects with a sprite sheet, calc the src rect
	switch (obj->objclass)
	{
		case playerclass:
			src.x = obj->dir * obj->def->w;
			src.y = obj->stage * obj->def->h;
			if (obj->tics && Blink(DAVE_BLINK)) // player hit, flash
				src.w = src.h = 0;
			break;
			
		case batclass:
			src.x = obj->stage * obj->def->w;
			src.y = obj->cooldown && Blink(DAVE_BLINK) ? obj->def->h : 0; // hit
			break;
			
		case snakeclass:
			src.x = obj->cooldown && Blink(DAVE_BLINK) ? obj->def->w : 0;
			src.y = obj->stage * obj->def->h;
			if (obj->dir == east)
				flip = SDL_FLIP_HORIZONTAL;
			break;
			
		case fireballclass:
			src.x = obj->dir * obj->def->w;
			src.y = obj->stage * obj->def->h;
			break;

		case torchclass:
		case fbexplodeclass:
			src.y = obj->stage * obj->def->h;
			
		default:
			break;
	}
	
	if (flip) {
		SDL_RenderCopyEx(renderer, objdefs[obj->objclass].tx->sdltx, &src, &dst, 0, NULL, flip);
	}
	else {
		SDL_RenderCopy(renderer, objdefs[obj->objclass].tx->sdltx, &src, &dst);
	}
	
#ifdef DRAWDEBUG
	
	int width = obj->right - obj->left;
	int height = obj->bottom - obj->top;
	SDL_Rect hitbox = { obj->left-originx,obj->top-originy,width,height };
	V_SetColor(RED);
	SDL_RenderDrawRect(renderer, &hitbox);

#endif
}
