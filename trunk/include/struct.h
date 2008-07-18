#ifndef STRUCTH
#define STRUCTH

#include <PA9.h>

typedef struct
{
   s32 x;
   s32 y;
} POINT;

typedef struct
{
   POINT Start;
   POINT End;
} BLOCK;

typedef struct
{
	u8   Name[16];
	u8   DID;
	u16* Ptr;
	u16  Width;
	u16  Height;
} Device;

typedef struct
{
	s32     Left;
	s32     Top;
	u32     Width;
	u32     Height;
	BLOCK   Bound;
	Device* Screen;
} VirScreen;

#endif
