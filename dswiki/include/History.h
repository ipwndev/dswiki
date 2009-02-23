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

#ifndef _HISTORY_H
#define _HISTORY_H

#include <PA9.h>
#include <string>
#include <vector>

using namespace std;

typedef struct
{
	string title;
	int    line;
} HistoryPosition;

class History
{
	public:
		History();
		void					clear();
		string					currentTitle();
		int						currentLine();
		void					updateCurrentLine(int line);
		void					insert(string ins_str, int ins_line);
		bool					back();
		bool					forward();
		vector <string>			get();
		void					setCurrentPosition(int pos);
		int						getCurrentPosition();
		int						size();
	private:
		vector<HistoryPosition>	_list;
		int						_currentPosition;
};

#endif
