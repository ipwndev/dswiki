//
// C++ Interface: Globals
//
// Description:
//
//
// Author: Oliver Gronau <ogronau@web.de>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <PA9.h>
#include "chrlib.h"
#include "struct.h"
#include <string>

using namespace std;

class Dumps;
class TitleIndex;
class WikiMarkupGetter;
class Markup;
class Search;
class PercentIndicator;
class Statusbar;
class Font;

class Globals
{
	public:
		string loadBookmark();
		void saveBookmark(string s);

		void setDumps(Dumps* dumps);
		void setTitleIndex(TitleIndex* titleIndex);
		void setWikiMarkupGetter(WikiMarkupGetter* wikiMarkupGetter);
		void setMarkup(Markup* markup);
		void setPercentIndicator(PercentIndicator* percentIndicator);
		void setStatusbar(Statusbar* statusbar);
		void setFont(Font* font, FontCut type);
		void setLanguage(int language);
		void toggleInverted();
		int backgroundColor();
		int textColor();
		int linkColor();
		int activeLinkColor();
		int templateColor();
		int imageColor();
		Dumps*            getDumps();
		TitleIndex*       getTitleIndex();
		WikiMarkupGetter* getWikiMarkupGetter();
		Markup*           getMarkup();
		PercentIndicator* getPercentIndicator();
		Statusbar*        getStatusbar();
		Font*             getFont(FontCut type);
		int               getLanguage();
	private:
		Dumps*            _dumps;
		TitleIndex*       _titleIndex;
		WikiMarkupGetter* _wikiMarkupGetter;
		Markup*           _markup;
		PercentIndicator* _percentIndicator;
		Statusbar*        _statusbar;
		Font*             _font_r;
		Font*             _font_b;
		Font*             _font_o;
		Font*             _font_bo;
		int               _language; //0 Japanese, 1 English, 2 Français, 3 Deutsch, 4 Italian, 5 Spanish
		bool              _isInverted;
};


#endif
