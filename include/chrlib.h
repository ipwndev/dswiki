#ifndef _CHRLIB_H
#define _CHRLIB_H

#include <PA9.h>
#include <string>

#include "struct.h"
#include "api.h"

using namespace std;

typedef enum {UTF,UTF8} Lid;
typedef enum {REGULAR,BOLD,ITALIC,BOLDITALIC} Cut;
typedef enum {NORMALWRAP,HARDWRAP,NOWRAP} ChrWrap;
typedef enum {DEG0,DEG90,DEG180,DEG270} ChrRot;
typedef enum {NONE,HOLLOW,SHADOW,BACKGR,SIMULATE} FX;

typedef struct
{
	unsigned short int*  Name;
	unsigned char        Height;
	unsigned int*        Index;
	unsigned char*       Data;
	const unsigned char* Ptr;
} SingleCut;

class Font
{
	public:
		Font();
		unsigned char* getCharacterData(unsigned int Uni, Cut CutType);
		unsigned char initOK();
		SingleCut Regular;
		SingleCut Bold;
		SingleCut Italic;
		SingleCut BoldItalic;
	private:
		void InitFont(SingleCut* FONT, const unsigned char* ptr);
		unsigned char* _data_regular;
		unsigned char* _data_bold;
		unsigned char* _data_italic;
		unsigned char* _data_bolditalic;
		unsigned char _initOK;
};

typedef struct
{
	Font*              FONT;
	Cut                FontCut;
	unsigned char      W_Space;
	unsigned char      H_Space;
	unsigned short int Color;
	unsigned short int FxColor;
	unsigned short int BgColor;
	ChrRot             Rotate;
	ChrWrap            Wrap;
	FX                 Fx;
	unsigned char      CutChar;
} CharStat;




// public functions

// private functions

// 16c Unicode printing functions
void            SwitchNewLine  (const CharStat* CStat,                   BLOCK* CharArea, s16 Origin,           unsigned char Height);
unsigned char   CheckLowerBound(const CharStat* CStat, BLOCK* PrintArea, BLOCK* CharArea,                       unsigned char Height);
unsigned char   CheckWrap      (const CharStat* CStat, BLOCK* PrintArea, BLOCK* CharArea, s16 Origin, unsigned char Width, unsigned char Height, unsigned char doWrap);

void iDrawChar(unsigned int* Uni,         const VirScreen* VScreen, const CharStat* CStat, BLOCK* CharArea);
unsigned int  iPrint   (const char*  Str, const VirScreen* VScreen, const CharStat* CStat, BLOCK* CharArea, int Limit, Lid Lang);
unsigned int  iPrint   (const string Str, const VirScreen* VScreen, const CharStat* CStat, BLOCK* CharArea, int Limit, Lid Lang);
unsigned int  SimPrint (const char*  Str, Device* Dev, unsigned short int Color, Lid Lang);
unsigned int  SimPrint (const string Str, Device* Dev, unsigned short int Color, Lid Lang);




#endif
