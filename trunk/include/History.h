#ifndef _HISTORY_H
#define _HISTORY_H

#include <PA9.h>
#include <string>
#include <vector>

#include "api.h"
#include "Big52Uni16.h"
#include "Cache.h"
#include "chrlib.h"
#include "History.h"
#include "main.h"
#include "Markup.h"
#include "SearchResults.h"
#include "struct.h"
#include "ter12rp.h"
#include "TitleIndex.h"
#include "WikiMarkupGetter.h"

//
// C++ Interface: History
//
// Description:
//
//
// Author: Oliver Gronau <ogronau@web.de>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

using namespace std;

typedef struct
{
	string title;
	s32    line;
} HistoryPosition;

class History
{
	public:
		History();
		string currentTitle();
		s32    currentLine();
		void   updateCurrentLine(s32 line);
		void   insert(string ins_str, s32 ins_line);
		void   display();
		u8     back();
		u8     forward();
	private:
		vector<HistoryPosition> _list;
		u32                     _currentPosition;
};

#endif
