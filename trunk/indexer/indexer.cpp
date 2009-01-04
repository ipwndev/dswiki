/*
*  indexer.cpp
*  Wiki2Touch/wikisrvd
*
*  Copyright (c) 2008 by Tom Haukap.
*
*  This file is part of Wiki2Touch.
*
*  Wiki2Touch is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  Wiki2Touch is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with Wiki2Touch. If not, see <http://www.gnu.org/licenses/>.
*/

const char version[] = "0.65b";
const char dsversion[] = "1.0.4";
#define INDEX_VERSION 2

#ifdef _WIN32
// windows defines
#define WIN32_LEAN_AND_MEAN 	// Exclude rarely-used stuff from Windows headers
#define fseeko fseek
#define ftello ftell
#pragma warning(disable : 4996)
#endif

#define _FILE_OFFSET_BITS 64   // needed on Linux

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include <errno.h>
#include <bzlib.h>
#include <string>
#include <map>
#include <vector>
#include "char_convert.h"
#include "indexer.h"

#define BUFFER_SIZE 65536
#define CONTENT_SIZE 65536
#define BZIP_BLOCK_SIZE_100K 1
#define SIZEOF_POSITION_INFORMATION 16

#define MAX_IGNORED_NAMESPACES 999

#define SPLIT_SIZE 2000000000

FILE*   srcFile = NULL;

unsigned char numberOfBZippedFile = 0;
int numberOfIgnoredNamespaces = 0;
char** ignoredNamespaces = NULL;
int* articlesIgnoredByNamespaces = NULL;

char    articlesifo[13] = "articles.ifo";
FILE*   file_ifo        = NULL;

char    articlesdbz[13] = "articles.db_";
FILE*   file_dbz        = NULL;
BZFILE* destBZFile      = NULL;

char    articlesidx[13] = "articles.idx";
FILE*   file_idx        = NULL;
size_t  articlesTitlesSize;

char    articlesao1[13] = "articles.ao1";
FILE*   file_ao1        = NULL;

char    articlesao2[13] = "articles.ao2";
FILE*   file_ao2        = NULL;

char  imageTitlesFileName[20] = "images.txt";
FILE*	imageTitlesFile = NULL;

char* imagesPrefix = NULL;

size_t currentDestSize = 0;
offset_t currentBlockPos = 0;
offset_t previousBlockPos = 0;

char* articlesTitles;
int* articlesIndex;

vector<string> aTitles;
vector<string> aTitlesOriginal;


bool internalIndexingStats = false;
IndexStatus *indexStatus = NULL;

// global options with their defaults
bool extractImagesOnly = false;					// only get the list of images used by the articles
bool dontExtractImages = true;					// only create the 'article.bin' database but not the lis of images
bool addIndexWithDiacriticsRemoved = true;		// add a second index which is diacrity/simplied chinese aware
bool longOutfileName = false;		            // add language code to the output filename
bool removedUnusedArticles = true;				// skip some namespaces, i.e. Image, Wikipedia or Help
bool verbose = true;							// except for the copyright and done message be silent
bool standaloneApp = true;						// don't write anything to the console window
bool includeTitleInImagelist = false;			// only for debugging: include a line for each scanned articel in images.txt
bool createSecondIndexTextfile = false;			// only for debugging: print the second index to articles.txt


// Adds an integer value as an utf8 encoded char to the destion string
inline void AddUtf8Coding(unsigned int number, char* dst)
{
	if ( !dst )
		return;

	if ( number<0x00080 )
		*dst += number;
	else if ( number<0x00800 )
	{
		*dst++ = (0xc0 | (number>>6));
		*dst++ = (0x80 | (number & 0x3f));
	}
	else if ( number<0x010000 )
	{
		*dst++ = (0xe0 | (number>>12));
		*dst++ = (0x80 | (number>>6 & 0x3f));
		*dst++ = (0x80 | (number & 0x3f));
	}
	else
	{
		*dst++ = (0xf0 | (number>>18));
		*dst++ = (0x80 | (number>>12 & 0x3f));
		*dst++ = (0x80 | (number>>6 & 0x3f));
		*dst++ = (0x80 | (number & 0x3f));
	}
}

// Removes xml style chars chars like "&gt"; "&amp;" or "#nnn;" from the given src string
static void XmlDecode(char* src)
{
	char* dst = src;
	while ( *src )
	{
		char c = *src++;
		if ( c=='&' )
		{
			char* tag = src;
			while ( *src && *src!=';' )
				src++;

			if ( *src )
			{
				*src++ = 0x0;

				if ( !strcmp(tag, "lt") )
					*dst++ = '<';
				else if ( !strcmp(tag, "gt") )
					*dst++ = '>';
				else if ( !strcmp(tag, "amp") )
					*dst++ = '&';
				else if ( !strcmp(tag, "quot") )
					*dst++ = '"';
				else if ( *tag=='#' && strlen(tag)>1 )
				{
					tag++;
					unsigned int number = atoi(tag);
					AddUtf8Coding(number, dst);
				}
				else
				{
					*dst++ = '&';
					strcat(dst, tag);
					*dst++ = ';';
				}
			}
		}
		else
			*dst++ = c;
	}

	*dst = 0;
}

static void Quicksort(int left, int right)
{
// 	printf("Quicksort: %d-%d\n", left, right);
	if ( left<right )
	{
		int i = left;
		int j = right - 1;
		int m = (left+right)/2;

		string h = aTitles[m];
		aTitles[m] = aTitles[right];
		aTitles[right] = h;
		int help = articlesIndex[m];
		articlesIndex[m] = articlesIndex[right];
		articlesIndex[right] = help;

		string pivotTitle = aTitles[right];
		do
		{
			while ( i<right )
			{
				string title = aTitles[i];
				if ( title <= pivotTitle )
					i++;
				else
					break;
			}

			while ( j>left )
			{
				string title = aTitles[j];
				if ( pivotTitle <= title )
					j--;
				else
					break;
			}

			if ( i<j )
			{
				string h = aTitles[i];
				aTitles[i] = aTitles[j];
				aTitles[j] = h;
				int help = articlesIndex[i];
				articlesIndex[i] = articlesIndex[j];
				articlesIndex[j] = help;
			}
		}
		while (i < j);
		if ( i!=right )
		{
			string h = aTitles[i];
			aTitles[i] = aTitles[right];
			aTitles[right] = h;
			int help = articlesIndex[i];
			articlesIndex[i] = articlesIndex[right];
			articlesIndex[right] = help;
		}

		Quicksort(left, i-1);
		Quicksort(i+1, right);
	}
}


static void WriteArticle(char* title, char* text)
{
	if ( !title || !text )
		return;

	// fix the title, remove xml codings
	XmlDecode(title);

	// do we need this article?
	if ( numberOfIgnoredNamespaces )
	{
		char* colon = strstr(title, ":");
		if ( colon )
		{
			*colon = 0x0;

			int i = 0;
			while ( i<numberOfIgnoredNamespaces )
			{
				if ( !strcmp(ignoredNamespaces[i], title) )
				{
					indexStatus->bytesSkipped += strlen(text);
					indexStatus->articlesSkipped++;
					articlesIgnoredByNamespaces[i]++;
					return;
				}

				i++;
			}

			*colon = ':';
		}
	}

	// fix the text, remove xml codings
	XmlDecode(text);

	size_t length = strlen(text);

	int bzerror = BZ_OK;
	if ( currentDestSize+length>BZIP_BLOCK_SIZE_100K*100000 )
	{
		if ( destBZFile )
		{
			BZ2_bzWriteClose(&bzerror, destBZFile, 0, NULL, NULL);
			destBZFile = NULL;
		}
		if (ftello(file_dbz) + length > SPLIT_SIZE)
		{
			previousBlockPos += ftello(file_dbz);
			fclose(file_dbz);
			numberOfBZippedFile++;
			articlesdbz[11] = numberOfBZippedFile + 'a';
			file_dbz = fopen(articlesdbz, "wb");
		}
	}

	if ( !destBZFile )
	{
		currentBlockPos=previousBlockPos+ftello(file_dbz)+256;
		currentDestSize = 0;
		indexStatus->blockCount++; // just for information

		destBZFile = BZ2_bzWriteOpen(&bzerror, file_dbz, BZIP_BLOCK_SIZE_100K, 0, 30);
	}

	// an entry is build like this:
	// 8 bytes block position in the file
	// 4 bytes position inside the block
	// 4 bytes length of the article
	// title in plain utf-8 coding
	// terminating zero

	fwrite(&currentBlockPos, 1, sizeof currentBlockPos, file_idx);
	fwrite(&currentDestSize, 1, sizeof currentDestSize, file_idx);
	fwrite(&length, 1, sizeof length, file_idx);
	fwrite(title, 1, strlen(title)+1, file_idx);

	// put the text to the articles data file
	// the text /may/ likely span multiple blocks, but /never/ multiple files
	BZ2_bzWrite(&bzerror, destBZFile, text, (int) length);
	currentDestSize += length;

	indexStatus->articlesWritten++;
	indexStatus->bytesTotal += length;
}

void Help()
{
	if ( !standaloneApp )
		return;

	printf("Usage:\r\n  indexer [-a] <filename> \r\n\r\n");
	printf("   filename: articles file from wikipedia like 'enwiki-latest-pages-articles.xml.bz2'\r\n");
	printf("    -a : Add all articles (not removing namespaces \n\t like 'Help', 'Category' etc.)\r\n");
	printf("    -t : Create the file articles.txt containing\n\t all article titles and lengths (not needed by DSwiki).\r\n");
	printf("\r\n");
}

void checkStructSizes()
{
    // we get paranoid here: check that all struct members have the right size
    FILEHEADER fileHeader;
    if (sizeof(fileHeader.numberOfArticles) != 4) {
        printf("ERROR: compiled application has int not 32 bit wide\n");
        exit(1);
    }
    if (sizeof(fileHeader.titlesPos) != 8) {
        printf("ERROR: compiled application has off_t not 64 bit wide\n");
        exit(1);
    }
    if (sizeof(fileHeader) != (32 + 2*32 + 160)) {
        printf("ERROR: compiled application has wrong struct alignment\n");
        exit(1);
    }
    fileHeader.titlesPos = 0x0203;
    unsigned char * p = (unsigned char *)&fileHeader.titlesPos;
    if (p[0] != 3 || p[1] != 2){
        printf("ERROR: compiled application has wrong endianess (1)\n");
        exit(1);
    }
    if (p[6] != 0 || p[7] != 0){
        printf("ERROR: compiled application has wrong endianess (2)\n");
        exit(1);
    }
}

void FreeIngoredNamespacesData()
{
	if ( ignoredNamespaces )
	{
		int i = 0;
		while ( i<numberOfIgnoredNamespaces )
		{
			if ( ignoredNamespaces[i] )
				free(ignoredNamespaces[i]);
			i++;
		}

		free(ignoredNamespaces);
		ignoredNamespaces = 0;
	}
	numberOfIgnoredNamespaces = 0;

	if ( articlesIgnoredByNamespaces )
	{
		free(articlesIgnoredByNamespaces);
		articlesIgnoredByNamespaces = NULL;
	}
}

void CleanupAfterError()
{
	if ( file_dbz )
	{
		fclose(file_dbz);
		remove(articlesdbz);
		file_dbz = NULL;
	}
	if ( file_ifo )
	{
		fclose(file_ifo);
		remove(articlesifo);
		file_ifo = NULL;
	}
	if ( file_idx )
	{
		fclose(file_idx);
		remove(articlesidx);
		file_idx = NULL;
	}
	if ( file_ao1 )
	{
		fclose(file_ao1);
		remove(articlesao1);
		file_ao1 = NULL;
	}
	if ( file_ao2 )
	{
		fclose(file_ao2);
		remove(articlesao2);
		file_ao2 = NULL;
	}

	if ( srcFile )
	{
		fclose(srcFile);
		srcFile = NULL;
	}

	if ( internalIndexingStats )
	{
		free(indexStatus);
		indexStatus = NULL;
		internalIndexingStats = false;
	}

	if ( articlesIndex )
	{
		free(articlesIndex);
		articlesIndex = NULL;
	}

	if ( articlesTitles )
	{
		free(articlesTitles);
		articlesTitles = NULL;
	}

	if ( imagesPrefix )
	{
		free(imagesPrefix);
		imagesPrefix = NULL;
	}

	FreeIngoredNamespacesData();
}

// The repacker/indexer
static int index(int argc, char* argv[])
{
	if ( standaloneApp )
	{
		printf("DSwiki repackager/indexer by OlliPolli, Version %s, completely based on\r\n", dsversion);
		printf("Wiki2Touch repackager/indexer, Copyright (c) 2008 T. Haukap, Version %s\r\n\r\n", version);
	}

	char* sourceFileName = NULL;
	char* sourceFileBasename = NULL;

	articlesTitles = NULL;
    checkStructSizes();

	// check for options
	for (int i=1; i<argc; i++)
	{
		if ( argv[i][0]=='-' )
		{
			for (size_t j=1; j<strlen(argv[i]); j++)
			{
				switch( argv[i][j] )
				{
					case 'a':
						removedUnusedArticles = false;
						break;
					case 't':
						createSecondIndexTextfile = true;
						break;
					default:
						Help();
						break;
				}
			}
		}
		else
		{
			sourceFileName = argv[i];

			i++;

			if ( standaloneApp )
			{
				while ( i<argc )
					printf("warning: additional argument '%s' ignored.\r\n", argv[i++]);
			}
			else
				i = argc;
		}
	}

	for (int i=1; i<argc; i++)
	{
		if ( argv[i][0]=='-' )
		{
		}
		else
		{
			sourceFileName = argv[i];

			i++;

			if ( standaloneApp )
			{
				while ( i<argc )
					printf("warning: additional argument '%s' ignored.\r\n", argv[i++]);
			}
			else
				i = argc;
		}
	}


	if ( !sourceFileName )
	{
		Help();
		return 1;
	}

	// init some globals (only necessary if the GUI indexer is used but who cares)
	srcFile = NULL;
	imageTitlesFile = NULL;
	destBZFile = NULL;

	numberOfIgnoredNamespaces = 0;
	ignoredNamespaces = NULL;
	articlesIgnoredByNamespaces = NULL;

	currentDestSize = 0;
	imagesPrefix = NULL;

	imagesPrefix = NULL;

	currentDestSize = 0;
	currentBlockPos = 0;
	previousBlockPos = 0;

	articlesTitlesSize = 0;;
	articlesTitles = NULL;

	articlesIndex = 0;

	// reserve space for 32 entries from the start
	ignoredNamespaces = (char**) malloc(sizeof(char*)*MAX_IGNORED_NAMESPACES);
	memset(ignoredNamespaces, 0x00, sizeof(char*)*MAX_IGNORED_NAMESPACES);

	articlesIgnoredByNamespaces = (int*) malloc(sizeof(int)*MAX_IGNORED_NAMESPACES);
	memset(articlesIgnoredByNamespaces, 0x00, sizeof(int)*MAX_IGNORED_NAMESPACES);

	internalIndexingStats = !indexStatus;
	if ( internalIndexingStats )
		indexStatus = (IndexStatus*) malloc(sizeof(IndexStatus));
	memset(indexStatus, 0x0, sizeof(IndexStatus));

	if ( verbose )
		printf("working on %s\r\n", sourceFileName);

	srcFile = fopen((char*) sourceFileName, "rb");
	if ( !srcFile )
	{
		CleanupAfterError();
		if ( standaloneApp )
			printf("error opening source file\r\n");
		return 1;
	}

	// try to ge the size
	offset_t sourceSize;
	fseeko(srcFile, 0, SEEK_END);
	sourceSize=ftello(srcFile);
	fseeko(srcFile, 0, SEEK_SET);

	int bzerror;
	BZFILE *bzf = BZ2_bzReadOpen(&bzerror, srcFile, 0, 0, NULL, 0);

	if ( !extractImagesOnly )
	{
		// open the dest file for the data
		articlesdbz[11] = numberOfBZippedFile + 'a';
		file_ifo = fopen(articlesifo, "wb");
		file_dbz = fopen(articlesdbz, "wb");
		file_idx = fopen(articlesidx, "wb");
		file_ao1 = fopen(articlesao1, "wb");
		file_ao2 = fopen(articlesao2, "wb");
		if ( (!file_ifo)||(!file_dbz)||(!file_idx)||(!file_ao1)||(!file_ao2) )
		{
			CleanupAfterError();
			return 1;
		}
	}
	else if (verbose)
		printf("extracting images only.\r\n");

	FILEHEADER fileHeader;
	memset(&fileHeader, 0, sizeof fileHeader);
	sourceFileBasename = sourceFileName;
	while (*sourceFileBasename) sourceFileBasename++;
	while ((*sourceFileBasename != '/') && (sourceFileBasename > sourceFileName)) sourceFileBasename--;
	if (*sourceFileBasename == '/') sourceFileBasename++;

	fileHeader.languageCode[0] = tolower(sourceFileBasename[0]);
	fileHeader.languageCode[1] = tolower(sourceFileBasename[1]);
	fileHeader.version = INDEX_VERSION;

	bool chinesesCharacters = (fileHeader.languageCode[0]=='z' && fileHeader.languageCode[1]=='h');

	imageTitlesFile = 0;
	if ( !dontExtractImages )
	{
		imageTitlesFile = fopen(imageTitlesFileName, "wb");
		if ( !imageTitlesFile )
		{
			CleanupAfterError();
			return 1;
		}
	}

	char buffer[BUFFER_SIZE];
	size_t read;

	int state = 0;
	bool endTag = false;

	char tagName[512];
	char* pTagName = NULL;
	int tagNameSize = 0;

	char tagAttributes[512];
	char* pTagAttributes = NULL;
	int tagAttributesSize = 0;

	char* content = NULL;  // pointer to first position
	char* pContent = NULL; // pointer to current position
	int contentSize = 0;
	int contentRemain = 0;
	bool collectContent = true;

	int namespaceKey = 0;

	offset_t totalBytes = 0;

	char* articleTitle = NULL;
	while ( (read=BZ2_bzRead(&bzerror, bzf, buffer, BUFFER_SIZE)) )
	{
		char *buf = buffer;
		totalBytes += read;

		while ( read-- )
		{
			char c = *buf++;
// 			if (state==1) printf("%c\n",c);
// 			else printf("         %c\n",c);
			switch (state)
			{
				case 0:
					// inside text
					if ( c=='<' )
					{
						if ( content )
							*pContent = 0x0;

						pTagAttributes = NULL;
						pTagName = NULL;
						endTag = false;

						state = 1;
// 						printf(" state 0->1\n");
					}
					else if ( content && collectContent )
					{
						// collect the content of the tag
						if ( !contentRemain )
						{
							// realloc
							contentSize += CONTENT_SIZE;
							content = (char*) realloc(content, contentSize + 1);
							contentRemain = CONTENT_SIZE;

							pContent = content + (contentSize - CONTENT_SIZE);
						}

						*pContent++ = c;
						contentRemain--;
// 						printf("             +\n");
					}
					break;

				case 1:
					// inside tag
					if ( c=='>' )
					{
						state = 0;
// 						printf(" state 1->0\n");

						// done reading the tag
						if ( pTagName )
							*pTagName = 0x0;
						else
							tagName[0] = 0x0;

						if ( pTagAttributes )
							*pTagAttributes++ = 0x0;
						else
							tagAttributes[0] = 0x0;

						if ( !endTag )
						{
							collectContent = true;
							if ( !strcmp(tagName, "title") || !strcmp(tagName, "text") || !strcmp(tagName, "namespace") )
							{
								if ( content )
									free(content);

								// start collecting the content
								contentSize = CONTENT_SIZE;
								content = (char*) malloc(contentSize + 1);
								contentRemain = CONTENT_SIZE;

								pContent = content;

								if ( !strcmp(tagName, "namespace") )
								{
									namespaceKey = 0;

									// save the key
									char* key = strstr(tagAttributes, "key=\"");
									if ( key )
									{
										key += 5;

										char buffer[512];
										char* pBuffer = buffer;
										while ( *key && *key!='"' )
											*pBuffer++ = *key++;
										*pBuffer = 0;

										namespaceKey = atoi(buffer);
									}
								}
							}
						}
						else if ( endTag )
						{
							if ( !strcmp(tagName, "page") )
							{
// 								printf("page end\n");
								if ( articleTitle )
								{
// 									printf("%s (%d):\n->%s<-\n",articleTitle,strlen(content),content);
// 									printf("Going to WriteArticle()\n");
									WriteArticle(articleTitle, content);
// 									printf("Finished to WriteArticle()\n");

									free(articleTitle);
									articleTitle = NULL;
								}
								if ( content )
									free(content);

								content = NULL;
								contentRemain = 0;
								contentSize = 0;
							}
							else if ( !strcmp(tagName, "title") )
							{
								if ( articleTitle )
									free(articleTitle);

								// store the title for later use
								articleTitle = content;

								// this prevents that the memory is freed
								content = NULL;

								contentRemain = 0;
								contentSize = 0;
							}
							else if ( !strcmp(tagName, "text") )
							{
								collectContent = false;
								if ( (indexStatus->articlesWritten&0x0ff)==0 )
								{
									offset_t currentPos;
									currentPos=ftello(srcFile);
									indexStatus->progress = (int)(10000*currentPos/sourceSize);
									if ( verbose && (indexStatus->articlesWritten&0x03ff)==0 )
									{
										printf("\rProcessed: %.0f%% (%i articles) ", (100.0*currentPos/sourceSize), indexStatus->articlesWritten);
										fflush(stdout);
									}
								}
							}
							else if ( !strcmp(tagName, "namespace") )
							{
								if ( namespaceKey && content && strlen(content) )
								{
									if ( strlen(content)<32 )
									{
										if ( namespaceKey==6 && strlen(content)<32 )
										{
											strcpy(fileHeader.imageNamespace, content);

											// only do that if it's not Image, these will always be searched
											if ( strcmp(content, "Image") ) // TODO: Localized?
											{
												// get it into the proper form
												imagesPrefix = (char*) malloc(2 + strlen(content) + 1 + 1);
												strcpy(imagesPrefix, "[[");
												strcat(imagesPrefix, content);
												strcat(imagesPrefix, ":");
											}
										}
										else if ( namespaceKey==10 && strlen(content)<32 )
											strcpy(fileHeader.templateNamespace, content);
										if ( removedUnusedArticles)
										{
											// add some namespaces to the list of ignored ones
											switch ( namespaceKey )
											{
											case -2: // Media
											case -1: // Wiki
											case  1: // Talk
											case  2: // User
											case  3: // User talk
											case  4: // Wikipedia
											case  5: // Wikipedia talk
											case  6: // Image
											case  7: // Image talk
											case  8: // Mediawiki
											case  9: // Mediawiki talk
											case 11: // Template talk
											case 12: // Help
											case 13: // Help talk
											case 14: // Category
											case 15: // Category talk
												if ( numberOfIgnoredNamespaces<MAX_IGNORED_NAMESPACES)
												{
													ignoredNamespaces[numberOfIgnoredNamespaces] = (char*) malloc(strlen(content)+1);
													strcpy(ignoredNamespaces[numberOfIgnoredNamespaces], content);
													if ( verbose )
														printf("Articles in namespace '%s' are ignored\r\n", ignoredNamespaces[numberOfIgnoredNamespaces]);
													numberOfIgnoredNamespaces++;
												}
												break;
											}
										}
									}
								}
								if ( content )
									free(content);

								content = NULL;
								contentRemain = 0;
								contentSize = 0;
							}
						}
					}
					else if ( !pTagName )
					{
						// first char after the "<"

						pTagName = tagName;
						tagNameSize = 0;

						if (c=='/')
							endTag = true;
						else if (c!=' ')
							*pTagName++ = c;
						else
							pTagAttributes = tagAttributes;
					}
					else if ( !pTagAttributes )
					{
						if ( c==' ' )
							pTagAttributes = tagAttributes;
						else if ( tagNameSize<512 )
						{
							*pTagName++ = c;
							tagNameSize++;
						}
					}
					else
					{
						if ( tagAttributesSize<512 )
						{
							*pTagAttributes++ = c;
							tagAttributesSize++;
						}
					}
					break;
			}
		}
	}

	fclose(file_idx); file_idx = NULL;

	if ( content )
	{
		free(content);
		content = NULL;
	}

	if ( verbose )
#ifdef WIN32
		printf("\n\rRepackaging done:\r\n%I64d bytes before, %i articles written (%I64d bytes)\r\n%i blocks\r\n%i articles skipped (%I64d bytes)\r\n",
#else
		printf("\n\rRepackaging done:\r\n%lld bytes before, %i articles written (%lld bytes)\r\n%i blocks\r\n%i articles skipped (%lld bytes)\r\n",
#endif
		totalBytes,
		indexStatus->articlesWritten,
		indexStatus->bytesTotal,
		indexStatus->blockCount,
		indexStatus->articlesSkipped,
		indexStatus->bytesSkipped
		);

	if ( verbose )
	{
		if ( numberOfIgnoredNamespaces )
		{
			int i = 0;
			while ( i<numberOfIgnoredNamespaces )
			{
				if ( articlesIgnoredByNamespaces[i] )
					printf("%i articles skipped in namespace '%s'\r\n", articlesIgnoredByNamespaces[i], ignoredNamespaces[i]);
				i++;
			}
		}

		printf("Images namespace is: %s\r\n", fileHeader.imageNamespace);
		printf("Templates namespace is : %s\r\n", fileHeader.templateNamespace);
	}

	if ( destBZFile )
	{
		BZ2_bzWriteClose(&bzerror, destBZFile, 0, NULL, NULL);
		destBZFile = NULL;
	}

	BZ2_bzReadClose(&bzerror, bzf);
	fclose(srcFile);
	srcFile = NULL;

	if ( imageTitlesFile )
	{
		fclose(imageTitlesFile);
		imageTitlesFile = NULL;
	}

	if ( extractImagesOnly )
	{
		// all done
		if ( standaloneApp )
			printf("extracting images only done.\r\n");
		return 0;
	}

	if ( file_dbz )
	{
		fileHeader.titlesPos = previousBlockPos+ftello(file_dbz)+sizeof(FILEHEADER);
		fileHeader.numberOfArticles = indexStatus->articlesWritten;
	}

	/* Indexing (one or two are created )*/
	int index = 0;
	int indexes = addIndexWithDiacriticsRemoved ? 2 : 1;
	for (index = 0; index<indexes; index++)
	{
		if ( verbose )
		{
			printf("\r\nIndexing: Processing index number %i\r\n", index);
			printf("reading articles titles\r\n");
		}

		struct stat statbuf;
		if (stat(articlesidx, &statbuf) < 0)
		{
			CleanupAfterError();
			if ( standaloneApp )
				printf("file not found.\r\n");

			return 1;
		}

		articlesTitlesSize = statbuf.st_size;
		articlesTitles = (char*) malloc(articlesTitlesSize);
		if ( !articlesTitles )
		{
			CleanupAfterError();
			if ( standaloneApp )
				printf("out of memory.\r\n");

			return 1;
		}

		FILE* f = fopen(articlesidx, "rb");
		if ( !f )
		{
			CleanupAfterError();
			if ( standaloneApp )
				printf("problems opening file of article titles.\r\n");

			return 1;
		}

		read = fread(articlesTitles, 1, articlesTitlesSize, f);
		if ( read!=articlesTitlesSize )
		{
			CleanupAfterError();
			if ( standaloneApp )
				printf("problems reading titles file.\r\n");

			return 1;
		}
		fclose(f);


		if ( verbose )
			printf("lowering and indexing articles titles\r\n");

		articlesIndex = (int*) malloc(indexStatus->articlesWritten * sizeof(int));
		int no = 0;

		aTitles.clear();

		FILE* f_titles = NULL;
		if (createSecondIndexTextfile)
		{
			f_titles = fopen("articles.txt","w");
		}

		char* help = articlesTitles;
		while ( (help-articlesTitles) < (int) read )
		{
			articlesIndex[no++] = (int) (help - articlesTitles);

			int articleLength = *(int*)(help+12);

			// skip the binary position information (8+4+4 bytes)
			help += SIZEOF_POSITION_INFORMATION;

			string titleTest = help;
			int length = titleTest.length();

			if (f_titles)
				fprintf(f_titles,"%d;%s\n",articleLength,titleTest.c_str());

			// remove the diacritics (only for index number 1)
			// attention this may change the length so length calculation has to be done before
			if ( index==1 )
			{
				titleTest = lowerPhrase(exchangeDiacriticChars(lowerPhrase(titleTest,INDEX_VERSION),INDEX_VERSION),INDEX_VERSION);
			}
			else
			{
				titleTest = lowerPhrase(titleTest,INDEX_VERSION);
			}

			aTitles.push_back(titleTest);

			// go to the start of the next title
			help += (length + 1);
		}
		if (f_titles)
		{
			fclose(f_titles);
		}

		if ( verbose )
			printf("sorting index\r\n");

		Quicksort(0, indexStatus->articlesWritten-1);

		if ( verbose )
			printf("checking index: ");

		bool failed = false;
		string title = aTitles[0];

		for (int i=1; i<indexStatus->articlesWritten; i++)
		{
			string next = aTitles[i];
			if ( title > next )
			{
				failed = true;
				break;
			}

			title = next;
		}

		if ( verbose )
			printf("%s\r\n", failed ? "failed" : "passed");

		if ( !failed )
		{
			/* write out our index */
			if ( verbose )
				printf("writing index: %i bytes\r\n", indexStatus->articlesWritten*sizeof(int));

			// finally append the articles index at the end of the file
			if ( index==0 )
			{
				fwrite(articlesIndex, 1, indexStatus->articlesWritten * sizeof(int), file_ao1);
				fileHeader.indexPos_0 = sizeof(FILEHEADER)+previousBlockPos+ftello(file_dbz)+articlesTitlesSize;
			}
			else
			{
				fwrite(articlesIndex, 1, indexStatus->articlesWritten * sizeof(int), file_ao2);
				fileHeader.indexPos_1 = sizeof(FILEHEADER)+previousBlockPos+ftello(file_dbz)+articlesTitlesSize+ftello(file_ao1);
			}
			if ( verbose )
				printf("written\r\n");
		}
		else
		{
			CleanupAfterError();
			if ( standaloneApp )
				printf("index check failed.\r\n");

			return 1;
		}

		free(articlesIndex);
		free(articlesTitles);
	}

	if ( imagesPrefix )
		free(imagesPrefix);

	FreeIngoredNamespacesData();

	if ( internalIndexingStats )
	{
		free(indexStatus);
		indexStatus = NULL;
		internalIndexingStats = false;
	}

	fwrite(&fileHeader, sizeof fileHeader, 1, file_ifo);

	fclose(file_ifo); file_ifo = NULL;
	fclose(file_dbz); file_dbz = NULL;
	fclose(file_ao1); file_ao1 = NULL;
	fclose(file_ao2); file_ao2 = NULL;

	if ( standaloneApp )
		printf("Done.\r\n\r\n");

	return 0;
}



int main(int argc, char* argv[])
{
	char* help = (char*) malloc(strlen(*argv)+1);
	strcpy(help, *argv);

	char* filename = help + strlen(help);
	while ( filename>help && *filename!='/' && *filename!='\\' )
	{
		if ( *filename=='.' )
			*filename = 0;
		filename--;
	}
	if ( *filename=='/' || *filename=='\\' )
		filename++;

	int result = 1;
	if ( strcmp(filename, "indexer")==0 )
		result = index(argc, argv);
	else
		printf("do not rename this file\r\n");

	free(help);

	return result;
}
