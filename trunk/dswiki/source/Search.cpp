//
// C++ Implementation: Search
//
// Description:
//
//
// Author: Oliver Gronau <ogronau@web.de>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "Search.h"

#include <PA9.h>
#include <string>
#include <deque>
#include "api.h"
#include "chrlib.h"
#include "TitleIndex.h"
#include "Globals.h"
#include "main.h"

#define MAX_SEARCH_RESULTS 128
#define SEARCHRESULT_LINES 13

Search::Search()
{
}

void Search::load(int articleNumber)
{
	int i;

	if (articleNumber < 0)
		articleNumber = 0;
	if (articleNumber >= _globals->getTitleIndex()->NumberOfArticles())
		articleNumber = _globals->getTitleIndex()->NumberOfArticles() - 1;

	// getSuggestedArticleNumber is using the highest possible index
	_absolute_CurrentArticleNumber = articleNumber;

	_absolute_FirstArticleNumber = _absolute_CurrentArticleNumber;
	_absolute_LastArticleNumber = _absolute_FirstArticleNumber + SEARCHRESULT_LINES - 1;

	if ( _absolute_LastArticleNumber >= _globals->getTitleIndex()->NumberOfArticles())
	{
		_absolute_FirstArticleNumber -= _absolute_LastArticleNumber - _globals->getTitleIndex()->NumberOfArticles() + 1;
		_absolute_LastArticleNumber = _globals->getTitleIndex()->NumberOfArticles() - 1;
	}

	_absolute_FirstDisplayNumber   = _absolute_FirstArticleNumber;
	_absolute_LastDisplayNumber    = _absolute_LastArticleNumber;

	_list_FirstArticleNumber   = 0;
	_list_FirstDisplayNumber   = 0;
	_list_CurrentArticleNumber = _absolute_CurrentArticleNumber - _absolute_FirstArticleNumber;
	_list_LastDisplayNumber    = _absolute_LastArticleNumber - _absolute_FirstArticleNumber;
	_list_LastArticleNumber    = _list_LastDisplayNumber;

	_list.clear();
	for (i=_absolute_FirstArticleNumber;i<=_absolute_LastArticleNumber;i++)
	{
		// getTitle is using the highest possible index
		_list.push_back(_globals->getTitleIndex()->getTitle(i));
	}

	_wasScrolled = 1;
}

void Search::load(string phrase)
{
	load(_globals->getTitleIndex()->getSuggestedArticleNumber(phrase));
}

string Search::currentHighlightedItem()
{
	if ((_list_CurrentArticleNumber<0) || (_list_CurrentArticleNumber>= (int) _list.size()))
		return "";
	return _list[_list_CurrentArticleNumber];
}


void Search::display()
{
	if (_wasScrolled)
	{
		FillVS(&ContentWin1,_globals->backgroundColor());
		_wasScrolled = 0;
	}
	BLOCK CharArea = {{0,0},{0,0}};
	int i;
	CharStat CS = NormalCS;
	CS.Wrap = NOWRAP;
	for (i=_list_FirstDisplayNumber;i<=_list_LastDisplayNumber;i++)  // TODO: Only paint necessary lines, not all
	{
		if (i!=_list_CurrentArticleNumber)
		{
			CS.Color = _globals->textColor();
			iPrint(_list[i]+"\n",&ContentWin1,&CS,&CharArea,-1,UTF8);
		}
		else
		{
			CS.Color = PA_RGB(31,0,0);
			iPrint(_list[i]+"\n",&ContentWin1,&CS,&CharArea,-1,UTF8);
		}
	}
}

bool Search::scrollLineUp()
{
	if (_absolute_CurrentArticleNumber <= 0)
		return 0;
	// we can do one step backward

	_absolute_CurrentArticleNumber--;
	_list_CurrentArticleNumber--;

	if (_absolute_CurrentArticleNumber < _absolute_FirstDisplayNumber)
	{
		// Scrolling over the bottom line
		_absolute_FirstDisplayNumber--;
		_absolute_LastDisplayNumber--;
		_list_FirstDisplayNumber--;
		_list_LastDisplayNumber--;
		_wasScrolled = 1;
	}

	if (_list_FirstDisplayNumber < 0)
	{
		// enlarge at the front neccessary
		_absolute_FirstArticleNumber -= (SEARCHRESULT_LINES-1);
		_list_FirstArticleNumber -= (SEARCHRESULT_LINES-1);

		if (_absolute_FirstArticleNumber < 0)
		{
			_list_FirstArticleNumber -= _absolute_FirstArticleNumber;
			_absolute_FirstArticleNumber = 0;
		}

		int i;
		for (i=_absolute_CurrentArticleNumber;i>=_absolute_FirstArticleNumber;i--)
		{
			_list.push_front(_globals->getTitleIndex()->getTitle(i));
			_list_FirstArticleNumber++;
			_list_FirstDisplayNumber++;
			_list_CurrentArticleNumber++;
			_list_LastDisplayNumber++;
			_list_LastArticleNumber++;
		}

		while (_list.size() > MAX_SEARCH_RESULTS)
		{
			_absolute_LastArticleNumber--;
			_list_LastArticleNumber--;
			_list.pop_back();
		}

	}
	return 1;
}

bool Search::scrollLineDown()
{
	if (_absolute_CurrentArticleNumber >= _globals->getTitleIndex()->NumberOfArticles() - 1)
		return 0;
	// we do one step more

	_absolute_CurrentArticleNumber++;
	_list_CurrentArticleNumber++;

	if (_absolute_CurrentArticleNumber > _absolute_LastDisplayNumber)
	{
		// Scrolling over the bottom line
		_absolute_FirstDisplayNumber++;
		_absolute_LastDisplayNumber++;
		_list_FirstDisplayNumber++;
		_list_LastDisplayNumber++;
		_wasScrolled = 1;
	}

	if (_list_LastDisplayNumber > _list_LastArticleNumber)
	{
		// enlarge at the tail
		_list_LastArticleNumber += (SEARCHRESULT_LINES-1);
		_absolute_LastArticleNumber += (SEARCHRESULT_LINES-1);

		if (_absolute_LastArticleNumber > _globals->getTitleIndex()->NumberOfArticles() - 1)
		{
			_absolute_LastArticleNumber = _globals->getTitleIndex()->NumberOfArticles() - 1;
			_list_LastArticleNumber = _absolute_LastArticleNumber - _absolute_FirstArticleNumber;
		}

		int i;
		for (i=_absolute_CurrentArticleNumber;i<=_absolute_LastArticleNumber;i++)
		{
			_list.push_back(_globals->getTitleIndex()->getTitle(i));
		}

		while (_list.size() > MAX_SEARCH_RESULTS)
		{
			_list_FirstDisplayNumber--;
			_list_CurrentArticleNumber--;
			_list_LastDisplayNumber--;
			_list_LastArticleNumber--;
			_absolute_FirstArticleNumber++;
			_list.pop_front();
		}

	}
	return 1;
}

bool Search::scrollPageUp()
{
	unsigned char any = 0;
	unsigned char i;
	for (i=0;i<SEARCHRESULT_LINES-1;i++)
	{
		if (scrollLineUp())
		{
			any = 1;
		}
	}
	return any;
}

bool Search::scrollPageDown()
{
	unsigned char any = 0;
	unsigned char i;
	for (i=0;i<SEARCHRESULT_LINES-1;i++)
	{
		if (scrollLineDown())
		{
			any = 1;
		}
	}
	return any;
}

bool Search::scrollLongUp()
{
	unsigned char change = _absolute_CurrentArticleNumber==0?0:1;
	load(_absolute_CurrentArticleNumber-25*(SEARCHRESULT_LINES-1));
	return change;
}

bool Search::scrollLongDown()
{
	unsigned char change = _absolute_CurrentArticleNumber==_globals->getTitleIndex()->NumberOfArticles()-1?0:1;
	load(_absolute_CurrentArticleNumber+25*(SEARCHRESULT_LINES-1));
	return change;
}

void Search::setGlobals(Globals* globals)
{
	_globals = globals;
}
