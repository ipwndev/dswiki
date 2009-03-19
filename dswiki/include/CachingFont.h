//
// C++ Interface: CachingFont
//
// Description:
//
//
// Author: Oliver Gronau <ogronau@web.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef _CACHINGFONT_H
#define _CACHINGFONT_H

#include <vector>

#define UNICODEBLOCKS_STATIC 6
#define UNICODEBLOCKS_TEMPORARY 6

class UnicodeBlock
{
	public:
		UnicodeBlock(char* filename, unsigned char blockNr);
		~UnicodeBlock();
		unsigned char* getCharacterData(unsigned char Uni);
		unsigned char Number();
	private:
		int*			_index; // 2 bytes would be OK either
		unsigned char*	_data;
		unsigned char	_blockNr;
		int				_size_of_data;
};

class Font
{
	public:
		Font(char* filename);
		~Font();
		unsigned char* getCharacterData(unsigned int Uni);
		unsigned char getCharacterWidth(unsigned int Uni) { return (getCharacterData(Uni)[0]); };
		bool initOK();
		unsigned char Height();
		unsigned char normalWidth();
	private:
		unsigned char _height;
		unsigned char _normalWidth;
		bool _initOK;
		char _filename[32];
		int* blockImpressions;
		int* cached;
		int _lastCached;
		std::vector<UnicodeBlock*> UB;
};

#endif
