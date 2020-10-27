#include "shared.h"
#include "Z80/Z80.h"

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

#include "sdl2_main.h"

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
