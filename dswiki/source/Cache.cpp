#include "Cache.h"

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
	CacheEntry newEntry = {title,markup};
	u32 newEntrySize = markup.length();
	while ((_size + newEntrySize > MAX_CACHE_SIZE) && (_list.size()>0) )
	{
		CacheEntry oldEntry = _list.front();
		_size -= oldEntry.markup.length();
		_list.pop_front();
	}
	_list.push_back(newEntry);
	_size += newEntrySize;
}

u8 Cache::isInCache(string title)
{
	u32 i;
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
	s32 i;
	PA_OutputText(1,0,23,"SIZE = %d, CAPACITY = %d",_size,MAX_CACHE_SIZE-_size);
	for (i=0;i<_list.size();i++)
	{
		PA_OutputText(1,0,22-i,"<%s>",_list[i].title.c_str());
	}
}
