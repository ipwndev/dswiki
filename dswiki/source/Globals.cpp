//
// C++ Implementation: Globals
//
// Description:
//
//
// Author: Oliver Gronau <ogronau@web.de>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "Globals.h"
#include "TextBox.h"
#include <string>
#include <fat.h>
#include "WIKI2XML_global.h"
#include <algorithm>
#include "Statusbar.h"

using namespace std;

void Globals::setDumps(Dumps* dumps) { _dumps = dumps; }
void Globals::setTitleIndex(TitleIndex* titleIndex) { _titleIndex = titleIndex; }
void Globals::setWikiMarkupGetter(WikiMarkupGetter* wikiMarkupGetter) {	_wikiMarkupGetter = wikiMarkupGetter; }
void Globals::setMarkup(Markup* markup) { _markup = markup; }
void Globals::setPercentIndicator(PercentIndicator* percentIndicator) { _percentIndicator = percentIndicator; }
void Globals::setStatusbar(Statusbar* statusbar) { _statusbar = statusbar; }
void Globals::setLanguage(int language) { _language = language; }

Dumps*            Globals::getDumps() {	return _dumps; }
TitleIndex*       Globals::getTitleIndex() { return _titleIndex; }
WikiMarkupGetter* Globals::getWikiMarkupGetter() { return _wikiMarkupGetter; }
Markup*           Globals::getMarkup() { return _markup; }
PercentIndicator* Globals::getPercentIndicator() { return _percentIndicator; }
Statusbar*        Globals::getStatusbar() { return _statusbar; }
int               Globals::getLanguage() { return _language; }

void Globals::setFont(Font* font, FontCut type)
{
	switch (type)
	{
		case FONT_R:
			_font_r = font;
			break;
		case FONT_B:
			_font_b = font;
			break;
		case FONT_O:
			_font_o = font;
			break;
		case FONT_BO:
			_font_bo = font;
			break;
	}
}

Font* Globals::getFont(FontCut type)
{
	switch (type)
	{
		case FONT_R:
			return _font_r;
			break;
		case FONT_B:
			return _font_b;
			break;
		case FONT_O:
			return _font_o;
			break;
		case FONT_BO:
			return _font_bo;
			break;
	}
	return NULL;
}

string Globals::loadBookmark()
{
	FILE* bookmarkfile = fopen("fat:/dswiki/bookmarks.txt","rb");
	if (bookmarkfile==NULL)
	{
		return "";
	}
	fseek(bookmarkfile,0,SEEK_END);
	int size = ftell(bookmarkfile);
	fseek(bookmarkfile,0,SEEK_SET);
	char* buffer = (char*) malloc(size+1);
	fread(buffer,size,1,bookmarkfile);
	buffer[size] = '\0';

	string bookmarkStr(buffer);
	free(buffer);
	buffer = NULL;

	vector<string> bookmarks;
	explode("\n",bookmarkStr,bookmarks);
	bookmarkStr.clear();

	for (vector<string>::iterator it = find(bookmarks.begin(),bookmarks.end(),""); it != bookmarks.end(); it = find(bookmarks.begin(),bookmarks.end(),"") )
	{
		bookmarks.erase(it);
	}

	TextBox BookmarkChooser(bookmarks);
	BookmarkChooser.setTitle("Load bookmark");
	BookmarkChooser.allowCancel(true);
	int chosen = BookmarkChooser.run();
	if (chosen >= 0)
		return bookmarks[chosen];
	else
		return "";
}

void Globals::saveBookmark(string s)
{
	vector<string> bookmarks;

	FILE* bookmarkfile = fopen("fat:/dswiki/bookmarks.txt","rb");
	if (bookmarkfile!=NULL)
	{
		fseek(bookmarkfile,0,SEEK_END);
		int size = ftell(bookmarkfile);
		fseek(bookmarkfile,0,SEEK_SET);
		char* buffer = (char*) malloc(size+1);
		fread(buffer,size,1,bookmarkfile);
		buffer[size] = '\0';
		string bookmarkStr(buffer);
		free(buffer);
		buffer = NULL;
		explode("\n",bookmarkStr,bookmarks);
		for (vector<string>::iterator it = find(bookmarks.begin(),bookmarks.end(),""); it != bookmarks.end(); it = find(bookmarks.begin(),bookmarks.end(),"") )
		{
			bookmarks.erase(it);
		}
		bookmarkStr.clear();
		fclose(bookmarkfile);
	}

	if ( find(bookmarks.begin(),bookmarks.end(),s) == bookmarks.end())
	{
		bookmarks.push_back(s);
		sort(bookmarks.begin(),bookmarks.end());
		string bookmarkStr;
		implode("\n",bookmarks,bookmarkStr);
		bookmarkfile = fopen("fat:/dswiki/bookmarks.txt","wb");
		if (bookmarkfile!=NULL)
		{
			fwrite(bookmarkStr.c_str(),strlen(bookmarkStr.c_str()),1,bookmarkfile);
			fclose(bookmarkfile);
			_statusbar->displayClearAfter("Bookmark added",40);
		}
	}
	else
	{
		_statusbar->displayClearAfter("Bookmark already present",40);
	}
}

void Globals::toggleInverted()
{
	_isInverted = !_isInverted;
	PA_SetBgPalCol(0, 0, backgroundColor());
	PA_SetBgPalCol(1, 0, backgroundColor());
	if (_isInverted)
	{
		PA_SetTextCol (0, 31, 31, 31);
		PA_SetTextCol (1, 31, 31, 31);
	}
	else
	{
		PA_SetTextCol (0, 0, 0, 0);
		PA_SetTextCol (1, 0, 0, 0);
	}
}

int Globals::backgroundColor()
{
	if (!_isInverted)
	{
		return PA_RGB(31,31,31);
	}
	else
	{
		return PA_RGB(0,0,0);
	}
}

int Globals::textColor()
{
	if (!_isInverted)
	{
		return PA_RGB(0,0,0);
	}
	else
	{
		return PA_RGB(26,26,26);
	}
}

int Globals::linkColor()
{
	if (!_isInverted)
	{
		return PA_RGB(0,5,23);
	}
	else
	{
		return PA_RGB(7,7,31);
	}
}

int Globals::activeLinkColor()
{
	if (!_isInverted)
	{
		return PA_RGB(31,0,0);
	}
	else
	{
		return PA_RGB(31,0,0);
	}
}

int Globals::templateColor()
{
	if (!_isInverted)
	{
		return PA_RGB(24,0,24);
	}
	else
	{
		return PA_RGB(24,0,24);
	}
}

int Globals::imageColor()
{
	if (!_isInverted)
	{
		return PA_RGB(0,22,0);
	}
	else
	{
		return PA_RGB(0,19,0);
	}
}
