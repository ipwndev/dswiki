#ifndef _WIKIMARKUPGETTER_H
#define _WIKIMARKUPGETTER_H

#include <PA9.h>
#include <string>

#include "TitleIndex.h"
#include "api.h"
#include "Big52Uni16.h"
#include "Cache.h"
#include "chrlib.h"
#include "History.h"
#include "main.h"
#include "Markup.h"
#include "SearchResults.h"
#include "struct.h"
#include "ter12rp.h"
#include "WikiMarkupGetter.h"

using namespace std;

class TitleIndex;

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
