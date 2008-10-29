//
// C++ Interface: Search
//
// Description:
//
//
// Author: Oliver Gronau <ogronau@web.de>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef _SEARCHRESULTS_H
#define _SEARCHRESULTS_H

#include <PA9.h>
#include <string>
#include <deque>
#include "api.h"
#include "chrlib.h"
#include "main.h"

using namespace std;

class TitleIndex;

class Search
{
	public:
		Search(TitleIndex* t, CharStat* CStat1, CharStat* CStat2);
		void load(string phrase);
		void load(s32 articleNumber);
		string currentHighlightedItem();
		u8	scrollLineDown();
		u8	scrollLineUp();
		u8	scrollPageDown();
		u8	scrollPageUp();
		u8	scrollLongDown();
		u8	scrollLongUp();
		void display();
	private:
		TitleIndex*	_titleindex;
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
