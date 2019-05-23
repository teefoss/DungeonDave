//
//  object.h
//  DungeonDave
//
//  Created by Thomas Foster on 5/1/19.
//  Copyright © 2019 Thomas Foster. All rights reserved.
//

#ifndef object_h
#define object_h

#include "info.h"

typedef enum
{
	DI_NORTH,
	DI_EAST,
	DI_SOUTH,
	DI_WEST,
	DI_NODIR,
	NUMDIRS
} dir_t;

typedef enum
{
	OF_SOLID 		= 1,
	OF_SHOOTABLE	= 2,
	OF_JUSTHIT		= 4,
	OF_JUSTATTACHED = 8,
	OF_PICKUP		= 16,
	
} objflag_t;

typedef enum
{
	OT_NOTHING,
	OT_PLAYER,
	OT_FIREBALL,
	OT_FBEXPLOSION,
	OT_BAT,
	OT_SNAKE,
	OT_SMALLHEALTH,
	OT_BIGHEALTH,
	OT_TORCH,
	
	NUMOBJECTS
} objtype_t;

typedef enum
{
	S_NULL,
	S_PL_STAND,
	S_PL_MOVE,
	S_BAT_STAND,
	S_BAT_CHASE,
	S_BAT_FLEE,
	
	NUMSTATES
} statenum_t;

struct object_s;


// handles sprite and animation within a single state
// and state transitions
typedef struct
{
	int 			numframes;	// number of sprite frames
	int				startrow;	// sprite sheet row to start animation in
	int				frameskip;	// animation - number of frames to skip
	int				duration;	// for limited duration states
	statenum_t 		nextstate;	// for transitions
	
	void (*think) (struct object_s *);
} state_t;




// an objdef_t holds the abstract object class definition,
// these are stored in objinfo[] and initialized with InitObjDefs
typedef struct objinfo_s
{
	int		spawnstate;
	int		spawnflags;
	int		width;
	int		height;
	float	speed;
	int		maxhp;
	int		damage;
	
	void 	(*think) (struct object_s *);
	void	(*contact) (struct object_s *, struct object_s *);
} objinfo_t;






typedef struct object_s
{
	objtype_t	type;
	objinfo_t 	*info;
	objflag_t	flags;
		
	enum		{ no, yes, always, removable } active;
	
	// object center
	float		x;
	float		y;
	
	int			xradius;
	int			yradius;
	
	// position where it was last drawn
	float		oldx;
	float		oldy;
	
	// animation
	state_t		*state;
	int			tics; 		// state timer
	int			frame; 		// animation
	rect_t		srcrect;	// for drawing - source rect in sprite sheet
	
	// hitbox
	float		top, bottom, left, right;
	
	// the tile where the object's center is
	int			tilex;
	int			tiley;
	
	// object's momentum
	float		dx;
	float		dy;

	int			randx,randy;	// multiplier for random monster movement
	int			tics1,tics2;		// timers
	dir_t		dir;			// direction facing
	int			health;			// current hit points
	struct player_s	*player;	// extended info for player object

	struct object_s	*next;		// next in list
} object_t;

// obj information
extern state_t states[NUMSTATES];
extern objinfo_t objinfo[NUMOBJECTS];
//extern objflag_t objflags[NUMOBJECTS];

// object linked list
extern object_t *first;

object_t * 	SpawnObject (objtype_t type, int x, int y);
void 		RemoveObject (object_t * obj);
void 		FreeAllObjects (void);

// object operations
boolean OnScreen (object_t * obj);
void 	AnimateObject (object_t * obj);
void 	ObjectThinker (object_t * obj);
boolean SetObjState (object_t * obj, statenum_t st);
void 	XYMovement (object_t * obj);
void 	LookForPlayer (object_t * obj);
boolean CheckPosition (object_t * obj, float x, float y);
void 	RenderObject (object_t * obj);
void 	DamageObject (object_t * obj, int amt);
void 	BlinkObject (object_t * obj, int rate);
boolean TryMove (object_t * obj, float x, float y);


#endif /* object_h */
