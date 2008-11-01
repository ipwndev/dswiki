#include "WikiMarkupGetter.h"

#include <sys/dir.h>
#include <stdio.h>
#include <stdlib.h>
#include <bzlib.h>
#include <string>

#include "main.h"
#include "TitleIndex.h"
#include "Globals.h"
#include "PercentIndicator.h"
#include "Dumps.h"

#define BUFFER_SIZE 8192
#define BZ_DECOMPRESS_SMALL 1


WikiMarkupGetter::WikiMarkupGetter()
{
}

void WikiMarkupGetter::load(string basename)
{
	vector<string> wikidbs = _globals->getDumps()->get_dbs(basename);
	for (int i=0;i<wikidbs.size();i++)
	{
		FILE* _f = fopen(wikidbs[i].c_str(), "rb");
		_filepointers.push_back(_f);
		fseek(_f,0,SEEK_END);
		fpos_t size = ftell(_f);
		fseek(_f,0,SEEK_SET);
		_filesizes.push_back(size);
		u64 absoluteEnd;
		if (i==0)
			absoluteEnd = size-1;
		else
			absoluteEnd = _file_absoluteEnds[i-1] + size;
		_file_absoluteEnds.push_back(absoluteEnd);
	}
}

WikiMarkupGetter::~WikiMarkupGetter()
{
	for (int i=0;i<_filepointers.size();i++)
	{
		fclose(_filepointers[i]);
	}
	_filepointers.clear();
	_filesizes.clear();
	_file_absoluteEnds.clear();
	_lastArticleTitle.clear();
}

string WikiMarkupGetter::getMarkup(string title)
{
	ArticleSearchResult* articleSearchResult = _globals->getTitleIndex()->findArticle(title, 1); // The only situation when we need the physical position of the article afterwards
	if ( !articleSearchResult )
	{
		return NULL;
	}

	u64 blockPos	= articleSearchResult->BlockPos();
	int articlePos	= articleSearchResult->ArticlePos();
	int articleLength	= articleSearchResult->ArticleLength();
	_lastArticleTitle	= articleSearchResult->TitleInArchive();

// 	PA_OutputText(1,5,6,"0x%x        ",blockPos);
// 	PA_OutputText(1,5,7,"%d        ",articlePos);
// 	PA_OutputText(1,5,8,"%d        ",articleLength);
// 	PA_OutputText(1,5,9,"%s        ",_lastArticleTitle.c_str());

	int fileNo = 0;
	while (_file_absoluteEnds[fileNo]<blockPos) fileNo++;
	for (int i=0;i<fileNo;i++)
		blockPos -= _filesizes[i];
	FILE* _f_data = _filepointers[fileNo];

	if ( !_f_data ) {
// 		PA_OutputText(1,5,10,"!fdata");
		return "";
	}
	// seek to the block
	fseek(_f_data, blockPos, SEEK_SET);

	// open the block
	int bzerror;
	BZFILE *bzf = BZ2_bzReadOpen(&bzerror, _f_data, 0, BZ_DECOMPRESS_SMALL, NULL, 0);
	if (bzerror != BZ_OK) {
// 		PA_OutputText(1,5,10,"BZ_ReadOpen failed");
		BZ2_bzReadClose ( &bzerror, bzf );
		return "";
	}

// 	PA_OutputText(1,5,11,"Bis kurz vorm Loop");
// 	PA_Sleep(60);
// 	PA_OutputText(1,5,11,"                  ");
	char buffer[BUFFER_SIZE];
	string markup = "";
	int read = 0;

	while ( read = BZ2_bzRead(&bzerror, bzf, buffer, BUFFER_SIZE) )
	{
		_globals->getPercentIndicator()->update(read);
// 		PA_OutputText(1,5,12,"Read %d       ",read);
// 		PA_Sleep(10);
// 		PA_OutputText(1,5,12,"              ",read);
		if ( articlePos-read<0 )
		{
// 			PA_OutputText(1,5,13,"Beginne");
// 			PA_Sleep(120);
// 			PA_OutputText(1,5,13,"       ");
			char* start = buffer + articlePos;
			int len = (read-articlePos);
			if ( len>articleLength )
				len = articleLength;

			markup.append(start, len);
			articleLength -= len;

			while ( articleLength && (read = BZ2_bzRead(&bzerror, bzf, buffer, BUFFER_SIZE)) )
			{
				if ( articleLength>read )
					len = read;
				else
					len = articleLength;

				markup.append(buffer, len);
				articleLength -= len;
			}
			break;
		}
		else
		{
			articlePos -= read;
		}
	}

	BZ2_bzReadClose ( &bzerror, bzf );
// 	PA_OutputText(1,5,14,"Nach dem Loop");
// 	PA_Sleep(60);
// 	PA_OutputText(1,5,14,"             ");

	return markup;
}

string WikiMarkupGetter::GetLastArticleTitle()
{
	return _lastArticleTitle;
}

void WikiMarkupGetter::setGlobals(Globals* globals)
{
	_globals = globals;
}
