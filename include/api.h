#ifndef _API_H
#define _API_H

#include <PA9.h>
#include "struct.h"

u8 IsInArea(BLOCK Area, POINT Point);
BLOCK CreateBlock(s32 xoff, s32 yoff, s32 w, s32 h);
BLOCK Intersection(BLOCK Area1,BLOCK Area2);

void DrawPoint (VirScreen* VScreen, s32 X, s32 Y, u16 Color);
void DrawBlock (VirScreen* VScreen, BLOCK Area, u16 Color, u8 Fill);
void DrawEmboss(VirScreen* VScreen, BLOCK Area, u16 Color);
void DrawGroove(VirScreen* VScreen, BLOCK Area, u16 Color);
void FillVS (VirScreen* VScreen, u16 Color);

void InitVS (VirScreen* VScreen);
void InitVS2(VirScreen* VScreen);

#endif

