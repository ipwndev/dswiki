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

int TextBox::run()
{
	PA_WaitForVBL();
	if (_lines.empty())
		return -1;
	if (_lines.size() == 1)
		return 0;

	CharStat CS = NormalCS;
	CS.Wrap = NOWRAP;
	CS.BgColor = PA_RGB(28,28,28);

	FillVS(&BoxSpace,PA_RGB(28,28,28));

	BLOCK CharArea = {{0,0},{0,0}};

	iPrint("┌",&BoxSpace,&CS,&CharArea);
	for (int a=1;a+1<boxDrawingCharsPerLine;a++)
	{
		iPrint("─",&BoxSpace,&CS,&CharArea);
	}
	iPrint("┐",&BoxSpace,&CS,&CharArea);
	for (int a=1;a+1<_numlines+2;a++)
	{
		CharArea.Start.x = 0;
		CharArea.Start.y = a*CS.FONT->Height();
		iPrint("│",&BoxSpace,&CS,&CharArea);
		CharArea.Start.x = usedWidth-boxDrawingWidth;
		iPrint("│",&BoxSpace,&CS,&CharArea);
	}
	CharArea.Start.x = 0;
	CharArea.Start.y = usedHeight-CS.FONT->Height();
	iPrint("└",&BoxSpace,&CS,&CharArea);
	for (int a=1;a+1<boxDrawingCharsPerLine;a++)
	{
		iPrint("─",&BoxSpace,&CS,&CharArea);
	}
	iPrint("┘",&BoxSpace,&CS,&CharArea);
	if (!_title.empty())
	{
		CharArea.Start.x = 4*boxDrawingWidth;
		CharArea.Start.y = 0;
		CS.Fx = BACKGR;
		iPrint(" "+_title+" ",&BoxSpace,&CS,&CharArea);
		CS.Fx = NONE;
	}

	bool fullupdate = true;
	bool update = true;
	bool acceptShoulderbuttons = false;

	while(1)
	{
		int lineClicked = (Stylus.Y - ContentSpace.AbsoluteBound.Start.y + 100 * boxDrawingHeight) / boxDrawingHeight - 100;

		if (Stylus.Newpress)
		{
			if (_allowCancel &&
						 ((Stylus.X < BoxSpace.AbsoluteBound.Start.x-5)
						 || (Stylus.X > BoxSpace.AbsoluteBound.End.x+5)
						 || (Stylus.Y < BoxSpace.AbsoluteBound.Start.y-5)
						 || (Stylus.Y > BoxSpace.AbsoluteBound.End.y+5))
			   )
			{
				PA_WaitForVBL();
				return -1;
			}

			if (_topItem + lineClicked == _currentItem)
			{
				PA_WaitForVBL();
				return _currentItem;
			}
		}
		else if (Stylus.Held)
		{
			if (lineClicked < 0)
			{
				// scroll up and highlight the first row
				if (_topItem>0)
				{
					_topItem--;
					_currentItem = _topItem;
					update = true;
					fullupdate = true;
					PA_Sleep(5);
				}
				else if (_currentItem != _topItem)
				{
					_currentItem = _topItem; // = 0
					update = true;
				}
			}
			else if (lineClicked > _numlines - 1)
			{
				if (_topItem + _numlines < (int) _lines.size())
				{
					_topItem++;
					_currentItem = _topItem + _numlines - 1;
					update = true;
					fullupdate = true;
					PA_Sleep(5);
				}
				else if (_currentItem != _topItem + _numlines)
				{
					_currentItem = _topItem + _numlines - 1;
					if (_currentItem >= (int) _lines.size())
					{
						_currentItem = _lines.size() - 1;
					}
					update = true;
				}
			}
			else if (_currentItem != _topItem + lineClicked)
			{
				_currentItem = _topItem + lineClicked;
				if (_currentItem >= (int) _lines.size())
				{
					_currentItem = _lines.size() - 1;
				}
				update = true;
			}
		}

		if (Pad.Newpress.A)
		{
			PA_WaitForVBL();
			return _currentItem;
		}

		if (_allowCancel && (Pad.Newpress.B || Pad.Newpress.Start) )
		{
			PA_WaitForVBL();
			return -1;
		}

		if (Pad.Newpress.L || Pad.Newpress.R)
			acceptShoulderbuttons = true;

		if ((acceptShoulderbuttons && Pad.Held.L) || Pad.Held.Up)
		{
			if (_currentItem>0)
			{
				_currentItem--;
				if (_currentItem<_topItem)
				{
					_topItem--;
					fullupdate = true;
				}
				update = true;
				PA_Sleep(10);
			}
		}

		if ( (acceptShoulderbuttons && Pad.Held.R) || Pad.Held.Down)
		{
			if (_currentItem< (int) _lines.size()-1)
			{
				_currentItem++;
				if (_currentItem>_topItem+_numlines-1)
				{
					_topItem++;
					fullupdate = true;
				}
				update = true;
				PA_Sleep(10);
			}
		}

		if (Pad.Held.Left)
		{
			for (int a=0;a+1<_numlines;a++)
			{
				if (_currentItem>0)
				{
					_currentItem--;
					if (_currentItem<_topItem)
					{
						_topItem--;
						fullupdate = true;
					}
					update = true;
				}
			}
			PA_Sleep(10);
		}

		if (Pad.Held.Right)
		{
			for (int a=0;a+1<_numlines;a++)
			{
				if (_currentItem < (int) _lines.size()-1)
				{
					_currentItem++;
					if (_currentItem>_topItem+_numlines-1)
					{
						_topItem++;
						fullupdate = true;
					}
					update = true;
				}
			}
			PA_Sleep(10);
		}

		if (fullupdate)
		{
			FillVS(&ContentSpace,PA_RGB(28,28,28));
			CharArea.Start.x = 2*boxDrawingWidth;
			CharArea.Start.y = 0;
			CS.Fx = BACKGR;
			if (_topItem>0)
			{
				iPrint("↑",&BoxSpace,&CS,&CharArea);
			}
			else
			{
				iPrint("─",&BoxSpace,&CS,&CharArea);
			}
			CharArea.Start.x = 2*boxDrawingWidth;
			CharArea.Start.y = usedHeight-boxDrawingHeight;
			if ( (int) _lines.size()>_numlines+_topItem)
			{
				iPrint("↓",&BoxSpace,&CS,&CharArea);
			}
			else
			{
				iPrint("─",&BoxSpace,&CS,&CharArea);
			}
			CS.Fx = NONE;
			fullupdate = false;
		}
		if (update)
		{
			CharArea.clear();
			for (int i=_topItem; (i < (int) _lines.size())&&(i-_topItem<_numlines); i++)
			{
				if (i==_currentItem)
				{
					CS.Color = PA_RGB(31, 0, 0);
					iPrint(_lines[i]+"\n",&ContentSpace,&LineEmphCS,&CharArea);
					CS.Color = PA_RGB(0, 0, 0);
				}
				else
				{
					iPrint(_lines[i]+"\n",&ContentSpace,&LineCS,&CharArea);
				}
			}
			update = false;
		}

		PA_WaitForVBL();
	}

	PA_WaitForVBL();
	return -1;
}


void TextBox::setCurrentPosition(int pos)
{
	_currentItem = pos;
	if (_numlines >= (int) _lines.size())
	{
		// the only case where we can see both ends
		_topItem = 0;
	}
	else
	{
		// center it the best way possible
		_topItem = pos - _numlines/2;
		if (_topItem<0)
			_topItem = 0;
		if (_topItem > (int) _lines.size()-_numlines)
			_topItem = _lines.size()-_numlines;
	}
}

TextBox::TextBox(vector<string> lines)
{
	_lines = lines;
	_topItem = 0;
	_currentItem = 0;
	_allowCancel = false;

	VirScreen MaxPossibleSpace = {18, 10, 220, 156, {{0,0},{0,0}}, &DnScreen};
	InitVS(&MaxPossibleSpace);

	LineCS = NormalCS;
	LineCS.Wrap = NOWRAP;
	LineEmphCS = NormalCS;
	LineEmphCS.Color = PA_RGB(31,0,0);
	LineEmphCS.Wrap = NOWRAP;

	boxDrawingWidth = NormalCS.FONT->getCharacterWidth(0x2500);
	boxDrawingHeight = NormalCS.FONT->Height();
	boxDrawingCharsPerLine = MaxPossibleSpace.Width / boxDrawingWidth;
	usedWidth = boxDrawingCharsPerLine * boxDrawingWidth;
	_numlines = _lines.size();
	while ((2+_numlines)*boxDrawingHeight > (int) MaxPossibleSpace.Height)
		_numlines--;
	usedHeight = (2+_numlines)*boxDrawingHeight;

	BoxSpace = (VirScreen) {MaxPossibleSpace.Left+(MaxPossibleSpace.Width-usedWidth)/2, MaxPossibleSpace.Top+(MaxPossibleSpace.Height-usedHeight)/2, usedWidth, usedHeight, {{0,0},{0,0}}, &DnScreen};
	InitVS(&BoxSpace);

	ContentSpace = (VirScreen) {BoxSpace.Left+boxDrawingWidth, BoxSpace.Top+boxDrawingHeight, usedWidth-2*boxDrawingWidth, usedHeight-2*boxDrawingHeight, {{0,0},{0,0}}, &DnScreen};
	InitVS(&ContentSpace);
}

void TextBox::maximize()
{
	VirScreen MaxPossibleSpace = {18, 10, 220, 156, {{0,0},{0,0}}, &DnScreen};
	InitVS(&MaxPossibleSpace);

	_numlines = (MaxPossibleSpace.Height / boxDrawingHeight) - 2;
	usedHeight = (2+_numlines)*boxDrawingHeight;

	BoxSpace = (VirScreen) {MaxPossibleSpace.Left+(MaxPossibleSpace.Width-usedWidth)/2, MaxPossibleSpace.Top+(MaxPossibleSpace.Height-usedHeight)/2, usedWidth, usedHeight, {{0,0},{0,0}}, &DnScreen};
	InitVS(&BoxSpace);

	ContentSpace = (VirScreen) {BoxSpace.Left+boxDrawingWidth, BoxSpace.Top+boxDrawingHeight, usedWidth-2*boxDrawingWidth, usedHeight-2*boxDrawingHeight, {{0,0},{0,0}}, &DnScreen};
	InitVS(&ContentSpace);
}

