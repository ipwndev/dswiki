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

	if ( articleNumber < 0 )
		articleNumber = 0;
	if ( articleNumber > _numberOfArticles - 1 )
		articleNumber = _numberOfArticles - 1;

	FILE* f_index = _f_index0;
	if ( (indexNo==1) && (_using_index1) )
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

string TitleIndex::getTitle(int articleNumber)
{
	// Try index1, but the other getTitle has an integrated fallback to index0
	return getTitle(articleNumber, 1, 0);
}

ArticleSearchResult* TitleIndex::findArticle(string title, u8 setPosition)
{
	if ( _numberOfArticles<=0  )
		return NULL;

	string lowercaseTitle = lowerPhrase(title);

	int foundAt = -1;
	int lBound = 0;
	int uBound = _numberOfArticles - 1;
	int index = 0;

	while ( lBound<=uBound )
	{
		index = (lBound + uBound) >> 1;

		// get the title at the specific index
		string titleAtIndex = getTitle(index, 0, setPosition);

		// make it lowercase and skip the prefix
		titleAtIndex = lowerPhrase(titleAtIndex);

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
		string titleAtIndex = getTitle(startIndex-1, 0, setPosition);
		titleAtIndex = lowerPhrase(titleAtIndex);

		if ( lowercaseTitle != titleAtIndex )
			break;

		startIndex--;
	}

	int endIndex = foundAt;
	while ( endIndex<(_numberOfArticles-1) )
	{
		string titleAtIndex = getTitle(endIndex+1, 0, setPosition);
		titleAtIndex = lowerPhrase(titleAtIndex);

		if ( lowercaseTitle != titleAtIndex )
			break;

		endIndex++;
	}

	// return a result only if we have a direct hit (case is taken into account)

	if ( startIndex!=endIndex )
	{
		// check if one matches 100%
		for(int i=startIndex; i<=endIndex; i++)
		{
			string titleInArchive = getTitle(i, 0, setPosition);
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
		string titleInArchive = getTitle(foundAt, 0, setPosition);

		return new ArticleSearchResult(title, titleInArchive, _lastBlockPos, _lastArticlePos, _lastArticleLength);
	}
	return NULL;
}

int TitleIndex::getSuggestedArticleNumber(string phrase)
{
	int indexNo = 0;
	if ( _using_index1 )
		indexNo = 1;

	string lowercasePhrase = preparePhrase(phrase, indexNo);

	int phraseLength = phrase.length();
	if ( phraseLength==0 )
		return 0;

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
		titleAtIndex = preparePhrase(titleAtIndex, indexNo);

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
		titleAtIndex = preparePhrase(titleAtIndex, indexNo);

		if ( titleAtIndex.length() > phraseLength )
			titleAtIndex.resize(phraseLength);

		if ( lowercasePhrase!=titleAtIndex )
			break;

		startIndex--;
	}

	if (startIndex >= _numberOfArticles)
		startIndex = _numberOfArticles - 1;
	return startIndex;
}

ArticleSearchResult* TitleIndex::isRedirect(string markup)
{
	Element* l;
	if (markup.empty())
		return NULL;
	string lowercaseMarkup = lowerPhrase(markup.substr(0,9));

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

	int articleNo = PA_RandMax(_numberOfArticles-1);

	string titleInArchive = getTitle(articleNo, 0);

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

void TitleIndex::test()
{
	Device	UpScreen = {"U", 1, (u16*)PA_DrawBg[1], 256, 192};

	PA_OutputText(1,0,2,"Artikel %d",_numberOfArticles);
	int i;
	u8 indexNo;
	string vergleich[2];
	u8 first = 0;
	u8 anzfehler=0;
	for (indexNo=1;indexNo<=_using_index1;indexNo++)
	{
		PA_OutputText(1,0,3,"Pruefe Index %d",indexNo);
		first = 0;
		vergleich[first] = preparePhrase(getTitle(0,indexNo,0),indexNo);
		for (i=133400;i<_numberOfArticles;i++)
		{
			if ((i%100)==0)
				PA_OutputText(1,0,4,"%d      ",i);
			vergleich[1-first] = preparePhrase(getTitle(i,indexNo,0),indexNo);
			if (!(vergleich[first]<=vergleich[1-first]))
			{
				PA_OutputText(0,0,anzfehler,"Fehler bei %d/%d",i-1,i);
				anzfehler++;
				SimPrint("\n\n\n "+vergleich[first]+"\n "+vergleich[1-first],&UpScreen,PA_RGB(0,0,0),UTF8);
				if (anzfehler==24)
				{
					anzfehler = 0;
					PA_OutputText(1,0,5,"Taste druecken");
					PA_WaitFor(Pad.Newpress.Anykey);
					PA_OutputText(1,0,5,"              ");
				}
			}
			first = 1-first;
		}
	}
}
