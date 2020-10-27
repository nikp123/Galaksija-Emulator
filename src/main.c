#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shared.h"
#include "sdl2_main.h"

void startujMasinu (void) {
	ResetZ80(&R); 
	R.ICount=CPU_SPEED / FrameRate;
	RunZ80(&R);
}

void ocistiSve() {
	ocistiGrafike();
	free(MEMORY);
	free(TZZ);
}

int main(int argc, char *argv[]) {
	inicijalizujProzor();
	inicijalizujGalaksiju();
	startujMasinu();
	ocistiSve();
	return 0;
}
