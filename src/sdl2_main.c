#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>

#include "shared.h"
#include "sdl2_main.h"

SDL_Window *globWin;
SDL_Surface *globFB;
SDL_Rect globScalingRect;
SDL_Surface *globBMP, *globFnt;
SDL_Event globEvent;

static const SDL_Keycode Kmap[54]={ 0, SDLK_a, SDLK_b, SDLK_c,
	SDLK_d, SDLK_e, SDLK_f, SDLK_g, SDLK_h, SDLK_i, SDLK_j,
	SDLK_k, SDLK_l, SDLK_m, SDLK_n, SDLK_o, SDLK_p, SDLK_q,
	SDLK_r, SDLK_s, SDLK_t, SDLK_u, SDLK_v, SDLK_w, SDLK_x,
	SDLK_y, SDLK_z, SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT,
	SDLK_SPACE, SDLK_0, SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5,
	SDLK_6, SDLK_7, SDLK_8, SDLK_9, SDLK_COLON, SDLK_QUOTE,
	SDLK_COMMA, SDLK_EQUALS, SDLK_PERIOD, SDLK_SLASH, SDLK_RETURN,
	SDLK_TAB, SDLK_LALT, SDLK_DELETE, SDLK_SCROLLLOCK, SDLK_LSHIFT};

void ocistiGrafike(void) {
	SDL_FreeSurface(globFB);
	SDL_FreeSurface(globFnt);
	SDL_DestroyWindow(globWin);
	SDL_Quit();
}

void UpdateWindowScalingRules(void) {
	// prozor moze biti bilo koje velicine
	SDL_SetWindowMinimumSize(globWin, 0, 0);

	switch(scaleMode) {
		case FULL_STRECH: // puna slika
			globScalingRect.x = 0; globScalingRect.y = 0;
			globScalingRect.w = windowW; globScalingRect.h = windowH;
			break;
		case STRETCH_ASPECT: { // razvucena slika ali odnos ostaje isti
			if((float)windowW/windowH > (float)MAX_X/MAX_Y) {
				float scale = (float)windowH/MAX_Y;
				globScalingRect.x = (windowW-MAX_X*scale)/2;
				globScalingRect.y = 0;
				globScalingRect.w = MAX_X*scale;
				globScalingRect.h = windowH;
			} else {
				float scale = (float)windowW/MAX_X;
				globScalingRect.y = (windowH-MAX_Y*scale)/2;
				globScalingRect.x = 0;
				globScalingRect.h = MAX_Y*scale;
				globScalingRect.w = windowW;
			}
			break;
		}
		case CLOSEST_MULTIPLE: { // N*minVisina,N*minSirina
			if((float)windowW/windowH > (float)MAX_X/MAX_Y) {
				float scale = floor((float)windowH/MAX_Y);
				globScalingRect.x = (windowW-MAX_X*scale)/2;
				globScalingRect.y = (windowH-MAX_Y*scale)/2;
				globScalingRect.w = MAX_X*scale;
				globScalingRect.h = MAX_Y*scale;
			} else {
				float scale = floor((float)windowW/MAX_X);
				globScalingRect.x = (windowW-MAX_X*scale)/2;
				globScalingRect.y = (windowH-MAX_Y*scale)/2;
				globScalingRect.w = MAX_X*scale;
				globScalingRect.h = MAX_Y*scale;
			}

			// ogranici minimalnu velicinu da se ne polomi algoritam
			SDL_SetWindowMinimumSize(globWin, MAX_X, MAX_Y); 
			break;
		}
		case ORIGINAL_CENTER: { // originalna slika u centru
			globScalingRect.x = (windowW-MAX_X)/2;
			globScalingRect.y = (windowH-MAX_Y)/2;
			globScalingRect.w = MAX_X;
			globScalingRect.h = MAX_Y;

			// ogranici minimalnu velicinu da se ne polomi algoritam
			SDL_SetWindowMinimumSize(globWin, MAX_X, MAX_Y); 
			break;
		}
	}
}

Uint16 ocitajTastaturu(void) {
	Uint8 A;

	// Jer SDL2 voli da komplikuje, trebaju se svi eventovi
	// procesovati PRIJE NEGO STO JE UOPSTE MOGUCE CITATI STANJE
	// TASTATURE
	while(SDL_PollEvent(&globEvent)) {
		switch(globEvent.type) {
			case SDL_WINDOWEVENT:
				switch(globEvent.window.event) {
					case SDL_WINDOWEVENT_CLOSE:
						return INT_QUIT;
					case SDL_WINDOWEVENT_RESIZED:
						windowW = globEvent.window.data1;
						windowH = globEvent.window.data2;
						UpdateWindowScalingRules();
						globBMP = SDL_GetWindowSurface(globWin);
						break;
				}
				break;
		}
	}

	// Originalni allegro kod je radio tako sto uzme stanje citave tastature i prema tome
	// procjeni sta je pritisnuto a sta nije

	// Iako ovo moze biti malo stetan dizajn, ja cu ga otpratiti radi jednostavnosti
	const Uint8 *kS = SDL_GetKeyboardState(NULL); // kS == keyboardState

	// Special key time :DDDDD
	if(kS[SDL_SCANCODE_LCTRL]) {
		if(kS[SDL_SCANCODE_Q]) { // Izlaz
			return INT_QUIT;
		} else if(kS[SDL_SCANCODE_R]) {
			ResetZ80(&R); // RESET
			// Ekran
			for (Uint16 n=0x2800; n<0x2A00; n++)
				MEMORY[n]=' '+n;
		} else if(kS[SDL_SCANCODE_N]) {
			// HARD BREJK
			IntZ80(&R, INT_NMI);
		}
	}

	for (A=1; A<54; A++) {
		// Ako smo u ScreenSaver modu, vrati se u normalno stanje i
		// anuliraj upravo pritisnuti taster.

		//if (key[Kmap[A]]) {
		//	InicijalizujSSaver();
		//	if (ssaver) {
		//		// Ovaj taster kao da nismo ni pritisnuli !!!
		//		key[Kmap[A]]=NE;
		//		return;
		//	}
		//}

		// Par modifikacija, radi udobnosti.
		// Shift je Shift, pa bio on levi ili desni. I tome slicno.
		switch (Kmap[A]) {
			case SDLK_LEFT:
				MEMORY[0x2000+A]=kS[SDL_GetScancodeFromKey(Kmap[A])] ||
								kS[SDL_SCANCODE_BACKSPACE] ? 0xFE : 0xFF;
				break;
			case SDLK_LSHIFT:
				MEMORY[0x2000+A]=kS[SDL_GetScancodeFromKey(Kmap[A])] ||
								kS[SDL_SCANCODE_RSHIFT] ? 0xFE : 0xFF;
				break;
			default:
				MEMORY[0x2000+A]=kS[SDL_GetScancodeFromKey(Kmap[A])] ? 0xFE : 0xFF;
		}
	}
	return 0;
}

void obrisiEkran(void) {
	SDL_FillRect(globFB, NULL, crnaPodzadina ? CRNA : SIVA);
}

void NacrtajKarakter(Uint8 karakter, Uint8 x, Uint8 y) {
	Uint8 tmpX;
	Uint8 n, pozX, pozY;
	Uint16 pozK;

	pozX=x*SIRINA;
	pozY=(y*VISINA)+8;
	pozK=(karakter&0x7F)*VISINA*SIRINA;

	SDL_Rect src, dest;
	src.x = 0; src.y = karakter*VISINA; src.w = SIRINA; src.h = VISINA;
	dest.x = x*SIRINA; dest.y = y*VISINA; dest.w = SIRINA; dest.h = VISINA;

	SDL_FillRect(globFB, &dest, crnaPodzadina ? CRNA : SIVA);
	SDL_BlitSurface(globFnt, &src, globFB, &dest);
}


void osveziEkran(void) {
	Uint8 x,y;
	Uint16 adresa, EZPadresa;
	int offset;

	adresa=0x2800;
	EZPadresa=0;

	for (y=0; y<16; y++) {
		for (x=0; x<32; x++) {
			// TZZ se inicijalizuje u funkciji 'InicijalizujMasinu'
			// Eto, sluzi da iole ubrza stvari. (Normalno, samo u SYNC 0 modu)

			if (TZZ[MEMORY[adresa]] != EZP[EZPadresa++])
				NacrtajKarakter (TZZ[MEMORY[adresa++]],x,y);
			else
				adresa++;
		}
	}

	if (MEMORY[0x2BB0]) {
		// Obrisi prvi galaksijin red.
		offset = (MEMORY[0x2BB0]*3);
		if (offset > MAX_Y-16) {
			offset = MAX_Y-16;
		}

		SDL_Rect x = {(-88+(MEMORY[0x2BA8]*8), 8,
			-88+(MEMORY[0x2BA8]*8))+MAX_X-1,
			8+offset};
		SDL_FillRect(globFB, &x, crnaPodzadina ? CRNA : SIVA);
	}

	memcpy (EZP, &MEMORY[0x2800], 512);

	SDL_FillRect(globBMP, NULL, crnaPodzadina ? CRNA : SIVA);
	SDL_BlitScaled(globFB, NULL, globBMP, &globScalingRect);
	SDL_UpdateWindowSurface(globWin);

	// logika koja managuje frametimeove
	Uint32 noviFrame = SDL_GetTicks();
	if(noviFrame-zadnjiFrame < 1000/FrameRate)
		SDL_Delay(1000/FrameRate - noviFrame + zadnjiFrame);
	zadnjiFrame = SDL_GetTicks();
}

void inicijalizujProzor() {
	SDL_Init(SDL_INIT_EVERYTHING);
	globWin = SDL_CreateWindow("Galaksija emulator",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			MAX_X, MAX_Y, SDL_WINDOW_RESIZABLE);
	globBMP = SDL_GetWindowSurface(globWin);
	UpdateWindowScalingRules();
}

void inicijalizujGalaksiju() {
	Uint8 a, b, x;
	Uint8 karakter;
	Uint16 n,adresa;
	FILE *rom, *chrgen;

	if ((chrgen=fopen("CHRGEN.BIN","rb"))==NULL) {
		fprintf(stderr, "CHRGEN.BIN nije prisutan!\n");
		SDL_Quit();
		exit(EXIT_FAILURE);
	}

	char *bafer = malloc(2048);
	fread(bafer, 1, 2048, chrgen);

	globFB = SDL_CreateRGBSurface(0, MAX_X, MAX_Y, 32,
			0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
	globFnt = SDL_CreateRGBSurface(0, SIRINA, VISINA*BROJ_ZNAKOVA,
			32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);

	SDL_FillRect(globFB, NULL, crnaPodzadina ? CRNA : SIVA);

	// Radi ubrzavanja operacije pretvaranje bitmap fonta u SDL2 framebuffer
	// mi zahtjevamo pointer od samog SDL objekta za framebuffer
	// pa preko njega ga crtamo
	SDL_LockSurface(globFnt); 
	Uint8 *pixel = (Uint8*)globFnt->pixels;
	for (karakter=0; karakter<BROJ_ZNAKOVA; karakter++) {
		for (n=0; n<VISINA; n++) {
			adresa=n<<7;
			adresa|=karakter;

			a=1;
			b=bafer[adresa];

			for (x=0; x<SIRINA; x++) {
				if (!(b&a))
					*((Uint32*)pixel) = BELA; // nemoj me slati u pakao zbog ovoga
				pixel+=4;
				a<<=1;
			}

			// kad jebes sa framebufferom, trebas biti takodje veoma oprezan 
			// pogledaj SDL2 dokumentaciju zasto
			pixel+=globFnt->pitch-SIRINA*4;
		}
	}
	SDL_UnlockSurface(globFnt); // zavrsi framebuffer biznis

	free(bafer);
	fclose(chrgen);

	// ROM1
	if ((rom=fopen("ROM1.BIN","rb"))==NULL) {
		fprintf(stderr, "ROM1.BIN nije prisutan!\n");
		SDL_Quit();
		exit(EXIT_FAILURE);
	}

	// RAM
	if ((MEMORY=(char *)malloc(WORK_SPACE))==NULL) {
		fprintf(stderr, "Memorijski problem u __FILE__:__LINE__!\n");
		SDL_Quit();
		exit(EXIT_FAILURE);
	}
	fread (&MEMORY[0], 1, 4096, rom);
	fclose (rom);

	// ROM2
	if((rom=fopen("ROM2.BIN", "rb"))!=NULL) {
		fread(&MEMORY[0x1000], 1, 4096, rom);
		fclose(rom);
	} else {
		printf("ROM2.BIN nije prisutan, idemo dalje bez njega!\n");
		memset(&MEMORY[0x1000], 0xff, 4096);
	}

	// GAL_PLUS
	if((rom=fopen("GAL_PLUS.BIN", "rb"))!=NULL) {
		fread(&MEMORY[0xE000], 1, 4096, rom);
		fclose(rom);
	} else {
		printf("GAL_PLUS.BIN nije prisutan, idemo dalje bez njega!\n");
		memset(&MEMORY[0xE000], 0xff, 4096);
	}

	// Tastatura
	for (n=0x2000; n<0x2800; n++)
		MEMORY[n]=0xFF;
	
	// Ekran
	for (n=0x2800; n<0x2A00; n++)
		MEMORY[n]=' '+n;
	
	// 6116-ice
	for (n=0x2A00; n<KRAJ_RAMA; n++)
		MEMORY[n]=0;

	// TZZ je iskoriscen da bi se izbegla provera unutar glavne petlje za ispis.
	TZZ=malloc(256);
	
	for (n=0; n<256; n++) {
		if ((n>63 && n<96) || (n>127 && n < 192)) {
			TZZ[n]=n-64;
		} else if (n > 191) {
			TZZ[n]=n-128;
		} else {
			TZZ[n]=n;
		}
	}

	Fassst=0;
	memset (EZP, 0, 512);

	//InitUnos=1;

	//F11_hold=NE;

	//ShowFPS=0;
	//Cur_fps=Disp_fps=0;
	//HorPos=0;
}
