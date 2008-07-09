#ifndef WIKIMARKUPGETTER_H
#define WIKIMARKUPGETTER_H

#include "TitleIndex.h"

using namespace std;

class WikiMarkupGetter
{
public:
	WikiMarkupGetter(char* language_code);
	~WikiMarkupGetter();

	char* GetMarkupForArticle(ArticleSearchResult* articleSearchResult);
	char* GetLastArticleTitle();

private:
	char* _languageCode;
	char _lastArticleTitle[1000];
};

#endif
