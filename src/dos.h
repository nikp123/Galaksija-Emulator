#ifndef GALAXY_DOS_H_
#define GALAXY_DOS_H_

#include <stdint.h>

typedef union {
 #ifdef LSB_FIRST
   struct { uint8_t l,h,h2,h3; } B;
   struct { uint16_t l,h; } W;
   Uint32 D;
 #else
   struct { uint8_t h3,h2,h,l; } B;
   struct { uint16_t h,l; } W;
   uint32_t D;
 #endif
} pairDOS;

typedef struct {
  pairDOS AF,BC,DE,HL,IX,IY,PC,SP;
  pairDOS AF2,BC2,DE2,HL2;
  unsigned IFF1,IFF2,HALT,IM,I,R,R2;
} Z80_RegsDOS;
Z80_RegsDOS DOS_R;

#endif

