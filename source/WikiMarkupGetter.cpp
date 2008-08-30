#include <PA9.h>
#include <fat.h>
#include <stdio.h>
#include <stdlib.h>
#include <bzlib.h>
#include <string>

#include "main.h"
#include "WikiMarkupGetter.h"
#include "TitleIndex.h"

#define BUFFER_SIZE 8192
#define BZ_DECOMPRESS_SMALL 1

WikiMarkupGetter::WikiMarkupGetter(string basename)
{
	_FileName_Data = basename + ".dbz";
	_f_data = fopen(_FileName_Data.c_str(), "rb");
}

WikiMarkupGetter::~WikiMarkupGetter()
{
	if (_f_data)
	{
		fclose(_f_data);
	}
}

string WikiMarkupGetter::getMarkup(TitleIndex* t, string title)
{
	ArticleSearchResult* articleSearchResult = t->FindArticle(title, 1);
	if ( !articleSearchResult )
		return NULL;

	fpos_t blockPos   = articleSearchResult->BlockPos();
	int articlePos    = articleSearchResult->ArticlePos();
	int articleLength = articleSearchResult->ArticleLength();
	_lastArticleTitle = articleSearchResult->TitleInArchive();

	if ( !_f_data ) {
		return NULL;
	}
	// seek to the block
	fseek(_f_data, blockPos, SEEK_SET);

	// open the block
	int bzerror;
	BZFILE *bzf = BZ2_bzReadOpen(&bzerror, _f_data, 0, BZ_DECOMPRESS_SMALL, NULL, 0);
	if (bzerror != BZ_OK) {
		BZ2_bzReadClose ( &bzerror, bzf );
		return NULL;
	}

	char buffer[BUFFER_SIZE];
	string markup;
	int read;

	while ( read = BZ2_bzRead(&bzerror, bzf, buffer, BUFFER_SIZE) )
	{
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

	return markup;
}

string WikiMarkupGetter::GetLastArticleTitle()
{
	return _lastArticleTitle;
}
