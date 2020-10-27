#include <stdio.h>
#include "shared.h"

// Globalne variable
Uint8 Fassst;
Uint8 *TZZ;
Uint8 EZP[512];
char *MEMORY;
int Z80_IRQ;
int HorPos;

int windowW = MAX_X, windowH = MAX_Y;
Z80 R;

Uint32 zadnjiFrame = 0;

_Bool crnaPodzadina = 1;
Uint32 FrameRate = 50;
Uint8 scaleMode = CLOSEST_MULTIPLE;

