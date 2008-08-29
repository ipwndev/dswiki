#ifndef _CACHE_H
#define _CACHE_H

#include <PA9.h>
#include <string>
#include <vector>
#include <queue>
#include <deque>

#include "main.h"
#include "api.h"
#include "struct.h"
#include "chrlib.h"

#define MAX_CACHE_SIZE 2097152

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
