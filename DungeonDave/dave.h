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
#define DAVE_BLINK		200 // sprite blinking rate
#define DAVE_SHOT_COOLDOWN 5

#define bound(a,b,c)	(a) = ((a)<(b)) ? (b) : (((a)>(c)) ? (c) : (a))
#define sign(x)			(x)>0 ? 1 : ((x)<0 ? -1 : 0)
#define drawx(x)		(x)*TILESIZE-originx // tile coord to pixel w screen offset
#define drawy(y) 		(y)*TILESIZE-originy
#define tilecoord(n)	(n)/TILESIZE
//#define tilecoord(n)	n >> 4


typedef enum { false, true } boolean;

typedef enum { title,play,editor,gameover } gamestate_t;
typedef enum { north,east,south,west,nodir } dir_t;

typedef enum {
	nothing,
	playerclass,
	fireballclass,
	fbexplodeclass,
	batclass,
	snakeclass,
	smhealthclass,
	bighealthclass,
	torchclass,
	NUMOBJECTS
} objclass_t;

typedef enum { notile,floorclass,wallclass,NUMTILES } tileclass_t;

struct objstruct;
struct objdef;

typedef struct objstruct
{
	objclass_t	objclass;
	enum		{ no, yes, always, removable } active;
//	boolean 	needstoreact;
	int 		cooldown;	// hit cooldown timer
	int			x,y;		// location of upper left corner in world
	int			top,bottom,left,right;	// edges
	int			tilex,tiley; // the tile where the object's center is
	int			xmove,ymove;
	int			randx,randy;	// multiplier for random monster movement
	int			stage;		// animation frame being drawn
	int 		delay;		// number of frames to pause without doing anything
	int			tics,tics1;		// timers
	dir_t		dir;		// direction facing
	int			hp;			// current hit points
	int			oldx, oldy;	// position where it was last drawn
	struct objdef *def;
} object_t;

// an objdef_t holds the abstract object class definition,
// these are stored in objdefs[] and initialized with InitObjDefs
typedef struct objdef
{
	void	(*think) (object_t *);
	void	(*contact) (object_t *, object_t *);
//	void	(*react) (object_t *);
	boolean	solid;
	int		w,h;
	int		speed;
	int		maxhp;
	int		damage;
	texture_t *tx;
} objdef_t;


typedef struct
{
	boolean	solid;
	int 	damage;	// damaging floors
	// other properties ...
} tiledef_t;

typedef struct
{
	tileclass_t	tileclass;
	int 		tilex,tiley;
	boolean		lightsrc;
	int 		light;			// alpha for a black rect
	tiledef_t	*def;
} tile_t;


typedef struct
{
	int 		width, height;	 	// of entire level,in 16*16 pixel tiles
	int			levelnum;			// which slot its in
	int			objectcount;
	object_t 	objects[MAXOBJ]; 	// list of all objects on level
	tile_t		tilemap[MAPMAX][MAPMAX];
} leveldata_t;

// for storing level data on disk
typedef struct
{
	objclass_t	objclass;
	int 		tilex, tiley;
} objstore_t;

typedef struct
{
	tileclass_t	tileclass;
	int			tilex, tiley;
} tilestore_t;


typedef struct
{
	int width, height;
	int levelnum;
	int objectcount;
	objstore_t	objects[MAXOBJ];
	tilestore_t	tiles[MAPMAX][MAPMAX];
} levelstore_t;

typedef struct
{
	
} game_t;


// DAVE.C

extern const uint8_t *key;
extern const SDL_Rect maprect;
extern const SDL_Rect statusrect;
extern const char 	*classnames[NUMOBJECTS]; // debug

extern leveldata_t 	current;
extern texture_t 	objtextures[NUMOBJECTS];
extern texture_t	tiletextures[NUMTILES];
extern int 			originx, originy; 	// for screen scrolling
extern int			maxx, maxy;
extern object_t		*player,*freeobj;
extern int 			lastobj;
extern boolean 		levelloaded;
extern gamestate_t	gamestate;
extern int			tics;

void 	InitObject (object_t *obj, objclass_t cl);
void 	SetObjectPosition (object_t *obj, int x, int y);
void 	Quit (char *error);
boolean Blink (int rate);
void 	UpdateOriginX (void);
void 	UpdateOriginY (void);



// OBJECT.C

void 	SetObjectX (object_t *obj, int x);
void 	SetObjectY (object_t *obj, int y);
boolean ClipToSolidTileHoriz (object_t *obj);
boolean ClipToSolidTileVert (object_t *obj);

object_t NewObject (objclass_t class, int x, int y);
void 	AddObject (object_t *obj);
void 	RemoveObject (object_t *obj);

boolean OnScreen (object_t *obj);
void	Render (object_t *obj);



// EDITOR.C

void 	EditorLoop (void);
void 	NewLevel (const char *fname, int width, int height, int levelnum);
boolean LoadLevel (leveldata_t *level, const char *name);

extern char filepath[80];



// DAVEPLAY.C

void BatThink (object_t *);
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


// DEF.C
extern objdef_t objdefs[NUMOBJECTS];
extern tiledef_t tiledefs[NUMTILES];

void InitObjDefs (void);
void InitTileDefs (void);


#endif /* dave_h */
