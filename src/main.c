#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "Z80/Z80.h"

// BOJE
#define CRNA 0x00000000
#define SIVA 0x80808080
#define BELA 0xffffffff

// SISTEMSKE KONSTANTE, ADRESE I OSTALE GLUPOSTI
#define CPU_SPEED 3072000 // 6.144/2 MHz
#define WORK_SPACE 0x4000
#define KRAJ_RAMA 0x4000

static const int MAX_X = 256; // 32 karaktera
static const int MAX_Y = 224; // 8*16+8 
							  // Prva i posljednja linija od po 8 ekranskih su statusne

static const int VISINA = 13;
static const int SIRINA = 8;
static const int BROJ_ZNAKOVA = 128;

static SDL_Keycode Kmap[54]={ 0, SDLK_a, SDLK_b, SDLK_c, SDLK_d, SDLK_e, SDLK_f, SDLK_g, SDLK_h,
				 SDLK_i, SDLK_j, SDLK_k, SDLK_l, SDLK_m, SDLK_n, SDLK_o, SDLK_p,
				 SDLK_q, SDLK_r, SDLK_s, SDLK_t, SDLK_u, SDLK_v, SDLK_w, SDLK_x,
				 SDLK_y, SDLK_z, SDLK_UP,SDLK_DOWN,SDLK_LEFT,SDLK_RIGHT,SDLK_SPACE,
				 SDLK_0, SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5, SDLK_6, SDLK_7,
				 SDLK_8, SDLK_9, SDLK_COLON, SDLK_QUOTE, SDLK_COMMA, SDLK_EQUALS, 
				 SDLK_STOP, SDLK_SLASH, SDLK_RETURN, SDLK_TAB, SDLK_LALT, SDLK_DELETE,
				 SDLK_SCROLLLOCK, SDLK_LSHIFT};


char *bafer; // ?
Uint8 Fassst; // ???????
Uint8    *TZZ;       // TabelaZameneZnakova
Uint8    EZP[512];   // Ekran Za Porediti
char *MEMORY;
int Z80_IRQ;                    // Current IRQ status. Checked after EI occurs.
int HorPos;

// Globalni objekti za render i prozor
SDL_Window *globWin;
SDL_Surface *globBMP, *globFnt;
SDL_Event globEvent;
Z80 R;

Uint32 zadnjiFrame = 0; // sluzi za spasavanje proc od pozara

// OPCIJE ZA KORISNIKA
_Bool CrnaPodzadina = 1;
Uint32 FrameRate = 50;
Uint16 S_gfx_X=320;
Uint16 S_gfx_Y=240;

// implicit
void OsveziEkran (void);
void OcitajTastaturu (void);
void ocistiSve (void);

// Z80 emulator funkcije
// Vidi Z80.H za dodatne informacije!

/*
byte Z80_In (byte Port) {
    return (0xFF);
}

void Z80_Out (byte Port,byte Value) {
}
*/

void OutZ80(register Uint16 Port,register Uint8 Value) {
}

Uint8 InZ80(register Uint16 Port) {
}

// Svako citanje iz memorije emulatora se obavlja ovde. Funkcija mora da bude
// uradjena bas u minimalistickom maniru. Ukoliko se iole poveca, drasticno
// se usporava emulacija !!!

/*
unsigned Z80_RDMEM(dword A) {
    // More citaj bre odakle god 'oces.
    return (MEMORY[A&0xFFFF]);
}


// Isto kao i za prethodnu funkciju s' tim sto sam ovde ipak morao da uradim
// jos nesto, da proverim da li je upis u RAM podrucju.

void Z80_WRMEM(dword A,byte V)
{
    // 0x0000 - 0x1FFF je ROM.
    // 0x2000 - 0x2800 su tastatura i kasetofon.

    if (A>=0x2800 && A<KRAJ_RAMA)
        MEMORY[A]=V;
}
*/

Uint8 RdZ80 (register Uint16 Addr) {
    return (MEMORY[Addr&0xFFFF]);
}

void WrZ80 (register Uint16 Addr,register Uint8 Value) {
    // 0x0000 - 0x1FFF je ROM.
    // 0x2000 - 0x2800 su tastatura i kasetofon.

    if (Addr>=0x2800 && Addr<KRAJ_RAMA)
        MEMORY[Addr]=Value;
}

void PatchZ80(register Z80 *R) {
}

Uint8 DebugZ80(register Z80 *R) {
}

//ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ
//Û              Û
//Û Z80 INTERAPT Û
//Û              Û
//ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ

// Nakon CPU_SPEED/fps T-ciklusa, jezgro Z80 emulatora proziva ovu funkciju.
// Tada se odradi sve sto je neophodno za emulator, a onda se izvrsi ona
// prava Z80 interapt rutina iz ROM-a.
// Dakle osvezi se ekran, skanira tastatura, obradi screen saver, ispituju
// kontrolni tasteri (Izlazak, Reset, NMI, Status, LOAD/SAVE), meri stvarni
// FPS ukoliko se ne radi o SYNC 1 modu, i sve ostalo sto je potrebno.

word LoopZ80(register Z80 *R)
{
	char    bafer[128];

	// Reset the ICount so that next interrupt happens 
	R->ICount=CPU_SPEED / FrameRate; // to ensure that all operations will be done

    // Da li treba da se pomera slika ?
	if (HorPos!=MEMORY[0x2BA8]) {
		// Crni ekran
		SDL_FillRect(globBMP, NULL, CRNA);
		HorPos=MEMORY[0x2BA8];
	}

	//if (Sekunda)
	//{
	//	ScreenSaverCounter++;
	//	if (ScreenSaverCounter>S_ssaver)
	//		ssaver=DA;

	//	Sekunda=0;
	//	Disp_fps=Cur_fps;
	//	Cur_fps=0;
	//}

	// Ako treba ispisi statusnu liniju.
	//if (ShowFPS)
	//{
	//    text_mode(-1);

	//    ObrisiGornjiDeoEkrana();

	//    sprintf (bafer, "FPS                CLK          ");
	//    textout (ekran, font, bafer, 0, 0, 7);

	//    sprintf (bafer, "%d", Disp_fps);
	//    textout (ekran, font, bafer, 32, 0, 6);

	//      sprintf (bafer, "%d", Disp_fps*(R->ICount));
	//    sprintf (bafer, "%d", Disp_fps*(R->ICount));
	//    textout (ekran, font, bafer, 184, 0, 6);

	//    OsveziPrviRed ();
	//}

	// Da li je screen saver aktivan ?
	//if (ssaver)
	//    FadeInOut();

	// Broji frejmove.
	//Cur_fps++;

	// Usluzi SAVE/LOAD opcije.
	//LoadSave (KEY_F2, Save, SAVE);
	//LoadSave (KEY_F3, Load, LOAD);

	//InitUnos=1;

    // Da prikazem/uklonim statusnu liniju ?

    //if (key[KEY_F11])
    //{
    //    if (!F11_hold)
    //    {
    //        F11_hold=DA;
    //        InicijalizujSSaver();

    //        if (ShowFPS)
    //        {
    //            ShowFPS=0;
    //            ObrisiGornjiDeoEkrana();
    //            OsveziPrviRed ();
    //        }
    //        else
    //            ShowFPS=1;
    //    }
    //}
    //else
    //    F11_hold=NE;

//    Z80_GetRegs (&R);        // Daj bre te registre ovamo ...

	if (!(R->IFF & IFF_2))
		Fassst++;
	else
		Fassst=0;

//        OsveziEkran ();
    // Ako je EI osvezavaj i ekran i tastaturu, kao i kod prave masine.
	if (!Fassst) {
		OcitajTastaturu ();
		OsveziEkran ();
	} else {
		switch (Fassst) {
			// Because screen is made under IRQ, there is no more further screen updates.
			case 1:
				// Need to clear it ?
				SDL_FillRect(globBMP, NULL, CRNA);
				break;

			// Stay where You are.
			case 2:
				Fassst--;
				break;
		}
	}
	return 0;
}


void OcitajTastaturu (void) {
	Uint8 A;

	// Jer SDL2 voli da komplikuje, trebaju se svi eventovi procesovati PRIJE
	// NEGO STO JE UOPSTE MOGUCE CITATI STANJE TASTATURE
	while(SDL_PollEvent(&globEvent)) {
		switch(globEvent.type) {
			case SDL_WINDOWEVENT:
				switch(globEvent.window.event) {
					case SDL_WINDOWEVENT_CLOSE:
						ocistiSve();
						exit(EXIT_SUCCESS);
						break;
					case SDL_WINDOWEVENT_RESIZED:
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
			ocistiSve();
			exit(EXIT_SUCCESS);
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

	SDL_FillRect(globBMP, &dest, CRNA);
	SDL_BlitSurface(globFnt, &src, globBMP, &dest);
}


void OsveziEkran(void)
{
	Uint8    x,y;
	Uint16    adresa,EZPadresa;
	int barva;
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
		barva = SIVA;
		if (CrnaPodzadina) {
			barva = CRNA;
		}
		offset = (MEMORY[0x2BB0]*3);
		if (offset > MAX_Y-16) {
			offset = MAX_Y-16;
		}

		SDL_Rect x = {((S_gfx_X-MAX_X)/2)-88+(MEMORY[0x2BA8]*8),          ((S_gfx_Y-MAX_Y)/2)+8,
		             (((S_gfx_X-MAX_X)/2)-88+(MEMORY[0x2BA8]*8))+MAX_X-1, ((S_gfx_Y-MAX_Y)/2)+8+offset};
		SDL_FillRect(globBMP, &x, barva);
	}

	memcpy (EZP, &MEMORY[0x2800], 512);
	SDL_UpdateWindowSurface(globWin);

	// logika koja managuje frametimeove
	Uint32 noviFrame = SDL_GetTicks();
	if(noviFrame-zadnjiFrame < 1000/FrameRate)
		SDL_Delay(1000/FrameRate - noviFrame + zadnjiFrame);
	zadnjiFrame = SDL_GetTicks();
}

void inicijalizujProzor() {
	SDL_Init(SDL_INIT_EVERYTHING);
	globWin = SDL_CreateWindow("Galaksija emulator", SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, MAX_X, MAX_Y, 0);
	globBMP = SDL_GetWindowSurface(globWin);
}

void inicijalizujGalaksiju() {
	Uint8 a, b, x;
	Uint8 karakter;
	Uint16 n,adresa;
	FILE *rom, *chrgen;

	if ((chrgen=fopen("chrgen.bin","rb"))==NULL) {
		fprintf(stderr, "CHRGEN.BIN nije prisutan!\n");
		SDL_Quit();
		exit(EXIT_FAILURE);
	}

	bafer = malloc(2048);
	fread(bafer, 1, 2048, chrgen);

	globFnt = SDL_CreateRGBSurface(0, SIRINA, VISINA*BROJ_ZNAKOVA,
			32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);

	if(CrnaPodzadina)
		SDL_FillRect(globBMP, NULL, CRNA);
	else SDL_FillRect(globBMP, NULL, SIVA);

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

	free (bafer);
	fclose (chrgen);

	if ((rom=fopen("rom1.bin","rb"))==NULL) {
		fprintf(stderr, "ROM1.BIN nije prisutan!\n");
		SDL_Quit();
		exit(EXIT_FAILURE);
	}

	if ((MEMORY=(char *)malloc(WORK_SPACE))==NULL) {
		fprintf(stderr, "Memorijski problem u __FILE__:__LINE__!\n");
		SDL_Quit();
		exit(EXIT_FAILURE);
	}
	fread (&MEMORY[0], 1, 4096, rom);
	fclose (rom);


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

void StartujMasinu (void) {
	ResetZ80(&R); 
	R.ICount=CPU_SPEED / FrameRate;
	RunZ80(&R);
}

void ocistiSve() {
	SDL_DestroyWindow(globWin);
}

int main(int argc, char *argv[]) {
	inicijalizujProzor();
	inicijalizujGalaksiju();
	StartujMasinu();
	ocistiSve();
	return 0;
}
