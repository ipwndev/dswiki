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

using namespace std;

class Globals;

class TextBox
{
	public:
		TextBox(vector<string> lines) { _lines = lines; _topItem = 0; _currentItem = 0; };
		TextBox(string oneliner) { _lines.push_back(oneliner); _topItem = 0; _currentItem = 0; };
		void allowCancel(unsigned char allowCancel);
		void setGlobals(Globals* globals) { _globals = globals; }
		void setTitle(string title) {_title = title; };
		void reset();
		string run();
	private:
		vector<string> _lines;
		string _title;
		int _topItem;
		int _currentItem;
		int _numlines;
		Globals* _globals;
		unsigned char _allowCancel;
};

#endif
