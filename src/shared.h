#include <SDL2/SDL.h>
#include "Z80/Z80.h"

// MAKROI
#define RET_ON_ERR(x) { Uint16 e = x; if(e) return e; }

// BOJE
#define CRNA 0x00000000
#define SIVA 0x80808080
#define BELA 0xffffffff

// SISTEMSKE KONSTANTE, ADRESE I OSTALE GLUPOSTI
#define CPU_SPEED 3072000 // 6.144/2 MHz
#define WORK_SPACE 0x4000
#define KRAJ_RAMA 0x4000

// SPECIFIKACIJE
#define MAX_X 256 // 32 karaktera
#define MAX_Y 216 // 8*16 

#define VISINA 13
#define SIRINA 8
#define BROJ_ZNAKOVA 128

// METODI RAZLACENJA SLIKE
#define CLOSEST_MULTIPLE 1
#define STRETCH_ASPECT   2
#define FULL_STRECH      3
#define ORIGINAL_CENTER  4

extern const SDL_Keycode Kmap[54];

// Globalne variable
extern Uint8 Fassst;	// ???????
extern Uint8 *TZZ;		// TabelaZameneZnakova
extern Uint8 EZP[512];	// Ekran Za Porediti
extern char *MEMORY;
extern int Z80_IRQ;		// Current IRQ status. Checked after EI occurs.
extern int HorPos;

extern int windowW, windowH;

// Globalni objekti za render i prozor
extern SDL_Window *globWin;
extern SDL_Surface *globFB;
extern SDL_Rect globScalingRect;
extern SDL_Surface *globBMP, *globFnt;
extern SDL_Event globEvent;
extern Z80 R;

extern Uint32 zadnjiFrame; // sluzi za spasavanje proc od pozara

// OPCIJE ZA KORISNIKA
extern _Bool crnaPodzadina;
extern Uint32 FrameRate;
extern Uint8 scaleMode;

