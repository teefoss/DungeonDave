//
//  light.c
//  DungeonDave
//
//  Created by Thomas Foster on 3/21/19.
//  Copyright © 2019 Thomas Foster. All rights reserved.
//

#include "light.h"
#include "dave.h"

//
// LightLevel
// Set current level's light properties
//
void InitLighting ()
{
	int x,y;
	

	// reset set all tiles to default
	// TODO def this or adjust per level!
	for (y=0 ; y<current.height ; y++) {
		for (x=0 ; x<current.width ; x++)
		{
			current.tilemap[y][x].lightsrc = false;
			current.tilemap[y][x].light = 128;
		}
	}
}


//
// FloodLight At
// x, y: origin of light source
// radius: extent of light flood (strength)
//
void FloodLightAt (int x, int y, int radius)
{
	int maxlevels = radius*2; // max different light levels within range
	int ld = DEFAULT_BRIGHTNESS / maxlevels; // light delta

	for (int y1 = y-radius ; y1 <= y+radius ; y1++)
	{
		if (y1 < 0 || y1 > current.height)
			continue;
		
		for (int x1 = x-radius ; x1 <= x+radius ; x1++)
		{
			if (x1 < 0 || x1 > current.width)
				continue;
			
			tile_t *set = &current.tilemap[y1][x1];
			int xdist = abs(x1 - x);
			int ydist = abs(y1 - y);
			set->light -= DEFAULT_BRIGHTNESS - ((xdist+ydist) * ld);
			if (set->light < 0)
				set->light = 0;
		}
	}
}
