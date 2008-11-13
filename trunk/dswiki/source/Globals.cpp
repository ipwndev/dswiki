//
// C++ Implementation: Globals
//
// Description:
//
//
// Author: Oliver Gronau <ogronau@web.de>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "Globals.h"

void Globals::setDumps(Dumps* dumps) { _dumps = dumps; }
void Globals::setTitleIndex(TitleIndex* titleIndex) { _titleIndex = titleIndex; }
void Globals::setWikiMarkupGetter(WikiMarkupGetter* wikiMarkupGetter) {	_wikiMarkupGetter = wikiMarkupGetter; }
void Globals::setMarkup(Markup* markup) { _markup = markup; }
void Globals::setSearch(Search* search) { _search = search; }
void Globals::setPercentIndicator(PercentIndicator* percentIndicator) { _percentIndicator = percentIndicator; }
void Globals::setStatusbar(Statusbar* statusbar) { _statusbar = statusbar; }
void Globals::setFont(Font* font) { _font = font; }

Dumps*            Globals::getDumps() {	return _dumps; }
TitleIndex*       Globals::getTitleIndex() { return _titleIndex; }
WikiMarkupGetter* Globals::getWikiMarkupGetter() { return _wikiMarkupGetter; }
Markup*           Globals::getMarkup() { return _markup; }
Search*           Globals::getSearch() { return _search; }
PercentIndicator* Globals::getPercentIndicator() { return _percentIndicator; }
Statusbar*        Globals::getStatusbar() { return _statusbar; }
Font*             Globals::getFont() { return _font; }
