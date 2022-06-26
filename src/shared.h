#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "Z80/Z80.h"

#define Uint8 uint8_t
#define Uint16 uint16_t
#define Uint32 uint32_t

// MAKROI
#define RET_ON_ERR(x) { Uint16 e = x; if(e) return e; }

// BOJE
#define CRNA 0xff000000
#define SIVA 0xff003535
#define BELA 0xffffffff

// SISTEMSKE KONSTANTE, ADRESE I OSTALE GLUPOSTI
#define CPU_SPEED 3072000 // 6.144/2 MHz
#define WORK_SPACE 0xFFFF
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

// Globalne variable
extern uint8_t Fassst;	// ???????
extern uint8_t *TZZ;		// TabelaZameneZnakova
extern uint8_t EZP[512];	// Ekran Za Porediti
extern uint8_t *MEMORY;
extern int32_t Z80_IRQ;		// Current IRQ status. Checked after EI occurs.
extern int32_t HorPos;

extern int32_t windowW, windowH;

// Globalni objekti za render i prozor
extern Z80 R;

extern uint32_t zadnjiFrame; // sluzi za spasavanje proc od pozara

// OPCIJE ZA KORISNIKA
extern bool crnaPodzadina;
extern uint32_t FrameRate;
extern uint8_t scaleMode;

// utility functions that are shared within the entire codebase
extern size_t file_size(FILE *fp);
extern size_t little_endian_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
extern size_t endian_swap_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
extern size_t little_endian_fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
extern size_t endian_swap_fread(const void *ptr, size_t size, size_t nmemb, FILE *stream);


