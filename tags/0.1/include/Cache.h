#ifndef _CACHE_H
#define _CACHE_H

#include <PA9.h>
#include <string>
#include <vector>
#include <deque>

#include "api.h"
#include "Big52Uni16.h"
#include "Cache.h"
#include "chrlib.h"
#include "History.h"
#include "main.h"
#include "Markup.h"
#include "SearchResults.h"
#include "struct.h"
#include "TitleIndex.h"
#include "WikiMarkupGetter.h"

using namespace std;

#define MAX_CACHE_SIZE 2097152
// #define MAX_CACHE_SIZE 0

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
