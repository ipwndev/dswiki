//
// C++ Implementation: Cache
//
// Description:
//
//
// Author: Oliver Gronau <ogronau@web.de>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "Cache.h"

#include <PA9.h>
#include <string>
#include <deque>


// #define MAX_CACHE_SIZE 2097152
#define MAX_CACHE_SIZE 0

Cache::Cache()
{
	_size = 0;
	_lastFoundPosition = 0;
}

void Cache::clear()
{
	_list.clear();
	_size = 0;
	_lastFoundPosition = 0;
}

void Cache::insert(string title, string markup)
{
	return;
	CacheEntry newEntry = {title,markup};
	unsigned int newEntrySize = markup.length();
	while ((_size + newEntrySize > MAX_CACHE_SIZE) && (_list.size()>0) )
	{
		CacheEntry oldEntry = _list.front();
		_size -= oldEntry.markup.length();
		_list.pop_front();
	}
	_list.push_back(newEntry);
	_size += newEntrySize;
}

unsigned char Cache::isInCache(string title)
{
	unsigned int i;
	for (i=0;i<_list.size();i++)
	{
		if (_list[i].title == title)
		{
// 			PA_OutputText(1,23,0,"%s & %s",_list[i].title.c_str(),title.c_str());
			_lastFoundPosition = i;
			return 1;
		}
	}
	return 0;
}

string Cache::getMarkup(string title)
{
	return _list[_lastFoundPosition].markup;
}

void Cache::display()
{
	int i;
	PA_OutputText(1,0,23,"SIZE = %d, CAPACITY = %d",_size,MAX_CACHE_SIZE-_size);
	for (i=0;i<_list.size();i++)
	{
		PA_OutputText(1,0,22-i,"<%s>",_list[i].title.c_str());
	}
}
