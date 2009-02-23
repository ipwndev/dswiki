#ifndef _CHRLIB_H
#define _CHRLIB_H

#include <PA9.h>
#include <string>
#include <vector>

#include "struct.h"
#include "api.h"
#include "CachingFont.h"

using namespace std;

typedef enum {UTF,UTF8} Lid;
typedef enum {NORMALWRAP,HARDWRAP,NOWRAP} ChrWrap;
typedef enum {DEG0,DEG90,DEG180,DEG270} ChrRot;
typedef enum {NONE,HOLLOW,SHADOW,BACKGR,SIMULATE} FX;
typedef enum {FONT_R,FONT_B,FONT_O,FONT_BO} FontCut;

typedef struct
{
	Font*				FONT;
	unsigned char		W_Space;
	unsigned char		H_Space;
	unsigned short int	Color;
	unsigned short int	FxColor;
	unsigned short int	BgColor;
	ChrRot				Rotate;
	ChrWrap				Wrap;
	FX					Fx;
	bool				CutChar;
} CharStat;

// 16c Unicode printing functions
void          SwitchNewLine  (const CharStat* CStat,                   BLOCK* CharArea, s16 Origin,                      unsigned char Height);
unsigned char CheckLowerBound(const CharStat* CStat, BLOCK* PrintArea, BLOCK* CharArea,                                  unsigned char Height);
unsigned char CheckWrap      (const CharStat* CStat, BLOCK* PrintArea, BLOCK* CharArea, s16 Origin, unsigned char Width, unsigned char Height, bool doWrap);

void iDrawChar(unsigned int* Uni,         const VirScreen* VScreen, const CharStat* CStat, BLOCK* CharArea);
unsigned int  iPrint   (const char*  Str, const VirScreen* VScreen, const CharStat* CStat, BLOCK* CharArea, int Limit, Lid Lang);
unsigned int  iPrint   (const string Str, const VirScreen* VScreen, const CharStat* CStat, BLOCK* CharArea, int Limit, Lid Lang);
unsigned int  SimPrint (const char*  Str, Device* Dev, unsigned short int Color, Lid Lang);
unsigned int  SimPrint (const string Str, Device* Dev, unsigned short int Color, Lid Lang);

#endif
