#include "WikiMarkupGetter.h"

#include <sys/dir.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <bzlib.h>
#include <string>

#include "main.h"
#include "TitleIndex.h"
#include "Globals.h"
#include "PercentIndicator.h"
#include "Dumps.h"

// #define BUFFER_SIZE 8192
#define BUFFER_SIZE 1024
#define BZ_DECOMPRESS_SMALL 1


WikiMarkupGetter::WikiMarkupGetter()
{
}

void WikiMarkupGetter::load(string basename, bool internal)
{
	if (debug)
	{
		PA_ClearTextBg(1);
		PA_OutputText(1,0,2,"Going to load\n%c3%s%c0 from %s",basename.c_str(),(internal)?("efs"):("fat"));
	}

	vector<string> wikidbs = _globals->getDumps()->get_dbs(basename,internal);
	for (int i=0;i< (int) wikidbs.size();i++)
	{
		FILE* _f = fopen(wikidbs[i].c_str(), "rb");
		if (_f)
		{
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
			if (debug)
			{
				PA_OutputText(1,0,5+3*i,"%c3%s\n%c0Loading %c2OK\n%c0size: %c3%d",wikidbs[i].c_str(),size);
			}
		}
		else
		{
		}
	}

	if (debug)
	{
		PA_WaitFor(Pad.Newpress.Anykey);
		PA_ClearTextBg(1);
	}
}

WikiMarkupGetter::~WikiMarkupGetter()
{
	for (int i=0;i< (int) _filepointers.size();i++)
	{
		fclose(_filepointers[i]);
	}
	_filepointers.clear();
	_filesizes.clear();
	_file_absoluteEnds.clear();
	_lastArticleTitle.clear();
}

void WikiMarkupGetter::getMarkup(string & markup, string title)
{
	markup.clear();

	ArticleSearchResult* articleSearchResult = _globals->getTitleIndex()->findArticle(title, "", 1); // The only situation when we need the physical position of the article afterwards
	if ( !articleSearchResult )
	{
		return;
	}

	u64 blockPos		= articleSearchResult->BlockPos();
	int articlePos		= articleSearchResult->ArticlePos();
	int articleLength	= articleSearchResult->ArticleLength();
	_lastArticleTitle	= articleSearchResult->TitleInArchive();

	const int bytesToRead = articlePos + articleLength;
	int bytesRead = 0;
	int percent = 0;

	int fileNo = 0;
	while (_file_absoluteEnds[fileNo]<blockPos) fileNo++;
	for (int i=0;i<fileNo;i++)
		blockPos -= _filesizes[i];
	FILE* _f_data = _filepointers[fileNo];

	if ( !_f_data ) {
		return;
	}
	// seek to the block
	fseek(_f_data, blockPos, SEEK_SET);

	// open the block
	int bzerror;
	BZFILE *bzf = BZ2_bzReadOpen(&bzerror, _f_data, 0, BZ_DECOMPRESS_SMALL, NULL, 0);
	if (bzerror != BZ_OK) {
		BZ2_bzReadClose ( &bzerror, bzf );
		return;
	}

	char buffer[BUFFER_SIZE];
	int read = 0;

	while ( (read = BZ2_bzRead(&bzerror, bzf, buffer, BUFFER_SIZE) ) )
	{
		bytesRead += read;
		percent = bytesRead*100/bytesToRead;
		if (percent>100) percent = 100;
		_globals->getPercentIndicator()->update(percent);

		if ( articlePos-read<0 )
		{
			char* start = buffer + articlePos;
			int len = (read-articlePos);
			if ( len>articleLength )
				len = articleLength;

			markup.append(start, len);
			articleLength -= len;

			while ( articleLength && (read = BZ2_bzRead(&bzerror, bzf, buffer, BUFFER_SIZE)) )
			{
				bytesRead += read;
				percent = bytesRead*100/bytesToRead;
				if (percent>100) percent = 100;
				_globals->getPercentIndicator()->update(percent);

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
	delete articleSearchResult;
	BZ2_bzReadClose ( &bzerror, bzf );
}

string WikiMarkupGetter::GetLastArticleTitle()
{
	return _lastArticleTitle;
}

void WikiMarkupGetter::setGlobals(Globals* globals)
{
	_globals = globals;
}
