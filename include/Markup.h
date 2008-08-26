#ifndef _MARKUP_H
#define _MARKUP_H

#include <PA9.h>
#include <string>
#include <vector>
#include "main.h"
#include "api.h"
#include "struct.h"
#include "chrlib.h"
#include "ter12rp.h"

using namespace std;

typedef enum {TEXT_EL, LINK_EL, ANCHOR_EL} ELEMENTTYPE;

class Element
{
	public:
		~Element();
		Element(string text);
		Element(string str_namesp, string str_targ, string str_anch, string str_disp, u32 start, u32 length, u32 link_id);
		ELEMENTTYPE Type;
		BLOCK BoundingBox;
		string wikiNamespace;
		string displayText;
		string target;
		string anchor;
		u32 sourcePositionStart;
		u32 sourceLength;
		u32 id;
	private:
};

class Markupline
{
	public:
		~Markupline();
		void drawToVScreen(VirScreen* VScreen, CharStat* CStat, s32 line);
		u8 containsCertainLink(u32 id);
		vector<Element> children;
	private:
};

class Markup
{
	public:
		Markup(string Str, VirScreen* VScreen1, VirScreen* VScreen2, CharStat* CStat);
		~Markup();

		void createLines(VirScreen* VScreen, CharStat* CStat);
		u32 numberOfLines();
		u8 setCurrentLine(s32 line);
		void draw();
		u8 ScrollLineDown();
		u8 ScrollLineUp();
		u8 ScrollPageDown();
		u8 ScrollPageUp();
		string evaluateClick(s16 x,s16 y);
	private:
		s32 _currentLine;
		vector<Markupline> lines;
		vector<Element> visibleChildren;
		VirScreen* _markupVScreen1;
		VirScreen* _markupVScreen2;
		CharStat*  _markupCStat;
		u8 _linesOnVScreen1;
		u8 _linesOnVScreen2;
};

Element* createLink(string Str, u32 startPos, u32 link_id);

#endif
