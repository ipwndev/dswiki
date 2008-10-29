#ifndef _CHRLIB_H
#define _CHRLIB_H

#include <PA9.h>
#include <string>

#include "struct.h"
#include "api.h"

using namespace std;

typedef enum {UTF,UTF8,BIG5,GBK,JIS} Lid;
typedef enum {REGULAR,BOLD,ITALIC,BOLDITALIC} Cut;
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
} SingleCut;

class Font
{
	public:
		Font();
		u8 * getCharacterData(u32 Uni, Cut CutType);
		u8 initOK();
		SingleCut Regular;
		SingleCut Bold;
		SingleCut Italic;
		SingleCut BoldItalic;
	private:
		u8* _data_regular;
		u8* _data_bold;
		u8* _data_italic;
		u8* _data_bolditalic;
		void InitFont(SingleCut* FONT, const u8* ptr);
		u8 _initOK;
};

typedef struct
{
	Font*   FONT;
	Cut     FontCut;
	u8      W_Space;
	u8      H_Space;
	u16     Color;
	u16     FxColor;
	u16     BgColor;
	ChrRot  Rotate;
	ChrWrap Wrap;
	FX      Fx;
	u8      CutChar;
} CharStat;

typedef struct
{
	string entity;
	u32    codepoint;
} NamedEntity;

#define MAX_TITLE_LENGTH 1000
#define MAX_NAMED_ENTITIES 253

u8   ToUTF   (const char* Chr, u32* UTF16, Lid Lang);
u32  UTF2UTF8(u32* Uni, char* U8);
u32  UTF82UTF(char* U8, u32* Uni);

string trimPhrase(string Str);
string lowerPhrase(string phrase);
string exchangeDiacriticCharsUTF8Phrase(string phrase);
string exchangeSGMLEntities(string phrase);
void tolower_utf8(char* data);
string treatNowikiText(string phrase);
string treatPreText(string phrase);

string preparePhrase(string phrase, u8 indexNo);

void SwitchNewLine  (const CharStat* CStat,                   BLOCK* CharArea, s16 Origin,           u8 Height);
u8   CheckLowerBound(const CharStat* CStat, BLOCK* PrintArea, BLOCK* CharArea,                       u8 Height);
u8   CheckWrap      (const CharStat* CStat, BLOCK* PrintArea, BLOCK* CharArea, s16 Origin, u8 Width, u8 Height, u8 doWrap);

void iDrawChar(u32* Uni,         const VirScreen* VScreen, const CharStat* CStat, BLOCK* CharArea);
u32  iPrint   (const char*  Str, const VirScreen* VScreen, const CharStat* CStat, BLOCK* CharArea, s32 Limit, Lid Lang);
u32  iPrint   (const string Str, const VirScreen* VScreen, const CharStat* CStat, BLOCK* CharArea, s32 Limit, Lid Lang);
u32  SimPrint (const char*  Str, Device* Dev, u16 Color, Lid Lang);
u32  SimPrint (const string Str, Device* Dev, u16 Color, Lid Lang);

const NamedEntity entities[] =
{
	{"&quot;"    , 34},
 {"&amp;"     , 38},
 {"&apos;"    , 39},
 {"&gt;"      , 62},
 {"&nbsp;"    ,160},
 {"&lt;"      , 60},
 {"&iexcl;"   ,161},
 {"&cent;"    ,162},
 {"&pound;"   ,163},
 {"&curren;"  ,164},
 {"&yen;"     ,165},
 {"&brvbar;"  ,166},
 {"&sect;"    ,167},
 {"&uml;"     ,168},
 {"&copy;"    ,169},
 {"&ordf;"    ,170},
 {"&laquo;"   ,171},
 {"&not;"     ,172},
 {"&shy;"     ,173},
 {"&reg;"     ,174},
 {"&macr;"    ,175},
 {"&deg;"     ,176},
 {"&plusmn;"  ,177},
 {"&sup2;"    ,178},
 {"&sup3;"    ,179},
 {"&acute;"   ,180},
 {"&micro;"   ,181},
 {"&para;"    ,182},
 {"&middot;"  ,183},
 {"&cedil;"   ,184},
 {"&sup1;"    ,185},
 {"&ordm;"    ,186},
 {"&raquo;"   ,187},
 {"&frac14;"  ,188},
 {"&frac12;"  ,189},
 {"&frac34;"  ,190},
 {"&iquest;"  ,191},
 {"&Agrave;"  ,192},
 {"&Aacute;"  ,193},
 {"&Acirc;"   ,194},
 {"&Atilde;"  ,195},
 {"&Auml;"    ,196},
 {"&Aring;"   ,197},
 {"&AElig;"   ,198},
 {"&Ccedil;"  ,199},
 {"&Egrave;"  ,200},
 {"&Eacute;"  ,201},
 {"&Ecirc;"   ,202},
 {"&Euml;"    ,203},
 {"&Igrave;"  ,204},
 {"&Iacute;"  ,205},
 {"&Icirc;"   ,206},
 {"&Iuml;"    ,207},
 {"&ETH;"     ,208},
 {"&Ntilde;"  ,209},
 {"&Ograve;"  ,210},
 {"&Oacute;"  ,211},
 {"&Ocirc;"   ,212},
 {"&Otilde;"  ,213},
 {"&Ouml;"    ,214},
 {"&times;"   ,215},
 {"&Oslash;"  ,216},
 {"&Ugrave;"  ,217},
 {"&Uacute;"  ,218},
 {"&Ucirc;"   ,219},
 {"&Uuml;"    ,220},
 {"&Yacute;"  ,221},
 {"&THORN;"   ,222},
 {"&szlig;"   ,223},
 {"&agrave;"  ,224},
 {"&aacute;"  ,225},
 {"&acirc;"   ,226},
 {"&atilde;"  ,227},
 {"&auml;"    ,228},
 {"&aring;"   ,229},
 {"&aelig;"   ,230},
 {"&ccedil;"  ,231},
 {"&egrave;"  ,232},
 {"&eacute;"  ,233},
 {"&ecirc;"   ,234},
 {"&euml;"    ,235},
 {"&igrave;"  ,236},
 {"&iacute;"  ,237},
 {"&icirc;"   ,238},
 {"&iuml;"    ,239},
 {"&eth;"     ,240},
 {"&ntilde;"  ,241},
 {"&ograve;"  ,242},
 {"&oacute;"  ,243},
 {"&ocirc;"   ,244},
 {"&otilde;"  ,245},
 {"&ouml;"    ,246},
 {"&divide;"  ,247},
 {"&oslash;"  ,248},
 {"&ugrave;"  ,249},
 {"&uacute;"  ,250},
 {"&ucirc;"   ,251},
 {"&uuml;"    ,252},
 {"&yacute;"  ,253},
 {"&thorn;"   ,254},
 {"&yuml;"    ,255},
 {"&OElig;"   ,338},
 {"&oelig;"   ,339},
 {"&Scaron;"  ,352},
 {"&scaron;"  ,353},
 {"&Yuml;"    ,376},
 {"&circ;"    ,710},
 {"&tilde;"   ,732},
 {"&fnof;"    ,402},
 {"&Alpha;"   ,913},
 {"&Beta;"    ,914},
 {"&Gamma;"   ,915},
 {"&Delta;"   ,916},
 {"&Epsilon;" ,917},
 {"&Zeta;"    ,918},
 {"&Eta;"     ,919},
 {"&Theta;"   ,920},
 {"&Iota;"    ,921},
 {"&Kappa;"   ,922},
 {"&Lambda;"  ,923},
 {"&Mu;"      ,924},
 {"&Nu;"      ,925},
 {"&Xi;"      ,926},
 {"&Omicron;" ,927},
 {"&Pi;"      ,928},
 {"&Rho;"     ,929},
 {"&Sigma;"   ,931},
 {"&Tau;"     ,932},
 {"&Upsilon;" ,933},
 {"&Phi;"     ,934},
 {"&Chi;"     ,935},
 {"&Psi;"     ,936},
 {"&Omega;"   ,937},
 {"&alpha;"   ,945},
 {"&beta;"    ,946},
 {"&gamma;"   ,947},
 {"&delta;"   ,948},
 {"&epsilon;" ,949},
 {"&zeta;"    ,950},
 {"&eta;"     ,951},
 {"&theta;"   ,952},
 {"&iota;"    ,953},
 {"&kappa;"   ,954},
 {"&lambda;"  ,955},
 {"&mu;"      ,956},
 {"&nu;"      ,957},
 {"&xi;"      ,958},
 {"&omicron;" ,959},
 {"&pi;"      ,960},
 {"&rho;"     ,961},
 {"&sigmaf;"  ,962},
 {"&sigma;"   ,963},
 {"&tau;"     ,964},
 {"&upsilon;" ,965},
 {"&phi;"     ,966},
 {"&chi;"     ,967},
 {"&psi;"     ,968},
 {"&omega;"   ,969},
 {"&thetasym;",977},
 {"&upsih;"   ,978},
 {"&piv;"     ,982},
 {"&ensp;"    ,8194},
 {"&emsp;"    ,8195},
 {"&thinsp;"  ,8201},
 {"&zwnj;"    ,8204},
 {"&zwj;"     ,8205},
 {"&lrm;"     ,8206},
 {"&rlm;"     ,8207},
 {"&ndash;"   ,8211},
 {"&mdash;"   ,8212},
 {"&lsquo;"   ,8216},
 {"&rsquo;"   ,8217},
 {"&sbquo;"   ,8218},
 {"&ldquo;"   ,8220},
 {"&rdquo;"   ,8221},
 {"&bdquo;"   ,8222},
 {"&dagger;"  ,8224},
 {"&Dagger;"  ,8225},
 {"&permil;"  ,8240},
 {"&lsaquo;"  ,8249},
 {"&rsaquo;"  ,8250},
 {"&euro;"    ,8364},
 {"&bull;"    ,8226},
 {"&hellip;"  ,8230},
 {"&prime;"   ,8242},
 {"&Prime;"   ,8243},
 {"&oline;"   ,8254},
 {"&frasl;"   ,8260},
 {"&weierp;"  ,8472},
 {"&image;"   ,8465},
 {"&real;"    ,8476},
 {"&trade;"   ,8482},
 {"&alefsym;" ,8501},
 {"&larr;"    ,8592},
 {"&uarr;"    ,8593},
 {"&rarr;"    ,8594},
 {"&darr;"    ,8595},
 {"&harr;"    ,8596},
 {"&crarr;"   ,8629},
 {"&lArr;"    ,8656},
 {"&uArr;"    ,8657},
 {"&rArr;"    ,8658},
 {"&dArr;"    ,8659},
 {"&hArr;"    ,8660},
 {"&forall;"  ,8704},
 {"&part;"    ,8706},
 {"&exist;"   ,8707},
 {"&empty;"   ,8709},
 {"&nabla;"   ,8711},
 {"&isin;"    ,8712},
 {"&notin;"   ,8713},
 {"&ni;"      ,8715},
 {"&prod;"    ,8719},
 {"&sum;"     ,8721},
 {"&minus;"   ,8722},
 {"&lowast;"  ,8727},
 {"&radic;"   ,8730},
 {"&prop;"    ,8733},
 {"&infin;"   ,8734},
 {"&ang;"     ,8736},
 {"&and;"     ,8743},
 {"&or;"      ,8744},
 {"&cap;"     ,8745},
 {"&cup;"     ,8746},
 {"&int;"     ,8747},
 {"&there4;"  ,8756},
 {"&sim;"     ,8764},
 {"&cong;"    ,8773},
 {"&asymp;"   ,8776},
 {"&ne;"      ,8800},
 {"&equiv;"   ,8801},
 {"&le;"      ,8804},
 {"&ge;"      ,8805},
 {"&sub;"     ,8834},
 {"&sup;"     ,8835},
 {"&nsub;"    ,8836},
 {"&sube;"    ,8838},
 {"&supe;"    ,8839},
 {"&oplus;"   ,8853},
 {"&otimes;"  ,8855},
 {"&perp;"    ,8869},
 {"&sdot;"    ,8901},
 {"&lceil;"   ,8968},
 {"&rceil;"   ,8969},
 {"&lfloor;"  ,8970},
 {"&rfloor;"  ,8971},
 {"&lang;"    ,9001},
 {"&rang;"    ,9002},
 {"&loz;"     ,9674},
 {"&spades;"  ,9824},
 {"&clubs;"   ,9827},
 {"&hearts;"  ,9829},
 {"&diams;"   ,9830}
};

const string  diacriticExchangeTable[] =
{
//   this tab le contains the char code for any diacritic char
//   0    1     2    3    4    5    6    7    8    9    a    b    c    d    e    f
  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "", // 0x0080 - 0x008F
  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "", // 0x0090 - 0x009F
  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "", // 0x00A0 - 0x00AF
  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "",  "", // 0x00B0 - 0x00BF
 "A", "A", "A", "A", "A", "A","AE", "C", "E", "E", "E", "E", "I", "I", "I", "I", // 0x00C0 - 0x00CF
 "D", "N", "O", "O", "O", "O", "O",  "", "O", "U", "U", "U", "U", "Y","TH","ss", // 0x00D0 - 0x00DF
 "a", "a", "a", "a", "a", "a","ae", "c", "e", "e", "e", "e", "i", "i", "i", "i", // 0x00E0 - 0x00EF
 "d", "n", "o", "o", "o", "o", "o",  "", "o", "u", "u", "u", "u", "y","th", "y", // 0x00F0 - 0x00FF
 "A", "a", "A", "a", "A", "a", "C", "c", "C", "c", "C", "c", "C", "c", "D", "d", // 0x0100 - 0x010F
 "D", "d", "E", "e", "E", "e", "E", "e", "E", "e", "E", "e", "G", "g", "G", "g", // 0x0110 - 0x011F
 "G", "g", "G", "g", "H", "h", "H", "h", "I", "i", "I", "i", "I", "i", "I", "i", // 0x0120 - 0x012F
 "I", "i","IJ","ij", "J", "j", "K", "k", "k", "L", "l", "L", "l", "L", "l", "L", // 0x0130 - 0x013F
 "l", "L", "l", "N", "n", "N", "n", "N", "n", "n",  "",  "", "O", "o", "O", "o", // 0x0140 - 0x014F
 "O", "o","OE","oe", "R", "r", "R", "r", "R", "r", "S", "s", "S", "s", "S", "s", // 0x0150 - 0x015F
 "S", "s", "T", "t", "T", "t", "T", "t", "U", "u", "U", "u", "U", "u", "U", "u", // 0x0160 - 0x016F
 "U", "u", "U", "u", "W", "w", "Y", "y", "Y", "Z", "z", "Z", "z", "Z", "z", "s", // 0x0170 - 0x017F
};

#endif
