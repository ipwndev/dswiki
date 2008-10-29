//
// C++ Implementation: Statusbar
//
// Description:
//
//
// Author: Oliver Gronau <ogronau@web.de>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "Statusbar.h"
#include "main.h"

void Statusbar::clear()
{
	FillVS(&StatusbarVS,PA_RGB(26,26,26));
}

void Statusbar::display(string message)
{
	clear();
	BLOCK CharArea = {{2,2},{0,0}};
	iPrint(message,&StatusbarVS,&StatusbarCS,&CharArea,-1,UTF8);
}

void Statusbar::displayError(string message)
{
	clear();
	BLOCK CharArea = {{2,2},{0,0}};
	iPrint(message,&StatusbarVS,&StatErrorCS,&CharArea,-1,UTF8);
}

void Statusbar::displayClearAfter(string message, int delay)
{
	display(message);
	clearAfter(delay);
}

void Statusbar::clearAfter(int delay)
{
	PA_Sleep(delay);
	clear();
}

void Statusbar::displayErrorClearAfter(string message, int delay)
{
	displayError(message);
	clearAfter(delay);
}
