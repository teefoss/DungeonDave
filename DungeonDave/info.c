//
//  info.c
//  DungeonDave
//
//  Created by Thomas Foster on 5/1/19.
//  Copyright © 2019 Thomas Foster. All rights reserved.
//

#include <stdio.h>
#include "info.h"
#include "dave.h"
#include "player.h"

#define PLAYER_SPEED	0.8f
#define BAT_FRSKIP		8

// state_t -

//int 				numframes;	// number of sprite frames
//int				startrow;	// sprite sheet row to start animation in
//int				frameskip;	// animation - number of frames to skip
//int				duration;
//statenum_t 		nextstate;	// for transitions
//void 				(*think) (struct object_s *);

state_t states[NUMSTATES] =
{
	// S_NULL
	{
		0, 							// numframes
		0,							// startrow
		0,							// frameskip
		0,							// duration
		0,							// nextstate
		NULL						// thinker
	},
	// S_PL_STAND
	{
		0, 							// numframes
		0,							// startrow
		0,							// frameskip
		0,							// duration
		0,							// nextstate
		PlayerStandThink			// thinker
	},
	// S_PL_MOVE
	{
		2,							// numframes
		1,							// startrow
		8,							// frameskip
		0,							// duration
		0,							// nextstate
		PlayerMoveThink				// thinker
	},
	// S_BAT_STAND
	{
		0,							// numframes
		0,							// startrow
		0,							// frameskip
		0,							// duration
		S_BAT_CHASE,				// nextstate
		LookForPlayer				// thinker
	},
	// S_BAT_CHASE
	{
		2,							// numframes
		0,							// startrow
		BAT_FRSKIP,					// frameskip
		0,							// duration
		0,							// nextstate
		Chase						// thinker
	},
	// S_BAT_FLEE
	{
		2,							// numframes
		0,							// startrow
		BAT_FRSKIP,					// frameskip
		30,							// duration
		S_BAT_CHASE,				// nextstate
		Flee						// thinker
	},
};





//
//	OBJECT INFO
//

objinfo_t objinfo[NUMOBJECTS] =
{
	// OT_NOTHING
	{
		0,		// spawnstate
		0,		// spawnflags
		0, 0,	// size
		0,		// speed
		0,		// maxhp
		0,		// damage
		NULL,
		NULL
	},
	// 	OT_PLAYER,
	{
		S_PL_STAND,					// spawnstate
		OF_SHOOTABLE, 	// spawnflags
		8, 16,						// size
		PLAYER_SPEED,				// speed
		100,						// maxhp
		0,							// damage
		PlayerThink,
		PlayerContact
	},
	//	OT_FIREBALL,
	{
		0,		// spawnstate
		0,		// spawnflags
		8, 8,	// size
		5,		// speed
		0,		// maxhp
		10,		// damage
		FireBallThink,
		FireBallContact
	},
	{			// OT_FBEXPLOSION,
		0,		// spawnstate
		0,		// spawnflags
		16, 16,	// size
		0,		// speed
		0,		// maxhp
		0,		// damage
		FireBallExplosionThink,
		NULL
	},
	{//	OT_BAT,
		S_BAT_STAND,				// spawnstate
		OF_SOLID | OF_SHOOTABLE, 	// spawnflags
		16, 8,						// size
		PLAYER_SPEED*0.6f,			// speed
		25,							// maxhp
		5,							// damage
		BatThink,
		BatContact
	},
	{//	OT_SNAKE,
		0,		// spawnstate
		OF_SOLID | OF_SHOOTABLE, // spawnflags
		16, 8,	// size
		2,		// speed
		45,		// maxhp
		15,		// damage
		SnakeThink,
		SnakeContact
	},
	{//	OT_SMALLHEALTH,
		0,		// spawnstate
		OF_PICKUP, // spawnflags
		7, 7,	// size
		0,		// speed
		10,		// maxhp
		0,		// damage
		NULL,
		NULL
	},
	{//	OT_BIGHEALTH,
		0,		// spawnstate
		OF_PICKUP, // spawnflags
		12, 12,	// size
		0,		// speed
		25,		// maxhp
		0,		// damage
		NULL,
		NULL
	},
	{//	OT_TORCH,
		0,		// spawnstate
		0,		// spawnflags
		16, 16,	// size
		0,		// speed
		0,		// maxhp
		0,		// damage
		TorchThink,
		NULL
	},
};


int	tileflags[NUMTILES] =
{
	0,			// TT_EMPTY
	0,			// TT_FLOOR
	TF_SOLID	// TT_WALL
};
