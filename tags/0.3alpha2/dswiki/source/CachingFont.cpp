//
// C++ Implementation: CachingFont
//
// Description:
//
//
// Author: Oliver Gronau <ogronau@web.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "CachingFont.h"
#include <PA9.h>
#include <fat.h>
#include "main.h"

Font::Font(char* filename)
{
	_initOK = false;
	strcpy(_filename,filename);
	FILE* f = fopen(_filename,"rb");
	if (f != NULL)
	{
		fseek(f,32,SEEK_SET);
		fread(&_height,1,1,f);
		blockImpressions = (int*) malloc(256*sizeof(int));
		cached = (int*) malloc(256*sizeof(int));
		_initOK = true;
		for (int a=0;a<256;a++)
		{
			blockImpressions[a] = 0;
			cached[a] = -1;
		}
		fclose(f);
	}
	_lastCached = UNICODEBLOCKS_STATIC;
	_normalWidth = getCharacterWidth(0x20);
}

Font::~Font()
{
	free(blockImpressions);
	free(cached);
	UB.clear();
}

unsigned char Font::Height()
{
	return _height;
}

unsigned char Font::normalWidth()
{
	return _normalWidth;
}

unsigned char* Font::getCharacterData(unsigned int Uni)
{
	// we only treat the Basic Multilingual Plane (BMP)
	if (Uni > 0xFFFF)
		Uni = 0xFFFD;

	unsigned int block      = Uni / 256;
	unsigned char character = Uni & 0xFF;

	int position = 0;

	blockImpressions[block]++;
	if (cached[block] != -1)
	{
		// block already cached
		return UB[cached[block]]->getCharacterData(character);
	}
	else
	{
		// cache the block
		if (UB.size() < UNICODEBLOCKS_STATIC + UNICODEBLOCKS_TEMPORARY)
		{
			// just add it, until the list is full
			UB.push_back(new UnicodeBlock(_filename,block));
			position = UB.size() - 1;
			cached[block] = position;
		}
		else
		{
			// another block has to be deleted
			int swap_in_threshold = blockImpressions[UB[0]->Number()]; // smallest number to be in the static blocks
			position = 0;
			for (int a=1;a<UNICODEBLOCKS_STATIC;a++)
			{
				if (blockImpressions[UB[a]->Number()] < swap_in_threshold)
				{
					swap_in_threshold = blockImpressions[UB[a]->Number()];
					position = a;
				}
			}
			if (blockImpressions[block] <= swap_in_threshold)
			{
				_lastCached++;
				if(_lastCached>=UNICODEBLOCKS_STATIC + UNICODEBLOCKS_TEMPORARY)
					_lastCached = UNICODEBLOCKS_STATIC;
				position = _lastCached;
			}

			cached[UB[position]->Number()] = -1;
			delete UB[position];
			UB[position] = new UnicodeBlock(_filename,block);
			cached[UB[position]->Number()] = position;
		}
	}
	return UB[position]->getCharacterData(character);
}

bool Font::initOK()
{
	return _initOK;
}

UnicodeBlock::UnicodeBlock(char* filename, unsigned char blockNr)
{
	int a, b;

	_blockNr = blockNr;

	FILE* f = fopen(filename,"rb");
	if (f != NULL)
	{
		unsigned int* index_tmp = (unsigned int*) malloc(256 * sizeof(unsigned int));
		fseek(f, 0x40 + (256 * blockNr) * sizeof(unsigned int), SEEK_SET);
		fread(index_tmp, 256 * sizeof(unsigned int), 1, f);

		_size_of_data = 0;
		unsigned char unique[256];
		int used[256];

		for (a=0;a<256;a++)
		{
			used[a] = 0;
			unsigned char bbx;
			for (b=0;(b<a) && (index_tmp[a] != index_tmp[b]);b++);
			unique[a] = b;
			if (unique[a] == a)
			{
				fseek(f,0x40040+index_tmp[a]+1, SEEK_SET);
				fread(&bbx,1,1,f);
				used[a] = 3+(((bbx>>4)+1)*((bbx&0xF)+1)+7)/8;
				_size_of_data += used[a];
			}
		}

		_index = (int*) malloc( 256 * sizeof(int) );
		_data = (unsigned char*) malloc( _size_of_data );

		b = 0;
		for (a=0;a<256;a++)
		{
			if (unique[a] == a)
			{
				_index[a] = b;
				fseek(f,0x40040+index_tmp[a], SEEK_SET);
				fread(_data+b,used[a],1,f);
				b += used[a];
			}
			else
			{
				_index[a] = _index[unique[a]];
			}
		}
		free(index_tmp);
		fclose(f);
	}
}

UnicodeBlock::~UnicodeBlock()
{
	if (_index)
	{
		free(_index);
		_index = NULL;
	}
	if (_data)
	{
		free(_data);
		_data = NULL;
	}
}

unsigned char* UnicodeBlock::getCharacterData(unsigned char Uni)
{
	return &(_data[_index[Uni]]);
}

unsigned char UnicodeBlock::Number()
{
	return _blockNr;
}
