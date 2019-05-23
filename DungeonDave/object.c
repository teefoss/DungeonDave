//
//  object.c
//  DungeonDave
//
//  Created by Thomas Foster on 3/13/19.
//  Copyright © 2019 Thomas Foster. All rights reserved.
//

#include <stdio.h>
#include "dave.h"
#include "info.h"
#include "player.h"

// linked list
//object_t	objarray[MAXOBJ]; // holds current level's objects
object_t *first = NULL;


dir_t opposite[] =
{
	DI_SOUTH, DI_WEST, DI_NORTH, DI_EAST, DI_NODIR
};

#if 0
objflag_t objflags[NUMOBJECTS] =
{
	0,							//	OT_NOTHING,
	OF_SOLID | OF_SHOOTABLE,	//	OT_PLAYER,
	0,							//	OT_FIREBALL,
	0,							//	OT_FBEXPLOSION,
	OF_SOLID | OF_SHOOTABLE,	//	OT_BAT,
	OF_SOLID | OF_SHOOTABLE,	//	OT_SNAKE,
	OF_PICKUP,					//	OT_SMALLHEALTH,
	OF_PICKUP,					//	OT_BIGHEALTH,
	0							//	OT_TORCH,
};
#endif


//
//	SpawnObject
// 	initialize a new object with given type
// 	and add it to the list
//
object_t *
SpawnObject
( objtype_t type,
  int 		x,
  int 		y )
{
	object_t 	*obj;
	objinfo_t 	*info;
	
	obj = malloc ( sizeof(object_t) );
	memset ( obj, 0, sizeof(object_t) );
	
	info = &objinfo[type];

	obj->type = type;
	obj->state = &states[info->spawnstate];
	obj->info = info;
	obj->flags = info->spawnflags;
	obj->active = yes;

	obj->x = x;
	obj->y = y;
	obj->xradius = info->width / 2;
	obj->yradius = info->height / 2;
	obj->oldx = obj->x;
	obj->oldy = obj->y;
	
	obj->left = obj->x - obj->xradius; // hit box
	obj->top = obj->y - obj->yradius;
	obj->right = obj->x + obj->xradius;
	obj->bottom = obj->y + obj->yradius;
	
	obj->tilex = obj->x / TILESIZE; // tile
	obj->tiley = obj->y / TILESIZE;
	
	obj->srcrect.w = info->width; // sprite size
	obj->srcrect.h = info->height;
	obj->health = info->maxhp;

	// update list
	if ( first == NULL )
	{
		first = obj;
	}
	else
	{
		object_t *last = first;
		while ( last->next != NULL )
			last = last->next;
		last->next = obj;
	}
	obj->next = NULL;
	
	if ( type == OT_PLAYER )
	{
		InitPlayer(obj);
	}
	
	return obj;
}

// remove object from the linked list
void RemoveObject (object_t * obj)
{
	object_t * prev;
	
	if ( obj == first && obj->next == NULL ) // only member in list
		first = NULL;

	else if ( obj == first && obj->next != NULL ) // first in list
		first = obj->next;

	else
	{
		prev = first;
		while ( prev->next != obj )
			prev = prev->next;
		if ( obj->next == NULL )
			prev->next = NULL;
		else
			prev->next = obj->next;
	}
	
	free(obj);
	obj = NULL;
}



void FreeAllObjects (void)
{
	object_t *obj, *temp;
	
	if ( !first ) return;
	
	obj = first;
	do
	{
		temp = obj;
		obj = obj->next;
		free(temp);
	} while (obj);
	
	first = NULL;
}



boolean OnScreen ( object_t * obj )
{
	return (obj->bottom >= originy && obj->right >= originx &&
			obj->top <= originy + maxy && obj->left <= originx + maxx);
}



//
// CheckPosition
// returns false if object is within a solid tile/object.
//
boolean
CheckPosition
( object_t 	*obj,
  float 	x,
  float		y )
{
	boolean 	UL, UR, LL, LR; // corner in solid tile?
//	float 		right, bottom;
	object_t *	check;
	int top, bottom, left, right;
	
	// hit a solid object?
#if 0
	check = first;
	do
	{
		if (check != obj)
		{
			if (obj->right > check->left &&
				obj->left < check->right &&
				obj->top < check->bottom &&
				obj->bottom > check->top &&
				((obj->flags & OF_SOLID) || (check->flags & OF_SOLID)))
			{
				return false;
			}
		}
		check = check->next;
	} while (check);
#endif
	
	// hit a solid tile?
//	right = x + (float)obj->info->width - 1.0f;
//	bottom = y + (float)obj->info->height - 1.0f;
	
	top = y - obj->yradius;
	bottom = y + obj->yradius - 1;
	left = x - obj->xradius;
	right = x + obj->xradius - 1;

	// current tile under each of the object's corners
	UL = tilemap[tilecoord(top)][tilecoord(left)].flags & TF_SOLID;
	UR = tilemap[tilecoord(top)][tilecoord(right)].flags & TF_SOLID;
	LL = tilemap[tilecoord(bottom)][tilecoord(left)].flags & TF_SOLID;
	LR = tilemap[tilecoord(bottom)][tilecoord(right)].flags & TF_SOLID;
	
	return !UL && !UR && !LL & !LR;
}



boolean
TryMove
( object_t  *obj,
  float 	x,
  float 	y )
{
	if ( !CheckPosition(obj, x, y) )
		return false; // solid wall or location occupied
	
	obj->oldx = obj->x;
	obj->oldy = obj->y;
	obj->x = x;
	obj->y = y;
	
	obj->left 	= obj->x - (float)obj->xradius; // update hit box
	obj->top 	= obj->y - (float)obj->yradius;
	obj->right 	= obj->x + (float)obj->xradius;
	obj->bottom = obj->y + (float)obj->yradius;

	obj->tilex = obj->x / TILESIZE; // update tile
	obj->tiley = obj->y / TILESIZE;
			
	return true;
}

void XYMovement (object_t *obj)
{
	float 	speed;
	float 	try_x;
	float 	try_y;
	
	if (!obj->dx && ! obj->dy)
		return;
	
	speed = obj->info->speed;
	bound(obj->dx, -speed, speed);
	bound(obj->dy, -speed, speed);
	
	try_x = obj->x + obj->dx;// * dt;
	try_y = obj->y + obj->dy;// * dt;
	
	// this might be dumb
	if ( !TryMove(obj, try_x, try_y) )
		if ( !TryMove(obj, try_x, obj->y) )
			TryMove( obj, obj->x, try_y );
}



// activate when on screen
void LookForPlayer (object_t * obj)
{
	if (OnScreen(obj))
		SetObjState(obj, obj->state->nextstate);
}




//
//	SetObjState
//	Sets object's state or removes it
// 	returns false if removed
//
boolean
SetObjState
( object_t *  obj,
  statenum_t  st )
{
	if (st == S_NULL)
	{
		obj->state = &states[S_NULL];
		RemoveObject(obj);
		return false;
	}
	
	obj->state = &states[st];
	// start timer
	if (obj->state->duration)
		obj->tics = obj->state->duration;
		
	return true;
}




//
//	AnimateObject
// 	animation for object's current state
//
void AnimateObject (object_t * obj)
{
	state_t * s = obj->state;
	int h = obj->info->height;
	
	if (s->numframes == 0)
	{
		obj->srcrect.y = s->startrow * h;
		return; // no animation for this state
	}
	
	// advance frame
	if (tics % s->frameskip == 0)
	{
		obj->frame++;
		obj->frame = obj->frame % s->numframes;
	}
	
	// set srcrect:
	// obj thinker handles srcrect x (dir)
	obj->srcrect.y = (s->startrow * h) + (obj->frame * h);
}



void DamageObject (object_t * obj, int amt)
{
	obj->health -= amt;
	if (obj->health <= 0)
	{
		obj->health = 0;
		SetObjState(obj, S_NULL);
	}
}


//
// ObjectThinker
// All objects do this every frame
//
void ObjectThinker (object_t * obj)
{
	// set direction
	if (obj->dx < 0)
		obj->dir = DI_WEST;
	if (obj->dx > 0)
		obj->dir = DI_EAST;
	if (obj->dy < 0 && !obj->dx)
		obj->dir = DI_NORTH;
	if (obj->dy > 0 && !obj->dx)
		obj->dir = DI_SOUTH;
	
	// run type's thinker
	if (obj->info->think)
		obj->info->think(obj);
	
	// run state's thinker (animation, etc)
	if (obj->state->think)
		obj->state->think(obj);

	// decrement timer for finite states / progress state
	if (obj->tics)
	{
		if (--obj->tics == 0)
			if ( !SetObjState(obj, obj->state->nextstate) )
				return; // object removed itself
	}
	
	AnimateObject(obj);	
	XYMovement(obj);
	
	obj->dx = 0;
	obj->dy = 0;
}



void
BlinkObject
( object_t * obj,
  int 		 rate )
{
	if (SDL_GetTicks() % rate*2 < rate)
		obj->srcrect.w = 0;
	else
		obj->srcrect.w = obj->info->width;
}





//
//	Render
//	Render the given object, adjusting the src rect for
//	objects that using a sprite sheet
//
void RenderObject ( object_t * obj )
{
	SDL_Rect dst;
	
	dst.x = (int)obj->x - obj->xradius - originx;
	dst.y = (int)obj->y - obj->yradius - originy;
	dst.w = obj->info->width;
	dst.h = obj->info->height;

	SDL_RenderCopy(renderer, objtextures[obj->type], &obj->srcrect, &dst);
}

#if 0
void Render (object_t *obj)
{
	if (obj->type == OT_NOTHING)
		Quit("Render: Error! free/removed object");
	
	SDL_Rect dst = { obj->x-originx, obj->y-originy, obj->info->width, obj->info->height };
	SDL_Rect src = { 0, 0, obj->info->width, obj->info->height };
	int flip = 0;

	// objects with a sprite sheet, calc the src rect
	switch (obj->type)
	{
		case OT_PLAYER:
			src.x = obj->dir * obj->info->width;
			src.y = obj->stage * obj->info->height;
			if (obj->tics && Blink(DAVE_BLINK)) // player hit, flash
				src.w = src.h = 0;
			break;
			
		case OT_BAT:
			src.x = obj->stage * obj->info->width;
			src.y = obj->cooldown && Blink(DAVE_BLINK) ? obj->info->height : 0; // hit
			break;
			
		case OT_SNAKE:
			src.x = obj->cooldown && Blink(DAVE_BLINK) ? obj->info->width : 0;
			src.y = obj->stage * obj->info->height;
			if (obj->dir == DI_EAST)
				flip = SDL_FLIP_HORIZONTAL;
			break;
			
		case OT_FIREBALL:
			src.x = obj->dir * obj->info->width;
			src.y = obj->stage * obj->info->height;
			break;

		case OT_TORCH:
		case OT_FBEXPLOSION:
			src.y = obj->stage * obj->info->height;
			
		default:
			break;
	}
	
	if (flip) {
		SDL_RenderCopyEx(renderer, objinfo[obj->type].texture, &src, &dst, 0, NULL, flip);
	}
	else {
		SDL_RenderCopy(renderer, objinfo[obj->type].texture, &src, &dst);
	}
	
#ifdef DRAWDEBUG
	
	int width = obj->right - obj->left;
	int height = obj->bottom - obj->top;
	SDL_Rect hitbox = { obj->left-originx,obj->top-originy,width,height };
	V_SetColor(RED);
	SDL_RenderDrawRect(renderer, &hitbox);

#endif
}
#endif
