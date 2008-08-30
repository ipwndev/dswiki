#ifndef _WIKIMARKUPGETTER_H
#define _WIKIMARKUPGETTER_H

#include "TitleIndex.h"
#include <string>

using namespace std;

class WikiMarkupGetter
{
public:
	WikiMarkupGetter(string basename);
	~WikiMarkupGetter();

	string getMarkup(TitleIndex* t, string title);
	string GetLastArticleTitle();

private:
	FILE*   _f_data;
	string	_FileName_Data;
	string _lastArticleTitle;
};

#endif
