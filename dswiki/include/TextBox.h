//
// C++ Interface: TextBox
//
// Description:
//
//
// Author: Oliver Gronau <ogronau@web.de>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef TEXTBOX_H_
#define TEXTBOX_H_

#include <PA9.h>
#include <vector>
#include <string>
#include "api.h"
#include "chrlib.h"

using namespace std;

class Globals;

class TextBox
{
	public:
		TextBox(vector<string> lines);
		void allowCancel(bool allowCancel) { _allowCancel = allowCancel; };
		void setGlobals(Globals* globals) { _globals = globals; }
		void setTitle(string title) {_title = title; };
		void maximize();
		int run();
		void setCurrentPosition(int pos);
	private:
		vector<string> _lines;
		string _title;
		int _topItem;
		int _currentItem;
		bool _allowCancel;
		VirScreen MaxPossibleSpace;
		VirScreen BoxSpace;
		VirScreen ContentSpace;
		CharStat LineCS;
		CharStat LineEmphCS;

		int boxDrawingWidth;
		int boxDrawingHeight;
		int boxDrawingCharsPerLine;
		int usedWidth;
		int _numlines;
		int usedHeight;

		Globals* _globals;
};

#endif
