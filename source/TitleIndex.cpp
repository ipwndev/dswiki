#include "TitleIndex.h"

typedef struct
{
	char languageCode[2];
	u32 numberOfArticles;
	fpos_t	titlesPos;
	fpos_t	indexPos_0;
	fpos_t	indexPos_1;
	unsigned char version;
	char reserved1[1];
	char imageNamespace[32];
	char templateNamespace[32];
	char reserved2[160];
} FILEHEADER;


TitleIndex::TitleIndex(string basename)
{
	_imageNamespace = "";
	_templateNamespace = "";
	isChinese = false;

	_numberOfArticles = -1;
	_using_index1 = 0;

	_imageNamespace = "";
	_templateNamespace = "";

	_FileName_Header    = basename + ".ifo";
	_FileName_Data      = basename + ".dbz";
	_FileName_DataIndex = basename + ".idx";
	_FileName_Index0    = basename + ".ao1";
	_FileName_Index1    = basename + ".ao2";
	_f_header           = fopen(_FileName_Header.c_str(),    "rb");
	_f_data             = fopen(_FileName_Data.c_str(),      "rb");
	_f_dataindex        = fopen(_FileName_DataIndex.c_str(), "rb");
	_f_index0           = fopen(_FileName_Index0.c_str(),    "rb");
	_f_index1           = fopen(_FileName_Index1.c_str(),    "rb");

	if ( _f_header )
	{
		int error = 0;

		FILEHEADER fileheader;

		fread(&fileheader.languageCode,      2, 1, _f_header);
		fread(&fileheader.numberOfArticles,  4, 1, _f_header);
		fread(&fileheader.titlesPos,         8, 1, _f_header);
		fread(&fileheader.indexPos_0,        8, 1, _f_header);
		fread(&fileheader.indexPos_1,        8, 1, _f_header);
		fread(&fileheader.version,           1, 1, _f_header);
		fread(&fileheader.reserved1,         1, 1, _f_header);
		fread(&fileheader.imageNamespace,    32, 1, _f_header);
		fread(&fileheader.templateNamespace, 32, 1, _f_header);
		fread(&fileheader.reserved2,         160, 1, _f_header);

		_numberOfArticles  = fileheader.numberOfArticles;
		_imageNamespace    = fileheader.imageNamespace;
		_templateNamespace = fileheader.templateNamespace;

		if (fileheader.indexPos_1)
		{
			_using_index1 = 1;
		}
	}
}

TitleIndex::~TitleIndex()
{
	if (_f_header)
	{
		fclose(_f_header);
	}
	if (_f_data)
	{
		fclose(_f_data);
	}
	if (_f_dataindex)
	{
		fclose(_f_dataindex);
	}
	if (_f_index0)
	{
		fclose(_f_index0);
	}
	if (_f_index1)
	{
		fclose(_f_index1);
	}
}

string TitleIndex::getTitle(int articleNumber, int indexNo, u8 setPosition)
{
	if ( _numberOfArticles<=0  )
		return "";

	if ((articleNumber<0) || (articleNumber>=_numberOfArticles))
		return "";

	FILE* f_index = _f_index0;
	if ( indexNo==1 && _using_index1 )
		f_index = _f_index1;

	int error = fseek(f_index, articleNumber*sizeof(int), SEEK_SET);
	if ( error )
		return "";

	int titlePos;
	size_t read = fread(&titlePos, sizeof(int), 1, f_index);

	if ( !read )
		return "";

	if (setPosition)
	{
		_lastBlockPos = 0;
		_lastArticlePos = 0;
		_lastArticleLength = 0;

		error = fseek(_f_dataindex, titlePos, SEEK_SET);
		if ( error )
			return "";

		fread(&_lastBlockPos, 8, 1, _f_dataindex);
		fread(&_lastArticlePos, 4, 1, _f_dataindex);
		fread(&_lastArticleLength, 4, 1, _f_dataindex);

		_lastBlockPos -= 256;
	}
	else
	{
		error = fseek(_f_dataindex, titlePos+16, SEEK_SET);
		if ( error )
			return "";
	}

	char readstr[MAX_TITLE_LENGTH+1];

	return string(fgets(readstr,MAX_TITLE_LENGTH+1,_f_dataindex));
}


//  to_lower_utf8(title)   :=   to_utf8(to_lower(from_utf8w(utf8_src)))
ArticleSearchResult* TitleIndex::findArticle(string title, u8 setPosition, u8 multiple)
{
	if ( _numberOfArticles<=0  )
		return NULL;

	string lowercaseTitle = title;
	strlwr(&lowercaseTitle.at(0)); // TODO CPPStringUtils::to_lower_utf8(title);

	int indexNo = 0;
	int foundAt = -1;
	int lBound = 0;
	int uBound = _numberOfArticles - 1;
	int index = 0;

	while ( lBound<=uBound )
	{
		index = (lBound + uBound) >> 1;

		// get the title at the specific index
		string titleAtIndex = getTitle(index, indexNo, setPosition);

		// make it lowercase and skip the prefix
		strlwr(&titleAtIndex.at(0)); // TODO CPPStringUtils::to_lower_utf8(title);

		if ( lowercaseTitle<titleAtIndex )
			uBound = index - 1;
		else if ( lowercaseTitle>titleAtIndex )
			lBound = ++index;
		else
		{
			foundAt = index;
			break;
		}
	}

	if ( foundAt<0 )
	{
		return NULL;
	}

	// check if there are more than one articles with the same lowercase name
	int startIndex = foundAt;
	while ( startIndex>0 )
	{
		string titleAtIndex = getTitle(startIndex-1, indexNo, setPosition);
		strlwr(&titleAtIndex.at(0)); // TODO CPPStringUtils::to_lower_utf8(titleAtIndex);

		if ( lowercaseTitle != titleAtIndex )
			break;

		startIndex--;
	}

	int endIndex = foundAt;
	while ( endIndex<(_numberOfArticles-1) )
	{
		string titleAtIndex = getTitle(endIndex+1, indexNo, setPosition);
		strlwr(&titleAtIndex.at(0)); // TODO CPPStringUtils::to_lower_utf8(titleAtIndex);

		if ( lowercaseTitle != titleAtIndex )
			break;

		endIndex++;
	}

	if ( !multiple )
	{
		// return a result only if we have a direct hit (case is taken into account)

		if ( startIndex!=endIndex )
		{
			// check if one matches 100%
			for(int i=startIndex; i<=endIndex; i++)
			{
				string titleInArchive = getTitle(i, indexNo, setPosition);
				if ( title==titleInArchive )
				{
					return new ArticleSearchResult(title, titleInArchive, _lastBlockPos, _lastArticlePos, _lastArticleLength);
				}
			}

			// nope, multiple matches
			return NULL;
		}
		else
		{
			// return the one and only result
			string titleInArchive = getTitle(foundAt, indexNo, setPosition);

			return new ArticleSearchResult(title, titleInArchive, _lastBlockPos, _lastArticlePos, _lastArticleLength);
		}
	}
	else
	{
		ArticleSearchResult* result = NULL;
		for(int i=startIndex; i<=endIndex; i++)
		{
			string titleInArchive = getTitle(i, indexNo, setPosition);

			if ( title == titleInArchive )
			{
				// 100% match
				return new ArticleSearchResult(title, titleInArchive, _lastBlockPos, _lastArticlePos, _lastArticleLength);
			}

			// collect the results
			if ( !result )
				result = new ArticleSearchResult(title, titleInArchive, _lastBlockPos, _lastArticlePos, _lastArticleLength);
			else
				result->Next = new ArticleSearchResult(title, titleInArchive, _lastBlockPos, _lastArticlePos, _lastArticleLength);
		}

		return result;
	}
	return NULL;
}


deque<string> TitleIndex::getSuggestions(string phrase, int before, int after)
{
	deque<string> s;

	if ( _numberOfArticles<=0  )
		return s;

	int indexNo = 0;
	if ( _using_index1 )
		indexNo = 1;

	string lowercasePhrase = preparePhrase(phrase);

	int phraseLength = phrase.length();
	if ( phraseLength==0 )
		return s;

	int lBound = 0;
	int uBound = _numberOfArticles - 1;
	int index = 0;
	int foundAt = -1;
	string titleAtIndex;

	while ( lBound<=uBound )
	{
		index = (lBound + uBound) >> 1;

		// get the title at the specific index
		titleAtIndex = getTitle(index, indexNo);
		titleAtIndex = preparePhrase(titleAtIndex);

		if ( lowercasePhrase < titleAtIndex)
			uBound = index - 1;
		else if ( lowercasePhrase > titleAtIndex )
			lBound = index + 1;
		else
		{
			foundAt = index;
			break;
		}
	}

	if ( foundAt<0 ) // not found
	{
		if ( lowercasePhrase > titleAtIndex )
		{
			index++;
		}
		foundAt = index;
	}

	int startIndex = foundAt;

	// go to the first article which starts with the phrase
	while ( startIndex>0 )
	{
		titleAtIndex = getTitle(startIndex-1, indexNo);
		titleAtIndex = preparePhrase(titleAtIndex);

		if ( titleAtIndex.length() > phraseLength )
			titleAtIndex.resize(phraseLength);

		if ( lowercasePhrase!=titleAtIndex )
			break;

		startIndex--;
	}

	index = startIndex;
	int results = 0;

	while ( (index<_numberOfArticles) && (results<after+1) )
	{
		string title = getTitle(index, indexNo);
		s.push_back(title);
		index++;
		results++;
	}

	index = startIndex-1;
	results = 0;

	while ( (index >= 0) && (results<before) )
	{
		string title = getTitle(index, indexNo);
		s.push_front(title);
		index--;
		results++;
	}

	return s;
}


ArticleSearchResult* TitleIndex::isRedirect(string markup)
{
	Element* l;
	if (markup.empty())
		return NULL;
	string lowercaseMarkup = markup.substr(0,9);
	strlwr(&lowercaseMarkup.at(0));

	if ( lowercaseMarkup == "#redirect" )
	{
		l = createLink(markup,9,0);
		if (l)
		{
			return findArticle(l->target);
		}
		else
		{
			return NULL;
		}
	}
	else
	{
		return NULL;
	}
}

ArticleSearchResult* TitleIndex::getRandomArticle()
{
	if ( _numberOfArticles<=0  )
		return NULL;

	int indexNo = 0;

	int articleNo = PA_RandMax(_numberOfArticles-1);

	string titleInArchive = getTitle(articleNo, indexNo);

	return new ArticleSearchResult(titleInArchive, titleInArchive, _lastBlockPos, _lastArticlePos, _lastArticleLength);
}

string TitleIndex::HeaderFileName()
{
	return _FileName_Header;
}

string TitleIndex::DataFileName()
{
	return _FileName_Data;
}

string TitleIndex::DataIndexFileName()
{
	return _FileName_DataIndex;
}

string TitleIndex::Index0FileName()
{
	return _FileName_Index0;
}

string TitleIndex::Index1FileName()
{
	return _FileName_Index1;
}

int TitleIndex::NumberOfArticles()
{
	return _numberOfArticles;
}

string TitleIndex::ImageNamespace()
{
	return _imageNamespace;
}

string TitleIndex::TemplateNamespace()
{
	return _templateNamespace;
}


ArticleSearchResult::ArticleSearchResult(string title, string titleInArchive, fpos_t blockPos, int articlePos, int articleLength)
{
	Previous = NULL;
	Next     = NULL;

	_title = title;
	_titleInArchive = titleInArchive;

	_blockPos = blockPos;
	_articlePos = articlePos;
	_articleLength = articleLength;
}

string ArticleSearchResult::Title()
{
	return _title;
}

string ArticleSearchResult::TitleInArchive()
{
	return _titleInArchive;
}


fpos_t ArticleSearchResult::BlockPos()
{
	return _blockPos;
}

int ArticleSearchResult::ArticlePos()
{
	return _articlePos;
}

int ArticleSearchResult::ArticleLength()
{
	return _articleLength;
}
