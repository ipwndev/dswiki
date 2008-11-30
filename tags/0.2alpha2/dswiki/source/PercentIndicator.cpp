//
// C++ Implementation: PercentIndicator
//
// Description:
//
//
// Author: Oliver Gronau <ogronau@web.de>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "PercentIndicator.h"
#include <string>
#include "main.h"
#include "api.h"
#include "chrlib.h"

void PercentIndicator::update(int value)
{
	if (value != _currentValue)
	{
		_currentValue = value;

		FillVS(&PercentArea,PA_RGB(26,26,26));

		char out[5];
		sprintf(out,"%d",value);
		string percentstr(out);
		while (percentstr.length()<3)
		{
			percentstr = " "+percentstr;
		}
		percentstr += "%";
		BLOCK CharArea = {{2,2},{0,0}};
		iPrint(percentstr,&PercentArea,&NormalCS,&CharArea,-1,UTF8);
	}
}

void PercentIndicator::clear()
{
	FillVS(&PercentArea,PA_RGB(26,26,26));
}

void PercentIndicator::redraw()
{
	FillVS(&PercentArea,PA_RGB(26,26,26));

	char out[5];
	sprintf(out,"%d",_currentValue);
	string percentstr(out);
	while (percentstr.length()<3)
	{
		percentstr = " "+percentstr;
	}
	percentstr += "%";
	BLOCK CharArea = {{2,2},{0,0}};
	iPrint(percentstr,&PercentArea,&NormalCS,&CharArea,-1,UTF8);

}
