#ifndef CHRLIBH
#define CHRLIBH

#include <PA9.h>

#include "struct.h"
#include "Big52Uni16.h"
#include "api.h"

#include "ter12r.h"

typedef enum {UTF,UTF8,BIG5,GBK,JIS} Lid;
typedef enum {NORMALWRAP,HARDWRAP,NOWRAP} ChrWrap;
typedef enum {DEG0,DEG90,DEG180,DEG270} ChrRot;
typedef enum {NONE,HOLLOW,SHADOW,BACKGR} FX;

typedef struct
{
  u16*      Name;
  u8        Height;
  u32*      Index;
  u8*       Data;
  const u8* Ptr;
} Font;

typedef struct
{
	u16     Color;
	u16     BgColor;
	ChrWrap Wrap;
	ChrRot  Rotate;
	FX      Fx;
	u8      CutChar;
	u8      W_Space;
	u8      H_Space;
	Font*   FONT;
} CharStat;

u8   InitFont(Font* FONT, const u8* ptr);

u8   ToUTF   (char* Chr, u16* UTF16, const u16* Table, Lid Lang);
u32  UTF2UTF8(u16* Uni, char* U8);
u32  UTF82UTF(char* U8, u16* Uni);
char* exchange_diacritic_chars_utf8(char* src);
char* preparePhrase(char* phrase);

void SwitchNewLine  (CharStat* CStat,                   BLOCK* CharArea, s16 Origin,           u8 Height);
u8   CheckLowerBound(CharStat* CStat, BLOCK* PrintArea, BLOCK* CharArea,                       u8 Height);
u8   CheckWrap      (CharStat* CStat, BLOCK* PrintArea, BLOCK* CharArea, s16 Origin, u8 Width, u8 Height, u8 doWrap);

void iDrawChar(u16* Uni,  VirScreen* Screen, CharStat* CStat, BLOCK CharArea);
u32  iPrint   (char* Str, VirScreen* VScreen, CharStat* CStat, s8 paddingX, s8 paddingY, s32 Limit, Lid Lang);
u32  SimPrint (char* Str, Device* Dev, s32 x, s32 y, u16 Color, Lid Lang);

#endif
