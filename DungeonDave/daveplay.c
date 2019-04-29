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

#define DAVE_DELAY		4
#define DAVE_COOLDOWN	17


#pragma mark - Spawners

void SpawnFireball (dir_t dir)
{
	int w, h;
	int offs;
	object_t fb;
	
	w = objdefs[fireballclass].w;
	h = objdefs[fireballclass].h;
	offs = objdefs[fireballclass].speed; // fireball starts right next to player

	switch (dir)
	{
		case north:
			fb = NewObject(fireballclass, player->left, player->top-h+offs);
			break;
		case east:
			fb = NewObject(fireballclass, player->right-offs, player->top);
			break;
		case south:
			fb = NewObject(fireballclass, player->left, player->bottom-offs);
			break;
		case west:
			fb = NewObject(fireballclass, player->left-w+offs, player->top);
			break;

		default:
			Quit("SpawnFireball: bad direction!");
			break;
	}

	fb.dir = dir;
	AddObject(&fb);
	player->tics1 = DAVE_SHOT_COOLDOWN;
}



//==============================================================================
#pragma mark - Thinkers
//==============================================================================
//
// Thinkers are only called when objects are active
//

#define BAT_SPEED 	2
#define BAT_DELAY	4

void BatThink (object_t *obj)
{
	if (obj->delay) {
		obj->delay--;
		return;
	}
	obj->delay = BAT_DELAY;
	
	if (obj->cooldown)
		obj->cooldown--;

	obj->oldx = obj->x;
	obj->oldy = obj->y;
	obj->stage ^= 1;

	obj->xmove = BAT_SPEED * (sign(player->x - obj->x));
	obj->ymove = BAT_SPEED * (sign(player->y - obj->y));
	
	// if tics timer, apply random motion
	if (obj->tics) {
		obj->xmove *= obj->randx;
		obj->ymove *= obj->randy;
		obj->tics--;
	}
	
	if (obj->xmove)
	{
		SetObjectX(obj, obj->x + obj->xmove);
		ClipToSolidTileHoriz(obj);
	}
	
	if (obj->ymove)
	{
		SetObjectY(obj, obj->y + obj->ymove);
		ClipToSolidTileVert(obj);
	}
	
	if (obj->hp < 0)
		obj->active = removable;
}


void SnakeThink (object_t *obj)
{
	if (obj->delay) {
		obj->delay--;
		return;
	}
	obj->delay = BAT_DELAY;
	
	if (obj->cooldown)
		obj->cooldown--;
	
	obj->oldx = obj->x;
	obj->oldy = obj->y;
	obj->stage ^= 1;
	
	obj->xmove = obj->def->speed * (sign(player->x - obj->x));
	obj->ymove = obj->def->speed * (sign(player->y - obj->y));

	// set direction
	if (obj->xmove < 0)
		obj->dir = west;
	if (obj->xmove > 0)
		obj->dir = east;
	
	// if tics timer, apply random motion
	if (obj->tics) {
		obj->xmove *= obj->randx;
		obj->ymove *= obj->randy;
		obj->tics--;
	}
	
	if (obj->xmove)
	{
		SetObjectX(obj, obj->x + obj->xmove);
		ClipToSolidTileHoriz(obj);
	}
	
	if (obj->ymove)
	{
		SetObjectY(obj, obj->y + obj->ymove);
		ClipToSolidTileVert(obj);
	}
	
	if (obj->hp < 0)
		obj->active = removable;
}




void PlayerThink (object_t *obj)
{	
	if (obj->delay) {
		obj->delay--;
		return;
	}
	obj->delay = DAVE_DELAY;
	
	if (obj->tics) {
		obj->tics--; // cooldown timer
	}
	if (obj->tics1) {
		obj->tics1--; // shot timer
	}
	
	// update player's direction
	if (obj->xmove < 0)		obj->dir = west;
	if (obj->xmove > 0)		obj->dir = east;
	if (obj->ymove < 0 && !obj->xmove)	obj->dir = north;
	if (obj->ymove > 0 && !obj->xmove)	obj->dir = south;
	
	// update animation frame, only if moving
	if ( (obj->xmove || obj->ymove) ) {
		obj->stage++;
		if (obj->stage > 2)
			obj->stage = 1;
	} else {
		obj->stage = 0; // standing animation
	}

	obj->oldx = obj->x; // save in case it needs to be moved back
	obj->oldy = obj->y;
	
	if (obj->xmove)
	{
		SetObjectX(obj, obj->x + obj->xmove);
		ClipToSolidTileHoriz(obj);
		UpdateOriginX();
	}
	
	if (obj->ymove)
	{
		SetObjectY(obj, obj->y + obj->ymove);
		ClipToSolidTileVert(obj);
		UpdateOriginY();
	}
	
	obj->xmove = 0;
	obj->ymove = 0;
}


#define FIREBALL_LIGHT 2

void FireBallThink (object_t *obj)
{
	FloodLightAt(obj->tilex, obj->tiley, FIREBALL_LIGHT);
	
	if (obj->delay) {
		obj->delay--;
		return;
	}
	obj->delay = DAVE_DELAY - 2;
	
	if (!obj->xmove && !obj->ymove) // new fireball, set x/y move
	{
		switch (obj->dir) // fb's are set up with a dir
		{
			case north: 	obj->ymove = -obj->def->speed; break;
			case south:		obj->ymove =  obj->def->speed; break;
			case east:		obj->xmove =  obj->def->speed; break;
			case west:		obj->xmove = -obj->def->speed; break;
			default:
				Quit("FireBallThink: Bad object direction!");
		}
	}
	
	obj->stage ^= 1; // update animation frame
	obj->oldx = obj->x; // save in case it needs to be moved back
	obj->oldy = obj->y;

	if (obj->xmove) {
		SetObjectX(obj, obj->x + obj->xmove);
		if (ClipToSolidTileHoriz(obj)) {
			obj->active = removable;
			object_t explosion = NewObject(fbexplodeclass, obj->x-4, obj->y-4);
			AddObject(&explosion);
		}
	}
	if (obj->ymove) {
		SetObjectY(obj, obj->y + obj->ymove);
		if (ClipToSolidTileVert(obj)) {
			obj->active = removable;
			object_t explosion = NewObject(fbexplodeclass, obj->x-4, obj->y-4);
			AddObject(&explosion);
		}
	}
	
	if (!OnScreen(obj))
		obj->active = removable;
	
//	obj->xmove = 0;
//	obj->ymove = 0;
}

#define FB_EXPLODE_DELAY 	DAVE_DELAY

void FireBallExplosionThink (object_t *expl)
{
	FloodLightAt(expl->tilex, expl->tiley, FIREBALL_LIGHT);
	
	if (expl->delay) {
		expl->delay--;
		return;
	}
	expl->delay = FB_EXPLODE_DELAY;

	expl->stage++;
	if (expl->stage > 4)
		expl->active = removable;
}


void TorchThink (object_t *obj)
{
	FloodLightAt(obj->tilex, obj->tiley, 3);

	if (obj->delay) {
		obj->delay--;
		return;
	}
	obj->delay = DAVE_DELAY + 2;
	
	obj->stage++;
	obj->stage = obj->stage % 3;
}



//==============================================================================
#pragma mark - Contact
//==============================================================================

void BatContact (object_t *bat, object_t *hit)
{
//	if (hit->def->solid) {
//		SetObjectX(bat, bat->oldx);
//		SetObjectY(bat, bat->oldy);
//	}

	switch (hit->objclass)
	{
		case playerclass: // always move away from the player
			if (!bat->tics)
				bat->tics = 10;
			bat->randx = -1;
			bat->randy = -1;
			break;
			
		case snakeclass:
		case batclass: // random bounce
			if (!bat->tics)
				bat->tics = 5;
			bat->randx = Random() % 3 - 1;
			bat->randy = Random() % 3 - 1;
			break;
		default:
			break;
	}
}


void SnakeContact (object_t *snake, object_t *hit)
{
	switch (hit->objclass)
	{
		case playerclass: // snake stops for a bit
			if (!snake->tics)
				snake->tics = 10;
			snake->randx = 0;
			snake->randy = 0;
			break;
			
		case batclass:
		case snakeclass:
			if (!snake->tics)
				snake->tics = 10;
			snake->randx = Random() % 3 - 1;
			snake->randy = Random() % 3 - 1;
			
		default:
			break;
	}
}


void PlayerContact (object_t *pl, object_t *hit)
{
	switch (hit->objclass)
	{
		// ENEMY COLLISIONS
		case batclass:
		case snakeclass:
			if (!pl->tics) {
				pl->hp -= hit->def->damage;
				pl->tics = DAVE_COOLDOWN;
			}
			break;
			
		case smhealthclass:
		case bighealthclass:
			if (pl->hp < 100) {
				pl->hp += hit->def->maxhp;
				if (pl->hp > 100)
					pl->hp = 100;
				hit->active = removable;
			}
			break;
			
		default:
			break;
	}
}

void FireBallContact (object_t *fb, object_t *hit)
{
	switch (hit->objclass) {
		case batclass:
		case snakeclass:
			hit->cooldown = 10;
			hit->hp -= fb->def->damage;
			break;
		default:
			break;
	}
	
	if (hit->def->solid) {
		fb->active = removable;
		object_t explosion = NewObject(fbexplodeclass, fb->x-4, fb->y-4);
		AddObject(&explosion);
	}
}



//==============================================================================
#pragma mark - React
//==============================================================================

void PlayerReact_HitMonster (object_t *mon)
{
//	if (!player->tics)
//		player->tics = 100; // start timer
//	if (--player->tics == 1) {
//		player->needstoreact = false;
//		player->def->react = NULL;
//		player->tics = 0;
//	}
}
