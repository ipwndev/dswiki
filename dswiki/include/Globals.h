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
#ifndef _GLOBALS_H
#define _GLOBALS_H

#include <PA9.h>
#include "struct.h"

class PercentIndicator;
class Markup;
class Dumps;
class TitleIndex;

class Globals
{
	public:
		void setPercentIndicator(PercentIndicator* percentIndicator);
		PercentIndicator* getPercentIndicator();
		void setMarkup(Markup* markup);
		Markup* getMarkup();
		void setDumps(Dumps* dumps);
		Dumps* getDumps();
		void setTitleIndex(TitleIndex* titleIndex);
		TitleIndex* getTitleIndex();
	private:
		PercentIndicator* _percentIndicator;
		Markup* _markup;
		Dumps* _dumps;
		TitleIndex* _titleIndex;
};


#endif
