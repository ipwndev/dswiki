//
// C++ Interface: Statusbar
//
// Description:
//
//
// Author: Oliver Gronau <ogronau@web.de>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef STATUSBAR_H_
#define STATUSBAR_H_

#include <PA9.h>
#include <string>

using namespace std;

class Statusbar
{
	public:
		void clear();
		void display(string message);
		void displayError(string message);
		void displayClearAfter(string message, int delay);
		void displayErrorClearAfter(string message, int delay);
		void clearAfter(int delay);
		static void removeIcons();
		static void showIcons();
	private:
};

#endif
