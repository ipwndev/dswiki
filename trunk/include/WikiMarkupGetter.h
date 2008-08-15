#ifndef WIKIMARKUPGETTER_H
#define WIKIMARKUPGETTER_H

#include "TitleIndex.h"
#include <string>

using namespace std;

class WikiMarkupGetter
{
public:
	WikiMarkupGetter(string basename);
	~WikiMarkupGetter();

	string GetMarkupForArticle(ArticleSearchResult* articleSearchResult);
	string GetLastArticleTitle();

private:
	FILE*   _f_data;
	string	_FileName_Data;
	string _lastArticleTitle;
};

#endif
