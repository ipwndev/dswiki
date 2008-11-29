#ifndef _STRUCT_H
#define _STRUCT_H

#include <PA9.h>

using namespace std;

// a physical device (lower/upper screen)
typedef struct
{
	unsigned char       Name[16];
	unsigned char       DID;
	unsigned short int* Ptr;
	unsigned short int  Width;
	unsigned short int  Height;
} Device;

// some basic structures
typedef struct
{
   int x;
   int y;
} POINT;

// a BLOCK is alway _RELATIVE_ to to virtual screen
typedef struct
{
   POINT Start;
   POINT End;
   void clear() {Start.x=0; Start.y=0; End.x=0; End.y=0;};
} BLOCK;

// a VIRTUAL SCREEN has to be _ABSOLUTE_ to a given device
typedef struct
{
	int           Left;
	int           Top;
	unsigned int  Width;
	unsigned int  Height;
	BLOCK         AbsoluteBound;
	const Device* Screen;
	BLOCK         RelativeBound(int shrink = 0);
	BLOCK         AbsoluteBlock(BLOCK b);
} VirScreen;

BLOCK VirScreen::RelativeBound(int shrink)
{
	return (BLOCK){{0-shrink,0-shrink},{Width-1+shrink,Height-1+shrink}};
};

BLOCK VirScreen::AbsoluteBlock(BLOCK b)
{
	return (BLOCK){{Left+b.Start.x,Top+b.Start.y},{Left+b.End.x,Top+b.End.y}};
}

#endif
