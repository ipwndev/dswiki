#include <PA9.h>
#include "TitleIndex.h"

#include <sys/dir.h>
#include <unistd.h>
#include <algorithm>
#include "Markup.h"
#include "main.h"
#include "chrlib.h"
#include "char_convert.h"
#include "Globals.h"
#include "Dumps.h"

typedef struct
{
	char languageCode[2];
	unsigned int numberOfArticles;
	u64	titlesPos;
	u64	indexPos_0;
	u64	indexPos_1;
	unsigned char version;
	char reserved1[1];
	char imageNamespace[32];
	char templateNamespace[32];
	char reserved2[160];
} FILEHEADER;

void TitleIndex::load(string basename)
{
	chdir("fat:/dswiki/");
	_f_header           = fopen(_globals->getDumps()->get_ifo(basename).c_str(), "rb");
	_f_dataindex        = fopen(_globals->getDumps()->get_idx(basename).c_str(), "rb");
	_f_index0           = fopen(_globals->getDumps()->get_ao1(basename).c_str(), "rb");
	_f_index1           = fopen(_globals->getDumps()->get_ao2(basename).c_str(), "rb");

	if ( _f_header )
	{
		FILEHEADER fileheader;

		fread(&fileheader.languageCode,        2, 1, _f_header);
		fread(&fileheader.numberOfArticles,    4, 1, _f_header);
		fread(&fileheader.titlesPos,           8, 1, _f_header);
		fread(&fileheader.indexPos_0,          8, 1, _f_header);
		fread(&fileheader.indexPos_1,          8, 1, _f_header);
		fread(&fileheader.version,             1, 1, _f_header);
		fread(&fileheader.reserved1,           1, 1, _f_header);
		fread(&fileheader.imageNamespace,     32, 1, _f_header);
		fread(&fileheader.templateNamespace,  32, 1, _f_header);
		fread(&fileheader.reserved2,         160, 1, _f_header);

		_numberOfArticles  = fileheader.numberOfArticles;
		_imageNamespace    = fileheader.imageNamespace;
		_templateNamespace = fileheader.templateNamespace;
		_indexVersion      = fileheader.version;

		if (_indexVersion > MAX_INDEX_VERSION)
		{
			PA_Clear16bitBg(0);
			BLOCK CharArea = {{0,0},{0,0}};
			iPrint("'''Warning:''' The index version of this dump is higher than the maximal supported version. Loading/searching ''may'' fail. Please upgrade to the latest release of DSwiki, to get the best browsing experience.", &ContentWin2, &ErrorCS, &CharArea, -1, UTF8);
			PA_WaitFor(Pad.Newpress.Anykey || Stylus.Newpress);
			PA_Clear16bitBg(0);
		}

		if (fileheader.indexPos_1)
		{
			_using_index1 = 1;
		}
	}
}

TitleIndex::TitleIndex()
{
	_imageNamespace = "";
	_templateNamespace = "";
	isChinese = false;

	_numberOfArticles = -1;
	_using_index1 = 0;

	_imageNamespace = "";
	_templateNamespace = "";

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

string TitleIndex::getTitle(int articleNumber, unsigned char indexNo, unsigned char setPosition)
{
	if ( _numberOfArticles<=0  )
		return "";

	if (!(_using_index1))
		indexNo = 0;

	if ( articleNumber < 0 )
		articleNumber = 0;
	if ( articleNumber > _numberOfArticles - 1 )
		articleNumber = _numberOfArticles - 1;

	FILE* f_index = _f_index0;
	if ( indexNo==1 )
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


ArticleSearchResult* TitleIndex::findArticle(string title, string previousTitle, unsigned char setPosition)
{
	if ( _numberOfArticles<=0  )
		return NULL;

	if ( previousTitle.empty() ) // we have to find the exact title in the index
	{
		string lowercaseTitle = preparePhrase(title,0,_indexVersion);

		string titleAtIndex;

		int foundAt = -1;
		int lBound = 0;
		int uBound = _numberOfArticles - 1;
		int index = 0;

		while ( lBound<=uBound )
		{
			index = (lBound + uBound) >> 1;

		// get the title at the specific index
			titleAtIndex = getTitle(index, 0, setPosition);

		// make it lowercase and skip the prefix
			titleAtIndex = preparePhrase(titleAtIndex,0,_indexVersion);

			if ( lowercaseTitle<titleAtIndex )
				uBound = index-1;
			else if ( lowercaseTitle>titleAtIndex )
				lBound = index+1;
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
			titleAtIndex = getTitle(startIndex-1, 0, setPosition);
			titleAtIndex = preparePhrase(titleAtIndex,0,_indexVersion);

			if ( lowercaseTitle != titleAtIndex )
				break;

			startIndex--;
		}

		int endIndex = foundAt;
		while ( endIndex<(_numberOfArticles-1) )
		{
			titleAtIndex = getTitle(endIndex+1, 0, setPosition);
			titleAtIndex = preparePhrase(titleAtIndex,0,_indexVersion);

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
		// check if one matches 100%, except for the first character
			string lowerFirstTitle = lowerPhrase(title.substr(0,1),255)+title.substr(1);
			for(int i=startIndex; i<=endIndex; i++)
			{
				string titleInArchive = getTitle(i, 0, setPosition);
				if ( lowerFirstTitle==lowerPhrase(titleInArchive.substr(0,1),255)+titleInArchive.substr(1) )
				{
					return new ArticleSearchResult(title, titleInArchive, _lastBlockPos, _lastArticlePos, _lastArticleLength);
				}
			}
		// no match at all
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
	else // we try combinations of previousTitle and title (subpage feature)
	{
// 		PA_ClearTextBg(1);
// 		PA_OutputText(1,0,4,"%s",previousTitle.c_str());
// 		PA_OutputText(1,0,10,"%s",title.c_str());
// 		PA_Sleep(120);

		ArticleSearchResult* ret;

		// try the article name itself
		ret = findArticle(title,"",setPosition);
		if (ret)
			return ret;

		// is it a direct child?
		if ( title.substr(0,1) == "/" )
		{
			ret = findArticle(previousTitle+title,"",setPosition);
			if (ret)
				return ret;
		}

		// or is it a child of an ancestor?
		if ( title.substr(0,3) == "../" )
		{
			string titleCopy = title;
			string previousTitleCopy = previousTitle;
			int slashPos;

			while ( (titleCopy.substr(0,3) == "../") && ( ( slashPos = previousTitleCopy.rfind("/") ) != string::npos ) )
			{
				previousTitleCopy.erase(slashPos);
				titleCopy.erase(0,3);
			}

			if ( titleCopy.empty() )
				ret = findArticle(previousTitleCopy,"",setPosition);
			else
				ret = findArticle(previousTitleCopy+"/"+titleCopy,"",setPosition);

			if (ret)
				return ret;
		}

		// try subpage
		ret = findArticle(previousTitle+"/"+title,"",setPosition);
		if (ret)
			return ret;

		// or has it unnecessary slashes at the end
		if ( title.substr(title.length()-1) == "/" )
		{
			ret = findArticle(title.substr(0,title.length()-1),previousTitle,setPosition);
			if (ret)
				return ret;
		}

		return NULL;
	}
}

int TitleIndex::getSuggestedArticleNumber(string title, unsigned char indexNo, unsigned char setPosition)
{
	if ( _numberOfArticles<=0 )
		return 0;

	if ( title.empty() )
		return 0;

	if ( !(_using_index1) )
		indexNo = 0;

	string lowercaseTitle = preparePhrase(title, indexNo, _indexVersion);
	int titleLength = lowercaseTitle.length();

	string titleAtIndex;

	int foundAt = -1;
	int lBound = 0;
	int uBound = _numberOfArticles - 1;
	int index = 0;

	while ( lBound<=uBound )
	{
		index = (lBound + uBound) >> 1;

		// get the title at the specific index
		titleAtIndex = getTitle(index, indexNo, setPosition);
		titleAtIndex = preparePhrase(titleAtIndex, indexNo, _indexVersion);

		if ( lowercaseTitle < titleAtIndex)
			uBound = index-1;
		else if ( lowercaseTitle > titleAtIndex )
			lBound = index+1;
		else // lowercaseTitle == titleAtIndex
		{
			foundAt = index;
			break;
		}
	}

	if ( foundAt<0 ) // not found
	{
		if ( (lowercaseTitle > titleAtIndex) && (index < _numberOfArticles - 1) )
		{
			index++;
		}
		foundAt = index;
	}

	int startIndex = foundAt;

	// go to the first article which starts with the phrase
	while ( startIndex>0 )
	{
		titleAtIndex = getTitle(startIndex-1, indexNo, setPosition);
		titleAtIndex = preparePhrase(titleAtIndex, indexNo, _indexVersion);

		if ( titleAtIndex.length() > titleLength )
			titleAtIndex.resize(titleLength);

		if ( lowercaseTitle!=titleAtIndex )
			break;

		startIndex--;
	}

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
			return findArticle(l->target, "");
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

	string titleInArchive = getTitle(articleNo, 0, 1);

	return new ArticleSearchResult(titleInArchive, titleInArchive, _lastBlockPos, _lastArticlePos, _lastArticleLength);
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

void TitleIndex::setGlobals(Globals* globals)
{
	_globals = globals;
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
