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

SearchResults::SearchResults(TitleIndex* t, VirScreen* VScreen, CharStat* CStat1, CharStat* CStat2)
{
	_titleindex = t;
	_vscreen = VScreen;
	_cstat1 = CStat1;
	_cstat2 = CStat2;
}

void SearchResults::load(string phrase)
{
	int i;

	// getSuggestedArticleNumber is using the highest possible index
	_absolute_CurrentArticleNumber = _titleindex->getSuggestedArticleNumber(phrase);

	_absolute_FirstArticleNumber = _absolute_CurrentArticleNumber;
	_absolute_LastArticleNumber = _absolute_FirstArticleNumber + 9;//13

	if ( _absolute_LastArticleNumber >= _titleindex->NumberOfArticles())
	{
		_absolute_FirstArticleNumber -= _absolute_LastArticleNumber - _titleindex->NumberOfArticles() + 1;
		_absolute_LastArticleNumber = _titleindex->NumberOfArticles() - 1;
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
		_list.push_back(_titleindex->getTitle(i));
	}

	_wasScrolled = 1;
}

string SearchResults::currentHighlightedItem()
{
	if ((_list_CurrentArticleNumber<0) || (_list_CurrentArticleNumber>=_list.size()))
		return "";
	return _list[_list_CurrentArticleNumber];
}


#define PREPARE_TO_DISPLAY 0

void SearchResults::display()
{
	if (_wasScrolled)
	{
		FillVS(_vscreen,PA_RGB(31,31,31));
		_wasScrolled = 0;
	}
	BLOCK CharArea = {{0,0},{0,0}};
	int i;
	for (i=_list_FirstDisplayNumber;i<=_list_LastDisplayNumber;i++)
	{
		if (i!=_list_CurrentArticleNumber)
		{
#if PREPARE_TO_DISPLAY
			iPrint(preparePhrase(_list[i],1)+"\n",_vscreen,_cstat1,&CharArea,-1,UTF8);
#else
			iPrint(_list[i]+"\n",_vscreen,_cstat1,&CharArea,-1,UTF8);
#endif
		}
		else
		{
#if PREPARE_TO_DISPLAY
			iPrint(preparePhrase(_list[i],1)+"\n",_vscreen,_cstat2,&CharArea,-1,UTF8);
#else
			iPrint(_list[i]+"\n",_vscreen,_cstat2,&CharArea,-1,UTF8);
#endif
		}
	}
	PA_OutputText(1,0,0,"                                ");
	PA_OutputText(1,0,1,"                                ");
#if PREPARE_TO_DISPLAY
	for (i=0;(i<preparePhrase(_list[_list_CurrentArticleNumber],1).length()) && (i<11);i++)
#else
	for (i=0;(i<_list[_list_CurrentArticleNumber].length()) && (i<11);i++)
#endif
	{
#if PREPARE_TO_DISPLAY
		PA_OutputText(1,3*i,0,"%x",preparePhrase(_list[_list_CurrentArticleNumber],1).at(i));
#else
		PA_OutputText(1,3*i,0,"%x",_list[_list_CurrentArticleNumber].at(i));
#endif
	}
	PA_OutputText(1,0,1,"%d/%d           ",_absolute_CurrentArticleNumber,_titleindex->NumberOfArticles());

}

u8 SearchResults::scrollLineUp()
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
		_absolute_FirstArticleNumber -= INCREASE_STEP;
		_list_FirstArticleNumber -= INCREASE_STEP;

		if (_absolute_FirstArticleNumber < 0)
		{
			_list_FirstArticleNumber -= _absolute_FirstArticleNumber;
			_absolute_FirstArticleNumber = 0;
		}

		int i;
		for (i=_absolute_CurrentArticleNumber;i>=_absolute_FirstArticleNumber;i--)
		{
			_list.push_front(_titleindex->getTitle(i));
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

u8 SearchResults::scrollLineDown()
{
	if (_absolute_CurrentArticleNumber >= _titleindex->NumberOfArticles() - 1)
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
		_list_LastArticleNumber += INCREASE_STEP;
		_absolute_LastArticleNumber += INCREASE_STEP;

		if (_absolute_LastArticleNumber > _titleindex->NumberOfArticles() - 1)
		{
			_absolute_LastArticleNumber = _titleindex->NumberOfArticles() - 1;
			_list_LastArticleNumber = _absolute_LastArticleNumber - _absolute_FirstArticleNumber;
		}

		int i;
		for (i=_absolute_CurrentArticleNumber;i<=_absolute_LastArticleNumber;i++)
		{
			_list.push_back(_titleindex->getTitle(i));
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

u8 SearchResults::scrollPageUp()
{
	u8 any = 0;
	u8 i;
	for (i=0;i<BIG_STEPSIZE;i++)
	{
		if (scrollLineUp())
		{
			any = 1;
		}
	}
	return any;
}

u8 SearchResults::scrollPageDown()
{
	u8 any = 0;
	u8 i;
	for (i=0;i<BIG_STEPSIZE;i++)
	{
		if (scrollLineDown())
		{
			any = 1;
		}
	}
	return any;
}

u8 SearchResults::scrollLongUp()
{
	load(_titleindex->getTitle(_absolute_CurrentArticleNumber-20*BIG_STEPSIZE));
	return 1;
}

u8 SearchResults::scrollLongDown()
{
	load(_titleindex->getTitle(_absolute_CurrentArticleNumber+20*BIG_STEPSIZE));
	return 1;
}
