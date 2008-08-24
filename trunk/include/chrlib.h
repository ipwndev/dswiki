#ifndef _CHRLIB_H
#define _CHRLIB_H

#include <PA9.h>
#include <string>

#include "struct.h"
#include "Big52Uni16.h"
#include "api.h"

#include "ter12r.h"

using namespace std;

typedef enum {UTF,UTF8,BIG5,GBK,JIS} Lid;
typedef enum {NORMALWRAP,HARDWRAP,NOWRAP} ChrWrap;
typedef enum {DEG0,DEG90,DEG180,DEG270} ChrRot;
typedef enum {NONE,HOLLOW,SHADOW,BACKGR,SIMULATE} FX;

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

u8   ToUTF   (const char* Chr, u16* UTF16, const u16* Table, Lid Lang);
u32  UTF2UTF8(u16* Uni, char* U8);
u32  UTF82UTF(char* U8, u16* Uni);
string exchange_diacritic_chars_utf8(string src);
string preparePhrase(string phrase);
string trim(string Str);

void SwitchNewLine  (const CharStat* CStat,                   BLOCK* CharArea, s16 Origin,           u8 Height);
u8   CheckLowerBound(const CharStat* CStat, BLOCK* PrintArea, BLOCK* CharArea,                       u8 Height);
u8   CheckWrap      (const CharStat* CStat, BLOCK* PrintArea, BLOCK* CharArea, s16 Origin, u8 Width, u8 Height, u8 doWrap);

void iDrawChar(u16* Uni,         const VirScreen* VScreen, const CharStat* CStat, BLOCK* CharArea);
u32  iPrint   (const char*  Str, const VirScreen* VScreen, const CharStat* CStat, BLOCK* CharArea, s32 Limit, Lid Lang);
u32  iPrint   (const string Str, const VirScreen* VScreen, const CharStat* CStat, BLOCK* CharArea, s32 Limit, Lid Lang);
u32  SimPrint (char*  Str, Device* Dev, u16 Color, Lid Lang);
u32  SimPrint (string Str, Device* Dev, u16 Color, Lid Lang);

#endif
