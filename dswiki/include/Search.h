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
#include "main.h"

using namespace std;

class Globals;

class Search
{
	public:
		Search();
		void load(string phrase);
		void load(int articleNumber);
		string currentHighlightedItem();
		bool scrollLineDown();
		bool scrollLineUp();
		bool scrollPageDown();
		bool scrollPageUp();
		bool scrollLongDown();
		bool scrollLongUp();
		void display();
		static void removeIcons();
		static void showIcons();
		static void showUpdateIcons(bool updateInRealTime);
		void setGlobals(Globals* globals);
	private:
		void enlarge_front();
		void enlarge_back();
		deque<string>	_list;

		int _list_FirstArticleNumber;
		int _list_FirstDisplayNumber;
		int _list_CurrentArticleNumber;
		int _list_LastDisplayNumber;
		int _list_LastArticleNumber;
		int _absolute_FirstArticleNumber;
		int _absolute_FirstDisplayNumber;
		int _absolute_CurrentArticleNumber;
		int _absolute_LastDisplayNumber;
		int _absolute_LastArticleNumber;

		bool _wasScrolled;

		Globals* _globals;
};

#endif
