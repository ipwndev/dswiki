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

class TextBox
{
	public:
		TextBox(vector<string> lines);
		string getChoice();
		void clear();
	private:
		vector<string> _lines;
};

#endif
