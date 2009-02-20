#ifndef _TITLEINDEX_H
#define _TITLEINDEX_H

#include <PA9.h>
#include <fat.h>
#include <string>
#include <vector>

class Globals;

using namespace std;

class ArticleSearchResult
{
	public:
		ArticleSearchResult(string title, string titleInArchive, u64 blockPos, int articlePos, int articleLength);

		string Title();
		string TitleInArchive();

		u64 BlockPos();
		int ArticlePos();
		int ArticleLength();

	private:
		string _title;
		string _titleInArchive;
		u64 _blockPos;
		int _articlePos;
		int _articleLength;
};


class TitleIndex
{
public:
	TitleIndex();
	~TitleIndex();

	void load(string basename, bool internal = false);

	string	getTitle(int articleNumber, unsigned char indexNo = 1, unsigned char setPosition = 0);

	ArticleSearchResult*	findArticle(string title, string previousTitle, unsigned char setPosition = 0);
	ArticleSearchResult*	getRandomArticle();
	ArticleSearchResult*	isRedirect(string markup);
	int		getSuggestedArticleNumber(string phrase, unsigned char indexNo = 1, unsigned char setPosition = 0);
	int		NumberOfArticles();
	string	ImageNamespace();
	string	TemplateNamespace();
	void setGlobals(Globals* globals);

private:
	FILE*   _f_header;
	FILE*   _f_dataindex;
	FILE*   _f_index0;
	FILE*   _f_index1;

	int     _numberOfArticles;
	unsigned char      _indexVersion;
	unsigned char      _using_index1;
	bool	isChinese;

	fpos_t	_titlesPos;

	u64		_lastBlockPos;
	int		_lastArticlePos;
	int		_lastArticleLength;

	string	_imageNamespace;
	string	_templateNamespace;
	Globals* _globals;
};

#endif
