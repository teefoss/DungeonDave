//
//  info.h
//  DungeonDave
//
//  Created by Thomas Foster on 5/1/19.
//  Copyright © 2019 Thomas Foster. All rights reserved.
//

#ifndef info_h
#define info_h

#include "type.h"
#include <SDL2/SDL.h>

typedef enum
{
	TT_EMPTY,
	TT_FLOOR,
	TT_WALL,
	
	NUMTILES
} tiletype_t;

typedef enum
{
	TF_SOLID 	= 1,
	TF_LIGHTSRC	= 2,
	TF_DAMAGING	= 4
	// other properties ...
} tileflags_t;

typedef struct
{
	tiletype_t	type;
	int			flags;
	
	// grid location
	int 		tilex;
	int			tiley;
	
	// light level (alpha for a overlaid black rect)
	int 		light;
} tile_t;

extern int	tileflags[NUMTILES];

#endif /* info_h */
