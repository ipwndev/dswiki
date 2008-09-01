#ifndef _SEARCHRESULTS_H
#define _SEARCHRESULTS_H

#include <PA9.h>
#include <string>
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
#include "ter12rp.h"
#include "TitleIndex.h"
#include "WikiMarkupGetter.h"

using namespace std;

#define MAX_SEARCH_RESULTS 128
#define INCREASE_STEP 13
#define BIG_STEPSIZE 13

//
// C++ Interface: SearchResults
//
// Description:
//
//
// Author: Oliver Gronau <ogronau@web.de>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

class TitleIndex;

class SearchResults
{
	public:
		SearchResults(TitleIndex* t, VirScreen* VScreen, CharStat* CStat1, CharStat* CStat2);
		void load(string phrase);
		string currentHighlightedItem();
		u8	scrollLineDown();
		u8	scrollLineUp();
		u8	scrollPageDown();
		u8	scrollPageUp();
		void display();
	private:
		TitleIndex*	_titleindex;
		VirScreen*	_vscreen;
		CharStat*	_cstat1;
		CharStat*	_cstat2;

		void enlarge_front();
		void enlarge_back();
		deque<string>	_list;

		s32 _list_FirstArticleNumber;
		s32 _list_FirstDisplayNumber;
		s32 _list_CurrentArticleNumber;
		s32 _list_LastDisplayNumber;
		s32 _list_LastArticleNumber;
		s32 _absolute_FirstArticleNumber;
		s32 _absolute_FirstDisplayNumber;
		s32 _absolute_CurrentArticleNumber;
		s32 _absolute_LastDisplayNumber;
		s32 _absolute_LastArticleNumber;

		u8	_wasScrolled;
};

#endif
