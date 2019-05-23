//
//  editor.c
//  DungeonDave
//
//  Created by Thomas Foster on 2/15/19.
//  Copyright © 2019 Thomas Foster. All rights reserved.
//

#include <unistd.h>
#include <sys/stat.h>
#include "dave.h"
#include "light.h"

// Structure to hold editor cursor info
typedef struct
{
	int 		x, y;	// current tile location
	objtype_t	obj;			// currently selected item for tilemode
	tiletype_t	tile;			// currently selected item for objmode
	int			fixedx,fixedy;		// fill box
} cursor_t;

const char *fillnames[] = { "Single", "Fill" };

cursor_t csr;
boolean fill; 		// fill mode is on/off
boolean dragging;	// currently dragging a fill box
boolean delconfim; 	// press x twice for fill delete

// objects don't move in the editor, so just put them all here
// for easier display / editing.
// when saved, objview is parsed and objs are added to an array

objtype_t 	objview[MAPMAX][MAPMAX];

char filepath[80] = "levels/map01.dave";



boolean FileExists (const char *name)
{
	struct stat buffer;
	return (stat(name, &buffer) == 0);
}

boolean ShiftKey()
{
	return key[SDL_SCANCODE_LSHIFT]||key[SDL_SCANCODE_LSHIFT];
}



// initialize an object that was stored in level data
// tilex, tiley, and class are already set
void LoadObject (object_t *obj)
{
	
}


#pragma mark -

//
// NewLevel
// If filename already exists, Quit()s
// Initializes map properties in 'current'
//
void
NewLevel
( const char *filename,
  int 		 width,	// in tiles
  int 		 height,
  int 		 levelnum ) // = game level slot
{
	FILE *stream;
	
	if (FileExists(filename)) {
		printf("NewLevel: File %s already exists!",filename);
		Quit(NULL);
	}
	memset(&current, 0, sizeof(current));
	current.width = width;
	current.height = height;
	current.levelnum = levelnum;
	
	stream = fopen(filename, "wb"); // create the file
	fclose(stream);
	if (FileExists(filename))
		printf("NewLevel: File successfully created");
	else
		Quit("NewLevel: Error. Could not create file");
}




//
// SaveLevel
// Write current file to disk
// objects in 'view' are parsed and added to current->objects[]
// current->objects will have class, and tile x/y set
//
void SaveLevel (const char *filename)
{
	FILE *stream;
	int tilex,tiley;
	objdata_t *obj;
	
	stream = fopen(filename, "wb");
	if (!stream) {
		printf("SaveLevel: Error. Could not open file %s\n",filename);
		return;
	}
	
	current.objectcount = 0; // reset so we can recount
	memset(&current.objects, 0, sizeof(current.objects));

	// parse 'view' array and add to ->objects
	for (tiley=0 ; tiley<current.height ; tiley++) {
		for (tilex=0 ; tilex<current.width ; tilex++)
		{
			if (!objview[tiley][tilex])
				continue;
			obj = &current.objects[current.objectcount];
			obj->type = objview[tiley][tilex];
			obj->tilex = tilex;
			obj->tiley = tiley;
			current.objectcount++;
		}
	}
	
	fwrite(&current, sizeof(leveldata_t), 1, stream);
	fclose(stream);
	printf("Level Save successful, %i objects\n",current.objectcount);
}




//
// LoadLevel
// Read level data from file, load editor view arrays,
// Initialize objects, returns true if load successful
// Load object linked list for gameplay
//
boolean
LoadLevel
( leveldata_t *	level,
  const char *	filename )
{
	FILE *		stream;
	int 		i;
	int			x, y;
	int			spawnx, spawny;
	objdata_t *	obj;
	tiletype_t	ttype;
	
	stream = fopen(filename, "rb");
	if (!stream) {
		printf("LoadLevel: Error. Could not open file %s\n",filename);
		return false;
	}

	fread(level, sizeof(leveldata_t), 1, stream);

	// reload in case it's already loaded
	memset(&objview, 0, sizeof(objview));
	FreeAllObjects();

	for (i=0 ; i<level->objectcount ; i++)
	{
		obj = &level->objects[i];
		
		// load editor view
		objview[obj->tiley][obj->tilex] = obj->type;
		// spawn game objects
		spawnx = obj->tilex * TILESIZE + objinfo[obj->type].width / 2;
		spawny = obj->tiley * TILESIZE + objinfo[obj->type].height / 2;
		SpawnObject(obj->type, spawnx, spawny);
	}
	
	// set up initial tile properties
	for (y=0 ; y<current.height ; y++) {
		for (x=0 ; x<current.width ; x++)
		{
			ttype = current.tilemap[y][x].type;
			tilemap[y][x].type = ttype;
			tilemap[y][x].flags = tileflags[ttype];
			tilemap[y][x].tilex = x;
			tilemap[y][x].tiley = y;
		}
	}
	
	InitLighting();
	levelloaded = true;
	UpdateOrigin();
	
	printf("Load level success - %s\n",filename);
	printf("Level number: %i\n",level->levelnum);
	printf("Numobjects: %i\n",level->objectcount);
	printf("Map Width: %i, Height: %i\n",level->width,level->height);
	fclose(stream);
	return true;
}





#pragma mark -

//
// MoveCursor
// Moves the cursor by x or y incrememnt and
// bounds to map edges. Shifts the screen
// origin for scrolling.
//
void MoveCursor (int xincr, int yincr)
{
	// don't move off screen
	if (csr.x == 0 && xincr == -1)
		xincr = 0;
	if (csr.x == current.width-1 && xincr == 1)
		xincr = 0;
	if (csr.y == 0 && yincr == -1)
		yincr = 0;
	if (csr.y == current.height-1 && yincr == 1)
		yincr = 0;
	if (!xincr && !yincr)
		return;

	csr.x += xincr;
	csr.y += yincr;
	
	// update the UL corner
	originx = csr.x*TILESIZE - MAP_W/2;
	originy = csr.y*TILESIZE - MAP_H/2;
	bound(originx, 0, current.width*TILESIZE-MAP_W);
	bound(originy, 0, current.height*TILESIZE-MAP_H);
}




//
// DoFillBox
// Space has been pressed, start a fill box if not currently dragging
// Set tiles to fill area is
//
typedef enum { startfill, tilefill, clrtilefill, objfill, clrobjfill } filltype;

void DoFillBox (filltype type)
{
	int xstep,ystep;
	int startx,starty,stopx,stopy;
	int x,y;
	
	if (type == startfill) // start dragging a box
	{
		dragging = true;
		csr.fixedx = csr.x;
		csr.fixedy = csr.y;
		printf("fixedx set to %d\n",csr.fixedx);
		return;
	}
	
	// we're dragging a box, fill:
	
	x = startx = csr.fixedx;
	y = starty = csr.fixedy;
	xstep = sign(csr.x - csr.fixedx);
	ystep = sign(csr.y - csr.fixedy);
	stopx = csr.x + xstep;
	stopy = csr.y + ystep;
	
	do
	{
		x = startx;
		do
		{
			switch (type) {
				case tilefill:
					current.tilemap[y][x].type = csr.tile;
					break;
				case objfill:
					objview[y][x] = csr.obj;
					break;
				case clrtilefill:
					current.tilemap[y][x].type = 0;
					break;
				case clrobjfill:
					objview[y][x] = 0;
				default:
					break;
			}
		} while ((x+=xstep) != stopx);
	} while ((y+=ystep) != stopy);
	
	dragging = false;
}




//
// Place Tile / Object
// Places a single tile/object or fills
// if dragging a fill box. add = false to remove
//
void PlaceTile (boolean add)
{
	tiledata_t *tile = &current.tilemap[csr.y][csr.x];
	
	if (dragging) {
		if (add) DoFillBox(tilefill);
		else	 DoFillBox(clrtilefill);
	} else {
		if (add) tile->type = (tile->type == csr.tile) ? TT_EMPTY : csr.tile;
		else	 tile->type = 0;
	}
}

void PlaceObject (boolean add)
{
	objtype_t *cell = &objview[csr.y][csr.x];
	
	if (dragging) {
		if (add) DoFillBox(objfill);
		else	 DoFillBox(clrobjfill);
	} else {
		if (add) *cell = (*cell == csr.obj) ? OT_NOTHING : csr.obj;
		else 	 *cell = 0;
	}
}





#pragma mark -

void DrawGrid ()
{
	V_SetColor(WHITE);
	for (int y=0 ; y<current.height*TILESIZE ; y+=TILESIZE)
		for (int x=0 ; x<current.width*TILESIZE ; x+=TILESIZE)
			SDL_RenderDrawPoint(renderer, x-originx, y-originy);
}

//
//	DrawMap
//	Draw tiles and game objects
//
void DrawMap ()
{
	int 			x, y;
	int				offx, offy;
	tiledata_t *	tile;
	objinfo_t  *	info;
	SDL_Rect		src, dest;
	
	for (y=0 ; y<current.height ; y++) {
		for (x=0 ; x<current.width ; x++)
		{
			tile = &current.tilemap[y][x];
			
			// draw tile if present
			if (tile->type) {
				SDL_Rect dst = { drawx(x), drawy(y), TILESIZE, TILESIZE };
				SDL_RenderCopy(renderer, tiletextures[tile->type], NULL, &dst);
			}
			
			// draw object, if present
			if (objview[y][x])
			{
				info = &objinfo[objview[y][x]];
				offx = (TILESIZE - info->width) / 2; // center obj in tile
				offy = (TILESIZE - info->height) / 2;
				
				dest = (SDL_Rect) { drawx(x)+offx,drawy(y)+offy,info->width,info->height };
				if (objview[y][x] == OT_PLAYER) {
					src = (SDL_Rect) { 8, 0, info->width, info->height };
				} else {
					src = (SDL_Rect) { 0, 0, info->width, info->height };
				}
				SDL_RenderCopy(renderer, objtextures[objview[y][x]], &src, &dest);
			}
		}
	}
}


void DrawCursor ()
{
	SDL_Rect r;
	
	if (SDL_GetTicks() % 640 > 320) // annoying blink effect!
		return;
	
	if (dragging) { // blue box over fill area
		r.x = (csr.fixedx < csr.x) ? drawx(csr.fixedx) : drawx(csr.x);
		r.y = (csr.fixedy < csr.y) ? drawy(csr.fixedy) : drawy(csr.y);
		r.w = abs((csr.x-csr.fixedx)*TILESIZE)+TILESIZE;
		r.h = abs((csr.y-csr.fixedy)*TILESIZE)+TILESIZE;
		V_SetColor(BLUE);
	} else { // regular single tile cursor
		r = (SDL_Rect){ drawx(csr.x), drawy(csr.y), TILESIZE, TILESIZE };
		V_SetColor(RED);
	}
	
	SDL_RenderDrawRect(renderer, &r);
}


void DrawStatus()
{
	SDL_Rect src, dst;
	
	SDL_SetRenderDrawColor(renderer, 32, 32, 32, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, NULL);
	
	cls();
	textcolor(YELLOW);
	printxy(0, 0, "Cursor");
	textcolor(BRIGHTWHITE);
	printxy(0, 1, "x: "); printintxy(3, 1, csr.x);
	printxy(0, 2, "y: "); printintxy(3, 2, csr.y);

	const int mapcol = 7;
	textcolor(YELLOW);
	printxy(mapcol, 0, "Map");
	textcolor(BRIGHTWHITE);
	printxy(mapcol, 1, "w: "); printintxy(mapcol+3, 1, current.width);
	printxy(mapcol, 2, "h: "); printintxy(mapcol+3, 2, current.height);
	
	const int tilecol = 13;
	textcolor(YELLOW);
	printxy(tilecol, 0, "Tile");
	dst = (SDL_Rect){ tilecol*8, 8, TILESIZE, TILESIZE };
	SDL_RenderCopy(renderer, tiletextures[csr.tile], NULL, &dst);
	
	const int objcol = 20;
	textcolor(YELLOW);
	printxy(objcol, 0, "Object");
	src = (SDL_Rect){ 0, 0, objinfo[csr.obj].width, objinfo[csr.obj].height };
	dst = (SDL_Rect){ objcol*8, 8, objinfo[csr.obj].width, objinfo[csr.obj].height };
	SDL_RenderCopy(renderer, objtextures[csr.obj], &src, &dst);
}



#pragma mark -

void EditorLoop ()
{
	SDL_Event ev;
	
	csr.obj = OT_PLAYER;
	csr.tile = 1; // whatever the first thing is
	dragging = fill = false;
	csr.x = csr.y = 0;
	MoveCursor(0, 0); // call because it sets the screen origin
	
	do
	{
		V_StartFrame();
		while(SDL_PollEvent(&ev))
		{
			if (ev.type == SDL_QUIT) {
				Quit(NULL);
			}
			
			if (ev.type == SDL_KEYDOWN)
			{
				switch (ev.key.keysym.sym)
				{
					case SDLK_ESCAPE:	// quit
						Quit(NULL);
						
					case SDLK_s: 		// save map
						SaveLevel(filepath);
						break;
						
					case SDLK_r:		// test
						puts("Testing level...");
						SaveLevel(filepath);
						LoadLevel(&current, filepath);
						gamestate = play;
						break;
						
					// move cursor
						
					case SDLK_UP: 		MoveCursor( 0, -1); break;
					case SDLK_DOWN:		MoveCursor( 0,  1); break;
					case SDLK_LEFT:		MoveCursor(-1,  0); break;
					case SDLK_RIGHT: 	MoveCursor( 1,  0); break;
						
					// place / erase / fill

					case SDLK_t: PlaceTile(true);		break;
					case SDLK_o: PlaceObject(true);		break;

					case SDLK_SPACE: // alternative to 't' or 'o'
						if (ShiftKey()) {
							PlaceTile(true);
						} else {
							PlaceObject(true);
						}
						break;
						
					case SDLK_SLASH: // start a fill box
						if (!dragging)
							DoFillBox(startfill);
						break;

						
					case SDLK_x: // remove tile / object
						if (ShiftKey()) {
							PlaceTile(false);
						} else {
							PlaceObject(false);
						}
						break;
						
					// change tile / object selection
						
					case SDLK_COMMA: // decr tile / object
						if (ShiftKey()) {
							csr.tile = (csr.tile == 1) ? NUMTILES-1 : csr.tile-1;
						} else {
							csr.obj = (csr.obj == 1) ? NUMOBJECTS-1 : csr.obj-1;
						}
						break;
						
					case SDLK_PERIOD: // incr tile / object
						if (ShiftKey()) {
							csr.tile = (csr.tile == NUMTILES-1) ? 1 : csr.tile+1;
						} else {
							csr.obj = (csr.obj == NUMOBJECTS-1) ? 1 : csr.obj+1;
						}
						break;
						
						
					default:
						break;
				}
			}
		}
		
		//
		// RENDER
		//
		
		V_Clear(BLACK);
		
		SDL_RenderSetViewport(renderer, &maprect);
		DrawGrid();
		DrawMap();
		DrawCursor();
		SDL_RenderSetViewport(renderer, &statusrect);
		DrawStatus();
		
		V_Draw();
		V_LimitFR();
	} while (gamestate == editor);
}
