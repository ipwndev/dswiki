#ifndef CHRLIBH
#define CHRLIBH

#include <PA9.h>
#include "struct.h"
#include "Big52Uni16.h"
#include "api.h"

#include "cu12.h"
#include "f04x06.h"
#include "f05x07.h"
#include "f05x08.h"
#include "f06x09.h"
#include "f06x10.h"
#include "f06x12.h"
#include "f06x13.h"
#include "f07x13.h"
#include "f07x14.h"
#include "f08x13.h"
#include "f09x15.h"
#include "f09x18.h"
#include "f10x20.h"
#include "ter12b.h"
#include "ter12bi.h"
#include "ter12i.h"
#include "ter12r.h"
#include "ter12rp.h"
#include "ter14b.h"
#include "ter14bi.h"
#include "ter14i.h"
#include "ter14r.h"
#include "ter16b.h"
#include "ter16bi.h"
#include "ter16i.h"
#include "ter16r.h"
#include "ter20b.h"
#include "ter20bi.h"
#include "ter20i.h"
#include "ter20r.h"
#include "ter24b.h"
#include "ter24bi.h"
#include "ter24i.h"
#include "ter24r.h"
#include "ter28b.h"
#include "ter28bi.h"
#include "ter28i.h"
#include "ter28r.h"
#include "ter32b.h"
#include "ter32bi.h"
#include "ter32i.h"
#include "ter32r.h"
#include "unifont.h"


typedef enum {UTF,UTF8,BIG5,GBK,JIS} Lid;
typedef enum {Deg0,Deg90,Deg180,Deg270} ChrRot;
typedef enum {NONE,HOLLOW,SHADOW,BACKGR} FX;
typedef enum {NO,YES} ABLE;

typedef struct
{
  u16*      Name;
  u8        Height;
  u8        W_Space;
  u32*      Index;
  u8*       Data;
  const u8* Ptr;
} Font;

typedef struct
{
	u16    Color;
	u16    BgColor;
	u8     Align;
	ChrRot Rotate;     //[0]:0;[1]:90;[2]:180;[3]:270;
	FX     Fx;         //[0]:None;[1]:?r;[2]:v;[3]:?;
	u8     CutChar;		//[0]:No;[1]:Yes;
	u8     W_Space;
	u8     H_Space;
	Font*  FONT;
} CharStat;

extern Font  terminus12regular;
extern Font  terminus12bold;
extern Font  terminus12italic;
extern Font  terminus12bolditalic;

extern const CharStat CS;
extern char U[255];
extern char N[255];

char*  U2A(char* Ui);
u32    UTFLen(char* Str);
u8     IsFullChar(char* Text);
u8     FontInit(Font* FONT,const u8* ptr);
u8     ToUTF(char* Chr,u16* UTF16, const u16* Table, Lid LID);
u32    UTF2UTF8(u16* Uni,u8* Utf);
u32    UTF82UTF(u8* U8,u16* U);
u32    len(char* Str);
u16    WordWidth(char* Word, CharStat *CStat, Lid Lang);
void   SwitchNewLine(CharStat *Status, BLOCK* CharArea, s16 Origin, u8 Height);
u8     CheckLowerBound(BLOCK* PrintArea, BLOCK *CharArea,CharStat* CStat);
u8     CheckWrap(CharStat* Status, BLOCK* PrintArea , BLOCK* CharArea, s16 Origin, u8 Width, u8 Height);
u32    iPrint(char* Str, VirScreen* Screen, CharStat* Status, u32 Num, Lid Lang);
void   iDrawChar(u16* Uni,VirScreen* Screen,CharStat* CS, BLOCK CharArea);
u32    SimPrint(char* Str, Device* Dev, s32 x, s32 y, u16 Color, Lid Lang);

#endif
