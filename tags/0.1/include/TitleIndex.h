#ifndef TITLEINDEX_H
#define TITLEINDEX_H

using namespace std;

class ArticleSearchResult
{
	public:
		ArticleSearchResult(char* title, char* titleInArchive, fpos_t blockPos, int articlePos, int articleLength);

		char* Title();
		char* TitleInArchive();

		fpos_t BlockPos();
		int ArticlePos();
		int ArticleLength();

		ArticleSearchResult* Previous;
		ArticleSearchResult* Next;

	private:
		char _title[1001];
		char _titleInArchive[1001];
		fpos_t _blockPos;
		int _articlePos;
		int _articleLength;
};

class TitleIndex
{
public:
	TitleIndex(char* nameOfDataFile, char* nameOfIndexFile);
	~TitleIndex();

	ArticleSearchResult* FindArticle(char* title, bool multiple=false);
	ArticleSearchResult* GetRandomArticle();
	ArticleSearchResult* isRedirect(char* markup);
	ArticleSearchResult* GetSuggestions(char* phrase, int maxSuggestions);

	void	DeleteSearchResult(ArticleSearchResult* articleSearchResult);

	char*	DataFileName();
	char*	IndexFileName();
	int		NumberOfArticles();
	char*	ImageNamespace();
	char*	TemplateNamespace();

private:
	char*	GetTitle(FILE* f, int articleNumber, int indexNo);

	char*	_dataFileName;
	char*	_indexFileName;
	int		_numberOfArticles;
	bool	isChinese;

	fpos_t	_titlesPos;
	fpos_t	_indexPos_0;
	fpos_t	_indexPos_1;

	fpos_t	_lastBlockPos;
	int		_lastArticlePos;
	int		_lastArticleLength;

	char*	_imageNamespace;
	char*	_templateNamespace;
	int iterations;
};

#endif
