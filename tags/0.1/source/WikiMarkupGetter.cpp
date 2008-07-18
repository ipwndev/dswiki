#include <PA9.h>
#include <fat.h>
#include <stdio.h>
#include <stdlib.h>
#include <bzlib.h>

#include "main.h"
#include "WikiMarkupGetter.h"
#include "TitleIndex.h"

#define BUFFER_SIZE 8192
#define BZ_DECOMPRESS_SMALL 1

WikiMarkupGetter::WikiMarkupGetter(char* language_code)
{
	_languageCode = language_code;
}

WikiMarkupGetter::~WikiMarkupGetter()
{
}

char* WikiMarkupGetter::GetMarkupForArticle(ArticleSearchResult* articleSearchResult)
{
	if ( !articleSearchResult )
		return NULL;

	fpos_t blockPos   = articleSearchResult->BlockPos();
	int articlePos    = articleSearchResult->ArticlePos();
	int articleLength = articleSearchResult->ArticleLength();
	strcpy(_lastArticleTitle,articleSearchResult->TitleInArchive());

	// open data file
// 	PA_OutputText(1,0,5,"fopen...");
	FILE* f = fopen("dewiki.dat", "rb");
	if ( !f ) {
		return NULL;
	}
// 	PA_OutputText(1,8,5,"%c2OK");

	// seek to the block
	fseek(f, blockPos, SEEK_SET);

	// open the block
	int bzerror;
// 	PA_OutputText(1,0,6,"BZopen...");
	BZFILE *bzf = BZ2_bzReadOpen(&bzerror, f, 0, BZ_DECOMPRESS_SMALL, NULL, 0);
// 	PA_OutputText(1,9,6,"%c2OK");
	if (bzerror != BZ_OK) {
		BZ2_bzReadClose ( &bzerror, bzf );
		return NULL;
	}

	char buffer[BUFFER_SIZE];
	char* markup = (char*) malloc(articleLength+1);
	int read;

	while ( read = BZ2_bzRead(&bzerror, bzf, buffer, BUFFER_SIZE) )
	{
		if ( articlePos-read<0 )
		{
			char* start = buffer + articlePos;
			int len = (read-articlePos);
			if ( len>articleLength )
				len = articleLength;

			char* pText = markup;
			strncpy(pText, start, len);
			articleLength -= len;
			pText += len;

			while ( articleLength && (read = BZ2_bzRead(&bzerror, bzf, buffer, BUFFER_SIZE)) )
			{
				if ( articleLength>read )
					len = read;
				else
					len = articleLength;

				strncpy(pText, buffer, len);
				articleLength -= len;
				pText += len;
			}
			*pText = '\0';
			break;
		}
		else
		{
			articlePos -= read;
		}
	}

	BZ2_bzReadClose ( &bzerror, bzf );
	fclose(f);

	return markup;
}

char* WikiMarkupGetter::GetLastArticleTitle()
{
	return _lastArticleTitle;
}
