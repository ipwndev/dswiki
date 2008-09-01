#ifndef _CHRLIB_H
#define _CHRLIB_H

#include <PA9.h>
#include <string>
#include <algorithm>

#include "main.h"
#include "struct.h"
#include "Big52Uni16.h"
#include "api.h"

#include "ter12rp.h"

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

#define MAX_TITLE_LENGTH 1000

const string diacriticExchangeTable[] =
{
//   this table contains the char code for any diacritic char
//   0    1    2    3    4    5    6    7    8    9    a    b    c    d    e    f
	"",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "", // 0x80 - 0x8f
 "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "", // 0x90 - 0x9f
 "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "", // 0xa0 - 0xaf
 "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "", // 0xb0 - 0xbf
 "A", "A", "A", "A", "A", "A",  "", "C", "E", "E", "E", "E", "I", "I", "I", "I", // 0xc0 - 0xcf
 "D", "N", "O", "O", "O", "O", "O",  "", "O", "U", "U", "U", "U", "Y",  "",  "", // 0xd0 - 0xdf
 "a", "a", "a", "a", "a", "a",  "", "c", "e", "e", "e", "e", "i", "i", "i", "i", // 0xe0 - 0xef
 "", "n", "o", "o", "o", "o", "o",  "", "o", "u", "u", "u", "u", "y",  "", "y" // 0xf0 - 0xff
};

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
u32  SimPrint (const char*  Str, Device* Dev, u16 Color, Lid Lang);
u32  SimPrint (const string Str, Device* Dev, u16 Color, Lid Lang);

#endif
