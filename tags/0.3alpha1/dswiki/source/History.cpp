#include "History.h"

#include <PA9.h>
#include <string>
#include <vector>

//
// C++ Implementation: History
//
// Description:
//
//
// Author: Oliver Gronau <ogronau@web.de>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
History::History()
{
	_currentPosition = -1;
}

void History::clear()
{
	_list.clear();
	_currentPosition = -1;
}

int History::size()
{
	return _list.size();
}

bool History::back()
{
	if ((_list.size()<2)||(_currentPosition==0))
		return false;
	_currentPosition--;
	return true;
}

bool History::forward()
{
	if ((_list.size()<2)||(_currentPosition==(int) _list.size()-1))
		return false;
	_currentPosition++;
	return true;
}

void History::insert(string ins_str, int ins_line)
{
	while ((int) _list.size()>_currentPosition+1)
		_list.pop_back();
	HistoryPosition newHistoryEntry = {ins_str,ins_line};
	_list.push_back(newHistoryEntry);
	_currentPosition++;
}

void History::updateCurrentLine(int line)
{
	_list[_currentPosition].line = line;
}

string History::currentTitle()
{
	return _list[_currentPosition].title;
}

int History::currentLine()
{
	return _list[_currentPosition].line;
}

vector<string> History::get()
{
	vector<string> tmp;
	for (int a=0; a < (int) _list.size(); a++)
	{
		tmp.push_back(_list[a].title);
	}
	return tmp;
}

void History::setCurrentPosition(int pos)
{
	_currentPosition = pos;
}

int History::getCurrentPosition()
{
	return _currentPosition;
}
