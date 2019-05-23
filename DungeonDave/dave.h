//
//  dave.h
//  DungeonDave
//
//  Created by Thomas Foster on 2/15/19.
//  Copyright © 2019 Thomas Foster. All rights reserved.
//

#ifndef dave_h
#define dave_h

#include "support/video.h"
#include "object.h"
#include "info.h"

//#define DRAWDEBUG
//#define DAVE_DEBUG

#define WINDOW_W		320
#define WINDOW_H		200
// status bar
#define STATUS_W		WINDOW_W
#define STATUS_H		24
#define STATUS_X		0
#define STATUS_Y		WINDOW_H - STATUS_H
// play area
#define MAP_W			WINDOW_W
#define MAP_H			(WINDOW_H - STATUS_H)

#define NUMLEVELS		40
#define MAXOBJ			2000
#define TILESIZE 		16
#define MAPMAX 			512

#define DAVE_SHOT_COOLDOWN 5

#define bound(a,b,c)	(a) = ((a)<(b)) ? (b) : (((a)>(c)) ? (c) : (a))
#define sign(x)			(x)>0 ? 1 : ((x)<0 ? -1 : 0)
#define drawx(x)		(x)*TILESIZE-originx // tile coord to pixel w screen offset
#define drawy(y) 		(y)*TILESIZE-originy
#define tilecoord(n)	((int)n)/TILESIZE
//#define tilecoord(n)	n >> 4

typedef enum { title,play,editor,gameover } gamestate_t;


// for storing level data on disk
typedef struct
{
	objtype_t	type;
	int 		tilex, tiley;
} objdata_t;

typedef struct
{
	tiletype_t	type;
	int			tilex, tiley;
} tiledata_t;


// structure that represent level data to be stored on disk
typedef struct
{
	int 		width, height;
	int 		levelnum;
	int 		objectcount; // initial count after loading
	objdata_t	objects[MAXOBJ];
	tiledata_t	tilemap[MAPMAX][MAPMAX];
} leveldata_t;

//typedef struct
//{
//	
//} game_t;




// DAVE.C

extern const uint8_t *	key;
extern const SDL_Rect 	maprect;
extern const SDL_Rect 	statusrect;
extern const char *		classnames[NUMOBJECTS]; // debug

extern SDL_Texture 	*objtextures[NUMOBJECTS];
extern SDL_Texture	*tiletextures[NUMTILES];

extern leveldata_t 	current;
extern tile_t		tilemap[MAPMAX][MAPMAX];	// game tiles
extern float 		originx, originy; 	// for screen scrolling
extern float		maxx, maxy;
extern boolean 		levelloaded;
extern gamestate_t	gamestate;
extern int			tics;	// game time

void 	InitObject (object_t * obj, objtype_t type);
void 	SetObjectPosition (object_t * obj, int x, int y);
void 	Quit (char * error);
boolean Blink (int rate);
void 	UpdateOriginX (void);
void 	UpdateOriginY (void);
void 	UpdateOrigin (void);



// EDITOR.C

void 	EditorLoop (void);
void 	NewLevel (const char *fname, int width, int height, int levelnum);
boolean LoadLevel (leveldata_t *level, const char *name);

extern char filepath[80];



// DAVEPLAY.C

void Chase (object_t * obj);
void Flee (object_t * obj);

void BatThink (object_t *);
void BatChaseThink (object_t *);
void BatFleeThink (object_t *);
void BatContact (object_t *, object_t *);
void SpawnFireball (dir_t dir);
void FireBallThink (object_t *);
void FireBallContact (object_t *fb, object_t *hit);
void FireBallExplosionThink (object_t *expl);
void PlayerThink (object_t *);
void PlayerContact (object_t *, object_t *);
void SnakeThink (object_t *obj);
void SnakeContact (object_t *snake, object_t *hit);
void TorchThink (object_t *obj);




#endif /* dave_h */
