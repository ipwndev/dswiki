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

void Globals::setPercentIndicator(PercentIndicator* percentIndicator)
{
	_percentIndicator = percentIndicator;
}

PercentIndicator* Globals::getPercentIndicator()
{
	return _percentIndicator;
}

void Globals::setMarkup(Markup* markup)
{
	_markup = markup;
}

Markup* Globals::getMarkup()
{
	return _markup;
}

void Globals::setDumps(Dumps* dumps)
{
	_dumps = dumps;
}

Dumps* Globals::getDumps()
{
	return _dumps;
}

void Globals::setTitleIndex(TitleIndex* titleIndex)
{
	_titleIndex = titleIndex;
}

TitleIndex* Globals::getTitleIndex()
{
	return _titleIndex;
}
