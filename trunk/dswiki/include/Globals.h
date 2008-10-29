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
#include "struct.h"

class Dumps;
class TitleIndex;
class WikiMarkupGetter;
class Markup;
class Search;
class PercentIndicator;
class Statusbar;

class Globals
{
	public:
		void setDumps(Dumps* dumps);
		void setTitleIndex(TitleIndex* titleIndex);
		void setWikiMarkupGetter(WikiMarkupGetter* wikiMarkupGetter);
		void setMarkup(Markup* markup);
		void setSearch(Search* search);
		void setPercentIndicator(PercentIndicator* percentIndicator);
		void setStatusbar(Statusbar* statusbar);
		Dumps*            getDumps();
		TitleIndex*       getTitleIndex();
		WikiMarkupGetter* getWikiMarkupGetter();
		Markup*           getMarkup();
		Search*           getSearch();
		PercentIndicator* getPercentIndicator();
		Statusbar*        getStatusbar();
	private:
		Dumps*            _dumps;
		TitleIndex*       _titleIndex;
		WikiMarkupGetter* _wikiMarkupGetter;
		Markup*           _markup;
		Search*           _search;
		PercentIndicator* _percentIndicator;
		Statusbar*        _statusbar;
};


#endif
