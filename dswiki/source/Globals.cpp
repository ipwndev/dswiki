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

using namespace std;

void Globals::setDumps(Dumps* dumps) { _dumps = dumps; }
void Globals::setTitleIndex(TitleIndex* titleIndex) { _titleIndex = titleIndex; }
void Globals::setWikiMarkupGetter(WikiMarkupGetter* wikiMarkupGetter) {	_wikiMarkupGetter = wikiMarkupGetter; }
void Globals::setMarkup(Markup* markup) { _markup = markup; }
void Globals::setSearch(Search* search) { _search = search; }
void Globals::setPercentIndicator(PercentIndicator* percentIndicator) { _percentIndicator = percentIndicator; }
void Globals::setStatusbar(Statusbar* statusbar) { _statusbar = statusbar; }
void Globals::setFont(Font* font) { _font = font; }
void Globals::setLanguage(int language) { _language = language; }

Dumps*            Globals::getDumps() {	return _dumps; }
TitleIndex*       Globals::getTitleIndex() { return _titleIndex; }
WikiMarkupGetter* Globals::getWikiMarkupGetter() { return _wikiMarkupGetter; }
Markup*           Globals::getMarkup() { return _markup; }
Search*           Globals::getSearch() { return _search; }
PercentIndicator* Globals::getPercentIndicator() { return _percentIndicator; }
Statusbar*        Globals::getStatusbar() { return _statusbar; }
Font*             Globals::getFont() { return _font; }
int               Globals::getLanguage() { return _language; }

void Globals::setOptions()
{
	vector<string> level1;
	level1.push_back("Help/Manual");
	level1.push_back("Invert color scheme");
	TextBox Options(level1);
	Options.setTitle("Configure DSwiki");
	Options.allowCancel(1);
	string choice = Options.run();
	if (choice == "Invert color scheme")
		toggleInverted();
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
	int i = size - 1;
	while ((i>=0) && (buffer[i]=='\n'))
		buffer[i--] = '\0';
	string bookmarkStr(buffer);
	free(buffer);
	buffer = NULL;
	vector<string> bookmarks;
	explode('\n',bookmarkStr,bookmarks);
	bookmarkStr.clear();

	TextBox BookmarkChooser(bookmarks);
	BookmarkChooser.setTitle("Load bookmark");
	BookmarkChooser.allowCancel(1);
	return BookmarkChooser.run();
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
		explode('\n',bookmarkStr,bookmarks);
		bookmarkStr.clear();
		fclose(bookmarkfile);
	}

	bookmarks.push_back(s);
	sort(bookmarks.begin(),bookmarks.end());
	string bookmarkStr;
	implode("\n",bookmarks,bookmarkStr);
	bookmarkfile = fopen("fat:/dswiki/bookmarks.txt","wb");
	if (bookmarkfile!=NULL)
	{
		fwrite(bookmarkStr.c_str(),strlen(bookmarkStr.c_str()),1,bookmarkfile);
		fclose(bookmarkfile);
	}
}

void Globals::toggleInverted()
{
	_isInverted = 1 - _isInverted;
	PA_SetBgPalCol(0, 0, backgroundColor());
	PA_SetBgPalCol(1, 0, backgroundColor());

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
		return PA_RGB(31,31,31);
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
