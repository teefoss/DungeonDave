//
//  light.h
//  DungeonDave
//
//  Created by Thomas Foster on 3/21/19.
//  Copyright © 2019 Thomas Foster. All rights reserved.
//

#ifndef light_h
#define light_h

#include <stdio.h>

#define DEFAULT_BRIGHTNESS	128

typedef struct
{
	int strength;
	
} light_t;

void InitLighting (void);
void FloodLightAt (int x, int y, int radius);

#endif /* light_h */
