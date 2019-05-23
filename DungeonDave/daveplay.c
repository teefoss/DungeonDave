//
//  daveplay.c
//  DungeonDave
//
//  Created by Thomas Foster on 2/16/19.
//  Copyright © 2019 Thomas Foster. All rights reserved.
//
//	Object Thinkers, Contact, Reacts

#include <stdio.h>
#include "dave.h"
#include "random.h"
#include "light.h"
#include "player.h"

#define FLINCH_SPEED 2

#pragma mark - Spawners

void SpawnFireball (dir_t dir)
{
	int w, h;
	int offs;
	object_t *fb = NULL;
	
	w = objinfo[OT_FIREBALL].width;
	h = objinfo[OT_FIREBALL].height;
	offs = objinfo[OT_FIREBALL].speed; // fireball starts right next to player

	switch (dir)
	{
		case DI_NORTH:
			fb = SpawnObject(OT_FIREBALL, player.obj->left, player.obj->top-h+offs);
			break;
		case DI_EAST:
			fb = SpawnObject(OT_FIREBALL, player.obj->right-offs, player.obj->top);
			break;
		case DI_SOUTH:
			fb = SpawnObject(OT_FIREBALL, player.obj->left, player.obj->bottom-offs);
			break;
		case DI_WEST:
			fb = SpawnObject(OT_FIREBALL, player.obj->left-w+offs, player.obj->top);
			break;

		default:
			Quit("SpawnFireball: bad direction!");
			break;
	}

	fb->dir = dir;
	//AddObject(&fb);
	player.cooldown = DAVE_SHOT_COOLDOWN;
}


void PickupHealth (object_t * health)
{
	if (player.obj->health < 100) {
		player.obj->health += health->info->maxhp;
		if (player.obj->health > 100)
			player.obj->health = 100;
		SetObjState(health, S_NULL);
	}
}



//==============================================================================

#pragma mark - THINKERS

#define BAT_SPEED 	2
#define BAT_DELAY	4

void Chase (object_t * obj)
{
	float spd = obj->info->speed;
	
	if (Random() > 200) {
		obj->dx = (Random() % 2 == 0) ? spd : -spd;
		obj->dy = (Random() % 2 == 0) ? spd : -spd;
	} else {
		obj->dx = spd * (sign(player.obj->x - obj->x));
		obj->dy = spd * (sign(player.obj->y - obj->y));
	}
}

void Flee (object_t * obj)
{
	obj->dx = obj->info->speed * -(sign(player.obj->x - obj->x));
	obj->dy = obj->info->speed * -(sign(player.obj->y - obj->y));
}


void BatThink (object_t * obj)
{
	if (obj->health <= 0)
		SetObjState(obj, S_NULL);
	
	if (!OnScreen(obj))
		SetObjState(obj, S_BAT_STAND);
}

void SnakeThink (object_t *obj)
{
#if 0
//	if (obj->delay) {
//		obj->delay--;
//		return;
//	}
//	obj->delay = BAT_DELAY;
	
	// todo
//	if (obj->cooldown)
//		obj->cooldown--;
	
	obj->oldx = obj->x;
	obj->oldy = obj->y;
//	obj->stage ^= 1;
	
	obj->dx = obj->info->speed * (sign(player.obj->x - obj->x));
	obj->dy = obj->info->speed * (sign(player.obj->y - obj->y));

	// set direction
	if (obj->dx < 0)
		obj->dir = DI_WEST;
	if (obj->dx > 0)
		obj->dir = DI_EAST;
	
	// if tics timer, apply random motion
	if (obj->tics) {
		obj->dx *= obj->randx;
		obj->dy *= obj->randy;
		obj->tics--;
	}
	
	if (obj->dx)
	{
		SetObjectX(obj, obj->x + obj->dx);
		ClipToSolidTileHoriz(obj);
	}
	
	if (obj->dy)
	{
		SetObjectY(obj, obj->y + obj->dy);
		ClipToSolidTileVert(obj);
	}
	
	if (obj->health < 0)
		obj->active = removable;
#endif
}







#define FIREBALL_LIGHT 2


void FireBallThink (object_t *obj)
{
#if 0
	FloodLightAt(obj->tilex, obj->tiley, FIREBALL_LIGHT);
	
//	if (obj->delay) {
//		obj->delay--;
//		return;
//	}
//	obj->delay = DAVE_DELAY - 2;
	
	if (!obj->dx && !obj->dy) // new fireball, set x/y move
	{
		switch (obj->dir) // fb's are set up with a dir
		{
			case DI_NORTH: 	obj->dy = -obj->info->speed; break;
			case DI_SOUTH:		obj->dy =  obj->info->speed; break;
			case DI_EAST:		obj->dx =  obj->info->speed; break;
			case DI_WEST:		obj->dx = -obj->info->speed; break;
			default:
				Quit("FireBallThink: Bad object direction!");
		}
	}
	
	// todo
//	obj->stage ^= 1; // update animation frame
	obj->oldx = obj->x; // save in case it needs to be moved back
	obj->oldy = obj->y;

	if (obj->dx) {
		SetObjectX(obj, obj->x + obj->dx);
		if (ClipToSolidTileHoriz(obj)) {
			obj->active = removable;
			//object_t *explosion =
			SpawnObject(OT_FBEXPLOSION, obj->x-4, obj->y-4);
			//AddObject(&explosion);
		}
	}
	if (obj->dy) {
		SetObjectY(obj, obj->y + obj->dy);
		if (ClipToSolidTileVert(obj)) {
			obj->active = removable;
			//object_t *explosion =
			SpawnObject(OT_FBEXPLOSION, obj->x-4, obj->y-4);
			//AddObject(&explosion);
		}
	}
	
	if (!OnScreen(obj))
		obj->active = removable;
	
//	obj->xmove = 0;
//	obj->ymove = 0;
#endif
}

#define FB_EXPLODE_DELAY 	4

void FireBallExplosionThink (object_t *expl)
{
	FloodLightAt(expl->tilex, expl->tiley, FIREBALL_LIGHT);
}


void TorchThink (object_t *obj)
{
	FloodLightAt(obj->tilex, obj->tiley, 3);
	
//	obj->state->frame++;
//	obj->state->frame &= obj->state->numframes;
}


//==============================================================================
#pragma mark - Contact
//==============================================================================

void BatContact (object_t *bat, object_t *hit)
{
//	if (hit->flags & OF_SOLID) {
//		TryMove(bat, bat->oldx, bat->oldy);
//		TryMove(hit, hit->oldx, hit->oldy);
//	}

	switch (hit->type)
	{
		case OT_PLAYER: // always move away from the player
			SetObjState(bat, S_BAT_FLEE);
			break;
			
		case OT_SNAKE:
		case OT_BAT: // random motion
			bat->randx = Random() % 3 - 1;
			bat->randy = Random() % 3 - 1;
			break;
		default:
			break;
	}
}


void SnakeContact (object_t *snake, object_t *hit)
{
	switch (hit->type)
	{
		case OT_PLAYER: // snake stops for a bit
			if (!snake->tics)
				snake->tics = 10;
			snake->randx = 0;
			snake->randy = 0;
			break;
			
		case OT_BAT:
		case OT_SNAKE:
			if (!snake->tics)
				snake->tics = 10;
			snake->randx = Random() % 3 - 1;
			snake->randy = Random() % 3 - 1;
			
		default:
			break;
	}
}


void PlayerContact (object_t *player, object_t *hit)
{
	if (player->type != OT_PLAYER)
		Quit("Error! Sent a non-player object to PlayerContact");

	switch (hit->type)
	{
		// ENEMY COLLISIONS
		case OT_BAT:
		case OT_SNAKE:
			DamagePlayer(hit->info->damage);
			break;
			
		case OT_SMALLHEALTH:
		case OT_BIGHEALTH:
			PickupHealth(hit);
			break;
			
		default:
			break;
	}
}

void FireBallContact (object_t *fb, object_t *hit)
{
	switch (hit->type) {
		case OT_BAT:
		case OT_SNAKE:
			//hit->cooldown = 10;
			hit->health -= fb->info->damage;
			break;
		default:
			break;
	}
	
	if (hit->flags & OF_SOLID) {
		fb->active = removable;
		//object_t explosion =
		SpawnObject(OT_FBEXPLOSION, fb->x-4, fb->y-4);
		//AddObject(&explosion);
	}
}
