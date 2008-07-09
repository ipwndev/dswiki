#ifndef APIH
#define APIH

#include <PA9.h>
#include "struct.h"

u8    IsInArea(BLOCK BLK, POINT pt);
BLOCK CreateBlock(s32 xoff, s32 yoff, s32 w, s32 h);
BLOCK Intersection(BLOCK BLK1,BLOCK BLK2);
void  DrawPoint(VirScreen* Graphic,s32 X,s32 Y, u16 Color);
void  DrawBlock(VirScreen* Graphic, BLOCK Area, u16 Color, u8 Fill);
void  DrawEmboss(VirScreen* Graphic, BLOCK Area, u16 Color);
void  DrawGroove(VirScreen* Graphic, BLOCK Area, u16 Color);
void  InitVS(VirScreen* VS);
void  InitVS2(VirScreen* VS);

#endif

