#include <stdio.h>
#include "shared.h"

const SDL_Keycode Kmap[54]={ 0, SDLK_a, SDLK_b, SDLK_c,
	SDLK_d, SDLK_e, SDLK_f, SDLK_g, SDLK_h, SDLK_i, SDLK_j,
	SDLK_k, SDLK_l, SDLK_m, SDLK_n, SDLK_o, SDLK_p, SDLK_q,
	SDLK_r, SDLK_s, SDLK_t, SDLK_u, SDLK_v, SDLK_w, SDLK_x,
	SDLK_y, SDLK_z, SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT,
	SDLK_SPACE, SDLK_0, SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5,
	SDLK_6, SDLK_7, SDLK_8, SDLK_9, SDLK_COLON, SDLK_QUOTE,
	SDLK_COMMA, SDLK_EQUALS, SDLK_PERIOD, SDLK_SLASH, SDLK_RETURN,
	SDLK_TAB, SDLK_LALT, SDLK_DELETE, SDLK_SCROLLLOCK, SDLK_LSHIFT};

// Globalne variable
Uint8 Fassst;
Uint8 *TZZ;
Uint8 EZP[512];
char *MEMORY;
int Z80_IRQ;
int HorPos;

int windowW = MAX_X, windowH = MAX_Y;

SDL_Window *globWin;
SDL_Surface *globFB;
SDL_Rect globScalingRect;
SDL_Surface *globBMP, *globFnt;
SDL_Event globEvent;
Z80 R;

Uint32 zadnjiFrame = 0;

_Bool crnaPodzadina = 1;
Uint32 FrameRate = 50;
Uint8 scaleMode = CLOSEST_MULTIPLE;

