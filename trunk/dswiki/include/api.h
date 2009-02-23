#ifndef _API_H
#define _API_H

#include <PA9.h>
#include "struct.h"

using namespace std;

bool IsInArea(BLOCK Area, POINT Point);
BLOCK CreateBlock(int xoff, int yoff, int w, int h);
BLOCK Intersection(BLOCK Area1,BLOCK Area2);

void DrawPoint (const VirScreen* VScreen, int X, int Y, unsigned short int Color);
void DrawBlock (const VirScreen* VScreen, BLOCK Area, unsigned short int Color, bool Fill);
void DrawEmboss(const VirScreen* VScreen, BLOCK Area, unsigned short int Color);
void DrawGroove(const VirScreen* VScreen, BLOCK Area, unsigned short int Color);
void FillVS (VirScreen* VScreen, unsigned short int Color);

void InitVS (VirScreen* VScreen);
void InitVS2(VirScreen* VScreen);

#endif

