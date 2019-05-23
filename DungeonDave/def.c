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
//objinfo_t objinfo[NUMOBJECTS];
//tileinfo_t tileinfo[NUMTILES];

#if 0
void InitObjDefs ()
{
	objinfo[OT_NOTHING] = (objinfo_t) {
		NULL,NULL,false,0,0,0,0,0,NULL
	};
	
	objinfo[OT_PLAYER] = (objinfo_t) {
		PlayerThink,PlayerContact,
		false,
		8,16,	// size
		4,		// speed
		100,	// maxhp
		0,		// damage
		&objtextures[OT_PLAYER]
	};
	
	objinfo[OT_FIREBALL] = (objinfo_t) {
		FireBallThink,FireBallContact,
		false, 		// solid
		8,8,		// size
		5,			// speed
		0,			// maxhp
		10,			// damage
		&objtextures[OT_FIREBALL]
	};
	
	objinfo[OT_FBEXPLOSION] = (objinfo_t) {
		FireBallExplosionThink,NULL,
		false,
		16,16,
		0,
		0,
		0,
		&objtextures[OT_FBEXPLOSION]
	};
	
	objinfo[OT_BAT] = (objinfo_t) {
		BatThink,BatContact,
		true,
		16,8, 	// size
		4,		// speed
		25,		// maxhp
		5,		// damage
		&objtextures[OT_BAT]
	};

	objinfo[OT_SNAKE] = (objinfo_t) {
		SnakeThink,SnakeContact,
		true,
		16,8, 	// size
		2,		// speed
		45,		// maxhp
		15,		// damage
		&objtextures[OT_SNAKE]
	};
	
	objinfo[OT_SMALLHEALTH] = (objinfo_t) {
		SnakeThink,NULL,
		false,
		7,7, 	// size
		0,		// speed
		10,		// maxhp
		0,		// damage
		&objtextures[OT_SMALLHEALTH]
	};
	
	objinfo[OT_BIGHEALTH] = (objinfo_t) {
		SnakeThink,NULL,
		false,
		12,12, 	// size
		0,		// speed
		25,		// maxhp
		0,		// damage
		&objtextures[OT_BIGHEALTH]
	};

	objinfo[OT_TORCH] = (objinfo_t) {
		TorchThink,NULL,
		false,
		16,16, 	// size
		0,		// speed
		0,		// maxhp
		0,		// damage
		&objtextures[OT_TORCH]
	};

}


void InitTileDefs ()
{
	tileinfo[TT_EMPTY] = (tileinfo_t) {
		false,0
	};
	tileinfo[TT_FLOOR] = (tileinfo_t) {
		false,0
	};
	tileinfo[TT_WALL] = (tileinfo_t) {
		true,0
	};
}
#endif
