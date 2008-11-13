#ifndef _STRUCT_H
#define _STRUCT_H

#include <PA9.h>

using namespace std;

typedef struct
{
   int x;
   int y;
} POINT;

typedef struct
{
   POINT Start;
   POINT End;
} BLOCK;

typedef struct
{
	unsigned char   Name[16];
	unsigned char   DID;
	unsigned short int* Ptr;
	unsigned short int  Width;
	unsigned short int  Height;
} Device;

typedef struct
{
	int     Left;
	int     Top;
	unsigned int     Width;
	unsigned int     Height;
	BLOCK   Bound;
	const Device* Screen;
} VirScreen;

#endif
