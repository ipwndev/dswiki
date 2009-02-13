//
// C++ Implementation: TextBox
//
// Description:
//
//
// Author: Oliver Gronau <ogronau@web.de>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <PA9.h>
#include "TextBox.h"
#include "main.h"
#include "api.h"

string TextBox::run()
{
	PA_WaitForVBL();
	if (_lines.empty())
		return "";

	VirScreen TextboxOuterBorder = {18, 10, 220, 156, {{0,0},{0,0}}, &DnScreen};
	InitVS(&TextboxOuterBorder);
	FillVS(&TextboxOuterBorder,PA_RGB(28,28,28));

	BLOCK border = TextboxOuterBorder.RelativeBound(-7);
	DrawBlock (&TextboxOuterBorder, border, PA_RGB(0,0,0), 0);

	CharStat neu = SearchResultsCS1;
	neu.BgColor = PA_RGB(28,28,28);
	neu.Fx = BACKGR;
	BLOCK CharArea = {{23,0},{0,0}};
	if (!_title.empty())
		iPrint(" "+_title+" ",&TextboxOuterBorder,&neu,&CharArea,-1,UTF8);

	VirScreen inner = {0,0,0,0, TextboxOuterBorder.AbsoluteBlock(TextboxOuterBorder.RelativeBound(-14)), &DnScreen};
	InitVS2(&inner);
	_numlines = inner.Height/SearchResultsCS1.FONT->Height();
	unsigned char fullupdate = 1;
	unsigned char update = 1;
	while(1)
	{
		if (Stylus.Held)
		{
			int y = Stylus.Y;
			y -= inner.AbsoluteBound.Start.y;
			int lineClicked = y / SearchResultsCS1.FONT->Height();
			if (Stylus.Newpress)
			{
				if (_allowCancel &&
					((Stylus.X < TextboxOuterBorder.AbsoluteBound.Start.x-5)
								|| (Stylus.X > TextboxOuterBorder.AbsoluteBound.End.x+5)
								|| (Stylus.Y < TextboxOuterBorder.AbsoluteBound.Start.y-5)
								|| (Stylus.Y > TextboxOuterBorder.AbsoluteBound.End.y+5))
				   )
				{
					PA_WaitForVBL();
					return "";
				}
				if (_topItem + lineClicked == _currentItem)
				{
					PA_WaitForVBL();
					return _lines[_currentItem];
				}
			}
			if (_currentItem != _topItem + lineClicked)
			{
				_currentItem = _topItem + lineClicked;
				if (_currentItem>=_lines.size()-1)
					_currentItem =_lines.size()-1;
				update = 1;
			}
		}
		if (Pad.Newpress.A)
		{
			PA_WaitForVBL();
			return _lines[_currentItem];
		}
		if (_allowCancel && (Pad.Newpress.B || Pad.Newpress.Start) )
		{
			PA_WaitForVBL();
			return "";
		}
		if (Pad.Held.Up && (_currentItem>0))
		{
			_currentItem--;
			if (_currentItem<_topItem)
			{
				_topItem--;
				fullupdate = 1;
			}
			update = 1;
			PA_Sleep(10);
		}
		if (Pad.Held.Down && (_currentItem<_lines.size()-1))
		{
			_currentItem++;
			if (_currentItem>_topItem+_numlines-1)
			{
				_topItem++;
				fullupdate = 1;
			}
			update = 1;
			PA_Sleep(10);
		}
		if (fullupdate)
		{
			FillVS(&inner,PA_RGB(28,28,28));
			if (_topItem>0)
			{
				CharArea = (BLOCK) {{13,0},{0,0}};
// 				iPrint("↑",&TextboxOuterBorder,&neu,&CharArea,-1,UTF8);
			}
			if (_lines.size()>_numlines+_topItem)
			{
				CharArea = (BLOCK) {{13,TextboxOuterBorder.Height-14},{0,0}};
// 				iPrint("↓",&TextboxOuterBorder,&neu,&CharArea,-1,UTF8);
			}
			fullupdate = 0;
		}
		if (update)
		{
			CharArea.clear();
			for (int i=_topItem; (i<_lines.size())&&(i-_topItem<_numlines); i++)
			{
				if (i==_currentItem)
					iPrint(_lines[i]+"\n",&inner,&SearchResultsCS2,&CharArea,-1,UTF8);
				else
					iPrint(_lines[i]+"\n",&inner,&SearchResultsCS1,&CharArea,-1,UTF8);
			}
			update = 0;
		}

		PA_WaitForVBL();
	}

	PA_WaitForVBL();
	return "";
}

void TextBox::allowCancel(unsigned char allowCancel)
{
	_allowCancel = allowCancel;
}

void TextBox::reset()
{
	_currentItem = 0;
	_topItem = 0;
}
