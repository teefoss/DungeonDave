//
//  trash.c
//  DungeonDave
//
//  Created by Thomas Foster on 2/22/19.
//  Copyright © 2019 Thomas Foster. All rights reserved.
//

// Just a bunch of shit, but might need it again...

#if 0

//
// AlignedVert / AlignedHoriz
// Returns true if the two objects are aligned vertically/horizontally
// use 'oldpos' to check with the obj's oldx and oldy
//
boolean AlignedVert (object_t *obj1, object_t *obj2, boolean oldpos)
{
	int l1 = oldpos ? obj1->oldx 				: obj1->left;
	int r1 = oldpos ? obj1->oldx + obj1->def->w : obj1->right;
	int l2 = oldpos ? obj2->oldx				: obj2->left;
	int r2 = oldpos ? obj2->oldx + obj2->def->w : obj2->right;
	
	return (r1 > l2 && l1 < r2);
}

boolean AlignedHoriz (object_t *obj1, object_t *obj2, boolean oldpos)
{
	int t1 = oldpos ? obj1->oldy 				: obj1->top;
	int b1 = oldpos ? obj1->oldy + obj1->def->h : obj2->bottom;
	int t2 = oldpos ? obj2->oldy 				: obj1->top;
	int b2 = oldpos ? obj2->oldy + obj2->def->h : obj2->bottom;
	
	return (t1 < b2 && b1 > t2);
}

#endif


// compare both objects tile alignement
#if 0
// horiz align
if (pl->tilex == hit->tilex) {
	if (pl->tiley > hit->tiley) // player is below
		SetObjectY(pl, hit->bottom+1);
		else
			SetObjectY(pl, hit->top-pl->def->h-1); // player is above
			}
// vert align
if (pl->tiley == hit->tiley) {
	if (pl->tilex < hit->tilex) // player is to the left
		SetObjectX(pl, hit->left-pl->def->w-1);
		else
			SetObjectX(pl, hit->right+1); // player is to the right
			}
#endif

// see which sides are close together and clip the player to it
#if 0
if (abs(pl->top - hit->bottom) <= DAVE_ACCEL)
SetObjectY(pl, hit->bottom+1);
if (abs(pl->bottom - hit->top) <= DAVE_ACCEL)
SetObjectY(pl, hit->top - pl->def->h);
if (abs(pl->left - hit->right) <= DAVE_ACCEL)
SetObjectX(pl, hit->right+1);
if (abs(pl->right - hit->left) <= DAVE_ACCEL)
SetObjectY(pl, hit->left - pl->def->w);
#endif

// check the old pos to find alignment, clip to side
#if 0

if (AlignedVert(pl, hit, true)) {
	if (pl->top < hit->top) // player is above
		SetObjectY(pl, hit->top - pl->def->h);
		else if (pl->bottom > hit->bottom) // player is below
			SetObjectY(pl, hit->bottom + 1);
			}
if (AlignedHoriz(pl, hit, true)) {
	if (pl->left < hit->left) // player is to the left
		SetObjectX(pl, hit->left - pl->def->w);
		if (pl->right > hit->right) // player is to the right
			SetObjectX(pl, hit->right + 1);
			}
#endif
