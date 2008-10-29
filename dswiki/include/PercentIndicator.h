//
// C++ Interface: PercentIndicator
//
// Description:
//
//
// Author: Oliver Gronau <ogronau@web.de>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef _PERCENTINDICATOR_H
#define _PERCENTINDICATOR_H

#include <PA9.h>

class Globals;

class PercentIndicator
{
	public:
		void update(int value);
		void redraw();
	private:
		int _currentValue;
};

#endif
