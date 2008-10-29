//
// C++ Interface: Cache
//
// Description:
//
//
// Author: Oliver Gronau <ogronau@web.de>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef _CACHE_H
#define _CACHE_H

#include <PA9.h>
#include <string>
#include <deque>

using namespace std;

typedef struct
{
	string title;
	string markup;
} CacheEntry;

class Cache
{
	public:
		Cache();
		void clear();
		void insert(string title, string markup);
		u8   isInCache(string title);
		void display();
		string getMarkup(string title);
	private:
		deque<CacheEntry> _list;
		u32               _size;
		u32               _lastFoundPosition;
};

#endif
