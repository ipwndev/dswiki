#ifndef _TITLEINDEX_H
#define _TITLEINDEX_H

#include <fat.h>
#include <string>

using namespace std;

class ArticleSearchResult
{
	public:
		ArticleSearchResult(string title, string titleInArchive, fpos_t blockPos, int articlePos, int articleLength);

		string Title();
		string TitleInArchive();

		fpos_t BlockPos();
		int ArticlePos();
		int ArticleLength();

		ArticleSearchResult* Previous;
		ArticleSearchResult* Next;

	private:
		string _title;
		string _titleInArchive;
		fpos_t _blockPos;
		int _articlePos;
		int _articleLength;
};

class TitleIndex
{
public:
	TitleIndex(string basename);
	~TitleIndex();

	ArticleSearchResult* FindArticle(string title, u8 setPosition = 0, u8 multiple = 0);
	ArticleSearchResult* GetRandomArticle();
	ArticleSearchResult* isRedirect(string markup);
	ArticleSearchResult* GetSuggestions(string phrase, int maxSuggestions);

	void	DeleteSearchResult(ArticleSearchResult* articleSearchResult);

	string	HeaderFileName();
	string	DataFileName();
	string	DataIndexFileName();
	string	Index0FileName();
	string	Index1FileName();
	int		NumberOfArticles();
	string	ImageNamespace();
	string	TemplateNamespace();

private:
	FILE*   _f_header;
	FILE*   _f_data;
	FILE*   _f_dataindex;
	FILE*   _f_index0;
	FILE*   _f_index1;

	string	GetTitle(int articleNumber, int indexNo, u8 setPosition = 1);

	string	_FileName_Header;
	string	_FileName_Data;
	string	_FileName_DataIndex;
	string	_FileName_Index0;
	string	_FileName_Index1;

	int		_numberOfArticles;
	u8      _using_index1;
	bool	isChinese;

	fpos_t	_titlesPos;

	fpos_t	_lastBlockPos;
	int		_lastArticlePos;
	int		_lastArticleLength;

	string	_imageNamespace;
	string	_templateNamespace;
};

#endif
