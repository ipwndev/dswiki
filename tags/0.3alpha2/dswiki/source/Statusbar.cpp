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

void Statusbar::removeIcons()
{
	PA_SetSpriteXY(0, SPRITE_FILEOPEN,  -16, -16);
	PA_SetSpriteXY(0, SPRITE_BOOKMARK,  -16, -16);
	PA_SetSpriteXY(0, SPRITE_VIEWMAG,   -16, -16);
	PA_SetSpriteXY(0, SPRITE_CONFIGURE, -16, -16);
}

void Statusbar::showIcons()
{
	PA_SetSpriteXY(0, SPRITE_FILEOPEN,    8, 176);
	PA_SetSpriteXY(0, SPRITE_BOOKMARK,   56, 176);
	PA_SetSpriteXY(0, SPRITE_VIEWMAG,    80, 176);
	PA_SetSpriteXY(0, SPRITE_CONFIGURE, 200, 176);
}

void Statusbar::clear()
{
	FillVS(&StatusbarVS,PA_RGB(26,26,26));
	showIcons();
}

void Statusbar::display(string message)
{
	clear();
	removeIcons();
	BLOCK CharArea = {{2,2},{0,0}};
	CharStat StatusbarCS = NormalCS;
	StatusbarCS.W_Space = 1;
	StatusbarCS.Color = PA_RGB( 5, 5, 5);
	StatusbarCS.Wrap = HARDWRAP;

	iPrint(message,&StatusbarVS,&StatusbarCS,&CharArea);
}

void Statusbar::displayError(string message)
{
	clear();
	removeIcons();
	BLOCK CharArea = {{2,2},{0,0}};
	CharStat StatErrorCS = NormalCS;
	StatErrorCS.Color = PA_RGB(27, 4, 4);
	StatErrorCS.Wrap = HARDWRAP;
	StatErrorCS.W_Space = 1;
	iPrint(message,&StatusbarVS,&StatErrorCS,&CharArea);
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
