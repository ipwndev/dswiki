#include "TitleIndex.h"

typedef struct
{
	char languageCode[2];
	u32 numberOfArticles;
	u64	titlesPos;
	u64	indexPos_0;
	u64	indexPos_1;
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

	chdir("/dswiki/");
	_FileName_Header    = basename + ".ifo";
	_FileName_DataIndex = basename + ".idx";
	_FileName_Index0    = basename + ".ao1";
	_FileName_Index1    = basename + ".ao2";
	_f_header           = fopen(_FileName_Header.c_str(),    "rb");
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

void TitleIndex::setNew(string basename)
{
	if (_f_header)
	{
		fclose(_f_header);
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

	_imageNamespace = "";
	_templateNamespace = "";
	isChinese = false;

	_numberOfArticles = -1;
	_using_index1 = 0;

	_imageNamespace = "";
	_templateNamespace = "";

	chdir("/dswiki/");
	_FileName_Header    = basename + ".ifo";
	_FileName_DataIndex = basename + ".idx";
	_FileName_Index0    = basename + ".ao1";
	_FileName_Index1    = basename + ".ao2";
	_f_header           = fopen(_FileName_Header.c_str(),    "rb");
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


ArticleSearchResult::ArticleSearchResult(string title, string titleInArchive, u64 blockPos, int articlePos, int articleLength)
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


u64 ArticleSearchResult::BlockPos()
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

vector<string> TitleIndex::getPossibleWikis()
{
	vector<string> possibleWikis;
	struct stat st;
	char filename[256]; // to hold a full filename and string terminator
	DIR_ITER* dir = diropen("/dswiki/");

	if (dir == NULL)
		dir = diropen("/");

	if (dir == NULL)
	{
		PA_OutputText(1,2,2,"Unable to open the directory.");
	}
	else
	{
		while (dirnext(dir, filename, &st) == 0)
		{
			if (!(st.st_mode & S_IFDIR)) // regular file
			{
				string filenamestr = filename;
				if (filenamestr.rfind(".ifo")==filenamestr.length()-4)
				{
					string basename = filenamestr.substr(0,filenamestr.length()-4);
					struct stat inner_st;
					char inner_filename[256];
					DIR_ITER* inner_dir = diropen("/dswiki/");
					if (inner_dir == NULL)
						inner_dir = diropen("/");
					u8 foundidx = 0;
					u8 foundao1 = 0;
					u8 foundao2 = 0;
					u8 founddb  = 0;
					while (dirnext(inner_dir, inner_filename, &inner_st) == 0)
					{
						if (!(inner_st.st_mode & S_IFDIR)) // regular file
						{
							string inner_filenamestr = inner_filename;
							if (inner_filenamestr==basename+".idx")
								foundidx = 1;
							if (inner_filenamestr==basename+".ao1")
								foundao1 = 1;
							if (inner_filenamestr==basename+".ao2")
								foundao2 = 1;
							if (inner_filenamestr.substr(0,inner_filenamestr.length()-1)==basename+".db")
							{
								if (inner_filenamestr.substr(inner_filenamestr.length()-1,1).find_first_of("abcdefghijklmnopqrstuvwxyz")!=string::npos)
									founddb = 1;
							}
						}
					}
					if (foundidx && foundao1 && founddb)
						possibleWikis.push_back(basename);
				}
			}
		}
	}
	return possibleWikis;
}

void TitleIndex::test(Font f)
{
	set<u32> baddies;
	int i;
	string title;
	u32 Uni = 0;
	u32 Skip = 0;
	u32 length = 0;
	u32 ref;
	u32 replaceref = f.Index[0xFFFD];
	for (i=0;i<_numberOfArticles;i++)
	{
		if ((i%100)==0) {
			PA_OutputText(1,0,0,"%d/%d",i,_numberOfArticles);
		}
		title = getTitle(i,0,0);
		length = title.length();
		Skip = 0;
		while (Skip<length)
		{
			Skip+=ToUTF(&title[Skip],&Uni,B2U16,UTF8);
			ref = f.Index[Uni];
			if (ref==replaceref)
			{
				baddies.insert(Uni);
				PA_OutputText(1,5,5,"%d",baddies.size());
			}
		}
	}
}
