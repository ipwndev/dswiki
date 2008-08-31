#include "SearchResults.h"

//
// C++ Implementation: SearchResults
//
// Description:
//
//
// Author: Oliver Gronau <ogronau@web.de>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

SearchResults::SearchResults(TitleIndex* t)
{
	_titleindex = t;
}

u8 SearchResults::load(string title)
{
	_list.clear();
	deque<string> temp = _titleindex->getSuggestions(title,0,10);
	int i;
	for (i=0;i<temp.size();i++)
	{
		_list.push_back(temp[i]);
	}
	_currentPosition = 0;
	return 1;
}

string SearchResults::currentHighlightedItem()
{
	if ((_currentPosition<0) || (_currentPosition>=_list.size()))
		return "";
	return _list[_currentPosition];
}

void SearchResults::display(VirScreen* VScreen, CharStat* CStat1, CharStat* CStat2)
{
	FillVS(VScreen,PA_RGB(31,31,31));
	BLOCK CharArea = {{0,0},{0,0}};
	s32 i;
	for (i=0;i<_list.size();i++)
	{
		if (i==_currentPosition)
			iPrint(_list[i]+"\n",VScreen,CStat2,&CharArea,-1,UTF8);
		else
			iPrint(_list[i]+"\n",VScreen,CStat1,&CharArea,-1,UTF8);
	}
}

u8 SearchResults::scrollLineUp()
{
	if (_currentPosition<=0)
		return 0;
	_currentPosition--;
	return 1;
}

u8 SearchResults::scrollLineDown()
{
	if (_currentPosition>=_list.size()-1)
		return 0;
	_currentPosition++;
	return 1;
}

u8 SearchResults::scrollPageUp()
{
	return scrollLineUp();
}

u8 SearchResults::scrollPageDown()
{
	return scrollLineDown();
}
