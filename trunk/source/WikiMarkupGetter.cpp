#include <PA9.h>
#include <fat.h>
#include <stdio.h>
#include <stdlib.h>
#include <bzlib.h>
#include <string>

#include "WikiMarkupGetter.h"
#include "TitleIndex.h"
#include "main.h"

#define BUFFER_SIZE 8192
#define BZ_DECOMPRESS_SMALL 1

WikiMarkupGetter::WikiMarkupGetter(string basename)
{
	struct stat st;
	char filename[256]; // to hold a full filename and string terminator
	DIR_ITER* dir = diropen("/dswiki/");

	if (dir == NULL)
	{
		PA_OutputText(1,2,2,"Unable to open the directory.");
	}
	else
	{
		while (dirnext(dir, filename, &st) == 0) // collect all *.db? files
		{
			if (!(st.st_mode & S_IFDIR)) // regular file
			{
				string filenamestr = filename;
				if (filenamestr.substr(0,filenamestr.length()-1)==basename+".db")
				{
					if (filenamestr.substr(filenamestr.length()-1,1).find_first_of("abcdefghijklmnopqrstuvwxyz")!=string::npos)
					{
						_filenames.push_back("/dswiki/" + filenamestr);
					}
				}
			}
		}
	}

	sort(_filenames.begin(),_filenames.end()); // alphabetical order is assumed

	int i;
	for (i=0;i<_filenames.size();i++)
	{
		_f_data = fopen(_filenames[i].c_str(), "rb");
		_filepointers.push_back(_f_data);
		fseek(_f_data,0,SEEK_END);
		fpos_t size = ftell(_f_data);
		_filesizes.push_back(size);
		rewind(_f_data);
		u64 absoluteEnd;
		if (i==0)
			absoluteEnd = size-1;
		else
			absoluteEnd = _file_absoluteEnds[i-1] + size;
		_file_absoluteEnds.push_back(absoluteEnd);
	}
}

void WikiMarkupGetter::setNew(string basename)
{
	if (_f_data)
	{
		fclose(_f_data);
	}
	_filenames.clear();
	_filepointers.clear();
	_filesizes.clear();
	_file_absoluteEnds.clear();
	struct stat st;
	char filename[256]; // to hold a full filename and string terminator
	DIR_ITER* dir = diropen("/dswiki/");

	if (dir == NULL)
	{
		PA_OutputText(1,2,2,"Unable to open the directory.");
	}
	else
	{
		while (dirnext(dir, filename, &st) == 0) // collect all *.db? files
		{
			if (!(st.st_mode & S_IFDIR)) // regular file
			{
				string filenamestr = filename;
				if (filenamestr.substr(0,filenamestr.length()-1)==basename+".db")
				{
					if (filenamestr.substr(filenamestr.length()-1,1).find_first_of("abcdefghijklmnopqrstuvwxyz")!=string::npos)
					{
						_filenames.push_back("/dswiki/" + filenamestr);
					}
				}
			}
		}
	}

	sort(_filenames.begin(),_filenames.end()); // alphabetical order is assumed

	int i;
	for (i=0;i<_filenames.size();i++)
	{
		_f_data = fopen(_filenames[i].c_str(), "rb");
		_filepointers.push_back(_f_data);
		fseek(_f_data,0,SEEK_END);
		fpos_t size = ftell(_f_data);
		_filesizes.push_back(size);
		rewind(_f_data);
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
	if (_f_data)
	{
		fclose(_f_data);
	}
	_filenames.clear();
	_filepointers.clear();
	_filesizes.clear();
	_file_absoluteEnds.clear();
	_lastArticleTitle.clear();
}

string WikiMarkupGetter::getMarkup(TitleIndex* t, string title)
{
	ArticleSearchResult* articleSearchResult = t->findArticle(title, 1);
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


	// TODO: Determine correct part
	// The text is completely in that file!

	FILE* _f_data = NULL;
	int fileNo = 0;
	int i;
	while (_file_absoluteEnds[fileNo]<blockPos) fileNo++;
	for (i=0;i<fileNo;i++)
		blockPos -= _filesizes[i];
	_f_data = _filepointers[fileNo];

	if ( !_f_data ) {
// 		PA_OutputText(1,5,10,"!fdata");
		return NULL;
	}
	// seek to the block
	fseek(_f_data, blockPos, SEEK_SET);

	// open the block
	int bzerror;
	BZFILE *bzf = BZ2_bzReadOpen(&bzerror, _f_data, 0, BZ_DECOMPRESS_SMALL, NULL, 0);
	if (bzerror != BZ_OK) {
// 		PA_OutputText(1,5,10,"BZ_ReadOpen failed");
		BZ2_bzReadClose ( &bzerror, bzf );
		return NULL;
	}

// 	PA_OutputText(1,5,11,"Bis kurz vorm Loop");
// 	PA_Sleep(60);
// 	PA_OutputText(1,5,11,"                  ");
	char buffer[BUFFER_SIZE];
	string markup = "";
	int read = 0;

	while ( read = BZ2_bzRead(&bzerror, bzf, buffer, BUFFER_SIZE) )
	{
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
