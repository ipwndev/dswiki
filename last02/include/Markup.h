#ifndef _MARKUP_H
#define _MARKUP_H

#include <PA9.h>
#include <string>
#include <vector>
#include "api.h"
#include "chrlib.h"
#include "tinyxml.h"

class TitleIndex;
class Globals;

using namespace std;

typedef enum {TEXT_EL, LINK_EL, ANCHOR_EL, IMG_EL} ELEMENTTYPE;

class Element
{
	public:
		~Element();
		Element(string text);
		Element(string str_namesp, string str_targ, string str_anch, string str_disp, unsigned int start, unsigned int length, unsigned int link_id);
		ELEMENTTYPE Type;
		BLOCK BoundingBox;
		string wikiNamespace;
		string displayText;
		string target;
		string anchor;
		unsigned int sourcePositionStart;
		unsigned int sourceLength;
		unsigned int id;
	private:
};

class Markupline
{
	public:
		~Markupline();
		void drawToVScreen(VirScreen* VScreen, CharStat* CStat, int line);
		unsigned char containsCertainLink(unsigned int id);
		vector<Element> children;
		void 			setGlobals(Globals* globals);
	private:
		Globals*		_globals;
};

class Markup
{
	public:
		Markup();
		~Markup();

		void 			parse(string Str);
		void			createLines(VirScreen* VScreen, CharStat* CStat);
		unsigned int	numberOfLines();
		int				currentLine();
		unsigned char	currentPercent();
		unsigned char	setCurrentLine(int line);
		void			draw();
		unsigned char	scrollLineDown();
		unsigned char	scrollLineUp();
		unsigned char	scrollPageDown();
		unsigned char	scrollPageUp();
		string			evaluateClick(s16 x,s16 y);
		void 			setGlobals(Globals* globals);
	private:
		int				_currentLine;
		vector<Markupline>	lines;
		vector<Element>	visibleChildren;
		CharStat*		_markupCStat;
		unsigned char	_linesOnVScreen1;
		unsigned char	_linesOnVScreen2;
		Globals*		_globals;
		TiXmlDocument*  _td;
};

Element* createLink(string Str, unsigned int startPos, unsigned int link_id);

#endif
