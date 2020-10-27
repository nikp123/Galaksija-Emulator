#include <stdio.h>
#include <stdlib.h>

#include "shared.h"
#include "Z80/Z80.h"

#include "sdl2_main.h"

typedef union {
 #ifdef LSB_FIRST
   struct { Uint8 l,h,h2,h3; } B;
   struct { Uint16 l,h; } W;
   Uint32 D;
 #else
   struct { Uint8 h3,h2,h,l; } B;
   struct { Uint16 h,l; } W;
   Uint32 D;
 #endif
} pairDOS;

typedef struct {
  pairDOS AF,BC,DE,HL,IX,IY,PC,SP;
  pairDOS AF2,BC2,DE2,HL2;
  unsigned IFF1,IFF2,HALT,IM,I,R,R2;
} Z80_RegsDOS;
Z80_RegsDOS DOS_R;

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

Uint16 LoopZ80(register Z80 *R) {
	char bafer[128];

	// Reset the ICount so that next interrupt happens 
	R->ICount=CPU_SPEED / FrameRate; // to ensure that all operations will be done

	// Da li treba da se pomera slika ?
	if (HorPos!=MEMORY[0x2BA8]) {
		// Crni ekran
		obrisiEkran();
		HorPos=MEMORY[0x2BA8];
	}

	// Broji frejmove.
	//Cur_fps++;

	// Usluzi SAVE/LOAD opcije.
	//LoadSave (KEY_F2, Save, SAVE);
	//LoadSave (KEY_F3, Load, LOAD);

	//Z80_GetRegs (&R);        // Daj bre te registre ovamo ...

	if (!(R->IFF & IFF_2))
		Fassst++;
	else
		Fassst=0;

	// Ako je EI osvezavaj i ekran i tastaturu, kao i kod prave masine.
	if (!Fassst) {
		RET_ON_ERR(ocitajTastaturu());
		osveziEkran();
	} else {
		switch (Fassst) {
			// Because screen is made under IRQ, there is no more further screen updates.
			case 1:
				// Need to clear it ?
				obrisiEkran();
				break;

			// Stay where You are.
			case 2:
				Fassst--;
				break;
		}
	}
	return 0;
}

Uint8 ucitajStanje(char *fajl) {
	FILE *f;

	if((f=fopen(fajl, "rb"))==NULL) {
		fprintf(stderr, "Nije moguce ucitati __FILE__:__LINE__ %s\n");
		return 1;
	}

	// DOS savestate
	if(fileSize(f) == 8268) {
		Z80_RegsDOS DOS_R;
		fread(&DOS_R, 1, sizeof(Z80_RegsDOS), f);
		fread(&MEMORY[0x2000], 1, WORK_SPACE-0x2000, f);
		// Convert registers Now !
		R.AF.W = DOS_R.AF.W.l;
		R.BC.W = DOS_R.BC.W.l;
		R.DE.W = DOS_R.DE.W.l;
		R.HL.W = DOS_R.HL.W.l;
		R.IX.W = DOS_R.IX.W.l;
		R.IY.W = DOS_R.IY.W.l;
		R.PC.W = DOS_R.PC.W.l;
		R.SP.W = DOS_R.SP.W.l;
		R.AF1.W = DOS_R.AF2.W.l;
		R.BC1.W = DOS_R.BC2.W.l;
		R.DE1.W = DOS_R.DE2.W.l;
		R.HL1.W = DOS_R.HL2.W.l;
		R.IFF = 0;
		if (DOS_R.IM == 1) R.IFF+=2;
		if (DOS_R.IM == 2) R.IFF+=4;
		if (DOS_R.IFF1) R.IFF+=1;
		R.I = DOS_R.I;
		R.R = (DOS_R.R&127)|(DOS_R.R2&128);
	} else { // normal "modern" savestate
		fread(&R, 1, sizeof(R), f);
		fread(&MEMORY[0x2000], 1, WORK_SPACE-0x2000, f);
	}
	fclose(f);
	return 0;
}

Uint8 sacuvajStanje(const char *fajl) {
	FILE *f;

	if((f=fopen(fajl, "wb"))==NULL) {
		fprintf(stderr, "Ne mogu da snimim __FILE__:__LINE__ %s\n",
			fajl);
		return 1;
	}

	fwrite(&R, 1, sizeof(R), f);
	fwrite(&MEMORY[0x2000], 1, WORK_SPACE-0x2000, f);
	fclose(f);
	return 0;
}
