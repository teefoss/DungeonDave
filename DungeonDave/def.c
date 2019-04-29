//
//  objdef.c
//  DungeonDave
//
//  Created by Thomas Foster on 2/15/19.
//  Copyright © 2019 Thomas Foster. All rights reserved.
//
//	Per-class definitions for tiles and objects

#include "dave.h"

// defs[] stores the object definitions for all classes
objdef_t objdefs[NUMOBJECTS];
tiledef_t tiledefs[NUMTILES];


void InitObjDefs ()
{
	objdefs[nothing] = (objdef_t) {
		NULL,NULL,false,0,0,0,0,0,NULL
	};
	
	objdefs[playerclass] = (objdef_t) {
		PlayerThink,PlayerContact,
		false,
		8,16,	// size
		4,		// speed
		100,	// maxhp
		0,		// damage
		&objtextures[playerclass]
	};
	
	objdefs[fireballclass] = (objdef_t) {
		FireBallThink,FireBallContact,
		false, 		// solid
		8,8,		// size
		5,			// speed
		0,			// maxhp
		10,			// damage
		&objtextures[fireballclass]
	};
	
	objdefs[fbexplodeclass] = (objdef_t) {
		FireBallExplosionThink,NULL,
		false,
		16,16,
		0,
		0,
		0,
		&objtextures[fbexplodeclass]
	};
	
	objdefs[batclass] = (objdef_t) {
		BatThink,BatContact,
		true,
		16,8, 	// size
		4,		// speed
		25,		// maxhp
		5,		// damage
		&objtextures[batclass]
	};

	objdefs[snakeclass] = (objdef_t) {
		SnakeThink,SnakeContact,
		true,
		16,8, 	// size
		2,		// speed
		45,		// maxhp
		15,		// damage
		&objtextures[snakeclass]
	};
	
	objdefs[smhealthclass] = (objdef_t) {
		SnakeThink,NULL,
		false,
		7,7, 	// size
		0,		// speed
		10,		// maxhp
		0,		// damage
		&objtextures[smhealthclass]
	};
	
	objdefs[bighealthclass] = (objdef_t) {
		SnakeThink,NULL,
		false,
		12,12, 	// size
		0,		// speed
		25,		// maxhp
		0,		// damage
		&objtextures[bighealthclass]
	};

	objdefs[torchclass] = (objdef_t) {
		TorchThink,NULL,
		false,
		16,16, 	// size
		0,		// speed
		0,		// maxhp
		0,		// damage
		&objtextures[torchclass]
	};

}


void InitTileDefs ()
{
	tiledefs[notile] = (tiledef_t) {
		false,0
	};
	tiledefs[floorclass] = (tiledef_t) {
		false,0
	};
	tiledefs[wallclass] = (tiledef_t) {
		true,0
	};
}
