//
//  video.h
//  invaders
//
//  Created by Thomas Foster on 12/7/18.
//  Copyright © 2018 Thomas Foster. All rights reserved.
//

#ifndef video_h
#define video_h

#include <SDL2/SDL.h>

typedef struct
{
	int width, height;
	int rows, cols;
	int	cursorx, cursory;
	int	color;
} videoinfo_t;

extern videoinfo_t vinfo;

enum // DOS Colors
{
	BLACK,
	BLUE,
	GREEN,
	CYAN,
	RED,
	MAGENTA,
	BROWN,
	WHITE,
	GRAY,
	LIGHTBLUE,
	LIGHTGREEN,
	LIGHTCYAN,
	LIGHTRED,
	LIGHTMAGENTA,
	YELLOW,
	BRIGHTWHITE,
	BLINK
};

typedef struct
{
	int  w, h;
	char *data;
} graphic_t;

typedef struct
{
	SDL_Texture	*sdltx;
	int	w, h;
} texture_t;

extern const uint8_t normal_font_data[];
extern char 			font8x8_basic[128][8];
extern SDL_Renderer 	*renderer;

extern float dt;

void V_StartFrame (void);
void V_LimitFR (unsigned fps);

void V_InitVideo (int screenw, int screenh, int scale, const char *winname);
void V_ShutDown (void);
texture_t V_LoadTexture (const char *file);

void V_Clear (int color);
void V_RenderTexture (texture_t *tx, int x, int y);
void V_Draw (void);
void V_SetColor (int c);

void V_RenderGraphic (graphic_t *g, int x, int y);
void V_RenderGraphicC (graphic_t *g, int x, int y);

void textcolor (int c);
void gotoxy(unsigned x, unsigned y);
void print (char *string);
void printxy (unsigned x, unsigned y, char *string);
void printintxy (unsigned x, unsigned y, int i);
void cls (void);

#endif /* video_h */
