#include <stdio.h>
#include <stdlib.h>
#include "shared.h"

// Globalne variable
uint8_t Fassst;	// ???????
uint8_t *TZZ;		// TabelaZameneZnakova
uint8_t EZP[512];	// Ekran Za Porediti
uint8_t *MEMORY;
int32_t Z80_IRQ;		// Current IRQ status. Checked after EI occurs.
int32_t HorPos;

int32_t windowW = MAX_X, windowH = MAX_Y;

// Globalni objekti za render i prozor
Z80 R;

uint32_t zadnjiFrame = 0; // sluzi za spasavanje proc od pozara

// OPCIJE ZA KORISNIKA
bool crnaPodzadina = 0;
uint32_t FrameRate = 50;
uint8_t scaleMode = CLOSEST_MULTIPLE;

size_t file_size(FILE *fp) {
	size_t wasPosition = ftell(fp); // back up address
	fseek(fp, 0, SEEK_END);
	size_t maxPosition = ftell(fp);
	fseek(fp, wasPosition, SEEK_SET); // restore
	return maxPosition;
}

size_t endian_swap_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
	unsigned char *buffer_src = (unsigned char*)ptr;
	unsigned char *buffer_dst = (unsigned char*)calloc(nmemb, sizeof(unsigned char*)*size);
	for (size_t i = 0; i < nmemb; i++) {
		for (size_t ix = 0; ix < size; ix++) {
			buffer_dst[size * i + (size - 1 - ix)] = buffer_src[size * i + ix];
		}
	}
	size_t result = fwrite(buffer_dst, size, nmemb, stream);
	free(buffer_dst);
	return result;
}

size_t endian_swap_fread(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
	unsigned char *buffer_src = (unsigned char*)calloc(nmemb, sizeof(unsigned char*)*size);
	size_t result = fread(buffer_src, size, nmemb, stream);
	unsigned char *buffer_dst = (unsigned char*)ptr;
	for (size_t i = 0; i < nmemb; i++) {
		for (size_t ix = 0; ix < size; ix++) {
			buffer_dst[size * i + (size - 1 - ix)] = buffer_src[size * i + ix];
		}
	}
	free(buffer_src);
	return result;
}

size_t little_endian_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
	#if defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN || \
		defined(__BIG_ENDIAN__) || \
		defined(__ARMEB__) || \
		defined(__THUMBEB__) || \
		defined(__AARCH64EB__) || \
		defined(_MIBSEB) || defined(__MIBSEB) || defined(__MIBSEB__)
		return endian_swap_fwrite(ptr, size, nmemb, stream);
	#elif defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN || \
		defined(__LITTLE_ENDIAN__) || \
		defined(__ARMEL__) || \
		defined(__THUMBEL__) || \
		defined(__AARCH64EL__) || \
		defined(_MIPSEL) || defined(__MIPSEL) || defined(__MIPSEL__)
		// It's a little-endian target architecture
		return fwrite(ptr, size, nmemb, stream);
	#else
	#error "I don't know what architecture this is!"
	#endif
}

size_t little_endian_fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	#if defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN || \
		defined(__BIG_ENDIAN__) || \
		defined(__ARMEB__) || \
		defined(__THUMBEB__) || \
		defined(__AARCH64EB__) || \
		defined(_MIBSEB) || defined(__MIBSEB) || defined(__MIBSEB__)
		return endian_swap_fread(ptr, size, nmemb, stream);
	#elif defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN || \
		defined(__LITTLE_ENDIAN__) || \
		defined(__ARMEL__) || \
		defined(__THUMBEL__) || \
		defined(__AARCH64EL__) || \
		defined(_MIPSEL) || defined(__MIPSEL) || defined(__MIPSEL__)
		// It's a little-endian target architecture
		return fread(ptr, size, nmemb, stream);
	#else
	#error "I don't know what architecture this is!"
	#endif
}

