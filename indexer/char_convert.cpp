/*
 *  char_convert.cpp
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "char_convert.h"

#include "tc_sc.inc"

/*
	Converts characters in tradional chineses to simplified chineses. Because an index
	tables is used the convertion is rather fast.
*/
void tc2sc_utf8(char* data)
{
	printf("TC2SC\n");
	if ( !data || !*data )
		return;

	while ( *data )
	{
		unsigned char c = *data;
		if ( (c & 0xE0)==0xE0 ) // three byte encoding in utf-8 starts with 1110xxxx
		{
			// only these are used int traditional chinese encoding
			if ( (c>=0xE4)&& (c<=0xE9) )
			{
				unsigned char d = *(data+1);

				unsigned short idx = tc_secondCodePos[c-0xE4][d-0x80];
				if ( idx!=0xffff )
				{
					unsigned char e = *(data+2);
					unsigned char* second = (unsigned char*) &chars_tc[idx][1];

					while( d==*second++ )
					{
						if ( *second<e )
							second += 4;
						else
						{
							if ( *second==e )
							{
								int pos = *(second+1) + *(second+2)*0x100;

								unsigned char* sc = (unsigned char*) chars_sc + (pos*5);
								*data     = *sc++;
								*(data+1) = *sc++;
								*(data+2) = *sc++;
							}
							break;
						}
					}
				}
				data += 3;
			}
			else
				data += 3;
		}
		else
			data++;
	}
}

const unsigned char diacriticExchangeTable[] =
{
	// this table contains the char code for any diacritic char
    // 0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80 - 0x8f
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x90 - 0x9f
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xa0 - 0xaf
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xb0 - 0xbf
	0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x00, 0x43, 0x45, 0x45, 0x45, 0x45, 0x49, 0x49, 0x49, 0x49, // 0xc0 - 0xcf
	0x44, 0x4E, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x00, 0x4F, 0x55, 0x55, 0x55, 0x55, 0x59, 0x00, 0x00, // 0xd0 - 0xdf
	0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x00, 0x63, 0x65, 0x65, 0x65, 0x65, 0x69, 0x69, 0x69, 0x69, // 0xe0 - 0xef
	0x00, 0x6E, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x00, 0x6f, 0x75, 0x75, 0x75, 0x75, 0x79, 0x00, 0x00, // 0xf0 - 0xff
};

// method for no utf-8 character strings
void exchange_diacritic_chars(char* data)
{
	if ( !data || !*data )
		return;

	while ( *data )
	{
		unsigned char c = *data;
		if ( c>=0x80 )
		{
			unsigned char ex = diacriticExchangeTable[c-0x80];
			if ( ex )
				*data = ex;
		}
		*data++;
	}
}

// method for utf-8 encoded character strings
void exchange_diacritic_chars_utf8(char* data)
{
	if ( !data || !*data )
		return;

	unsigned char* dst = (unsigned char*) data;
	while ( *data )
	{
		unsigned char c = *data;
// 		if ( (c&0xc0)==0xc0 ) // beginning of a sequence with _AT LEAST_ 2 Bytes, WRONG!!!
		if ( (c&0xe0)==0xc0 ) // beginning of a sequence with _EXACTLY_ 2 Bytes
		{
			int d = ((c&0x1f)<<6) + (*(data+1)&0x3f); // Lowest 5 & Lowest 6 give one character
			if ( d>=0x80 && d<=0xff )
			{
				unsigned char ex = diacriticExchangeTable[d-0x80];
				if ( ex )
				{
					*dst++ = ex;
					data += 2;
					continue;
				}
			}

			// nothing to to, simly copy the code
			*dst++ = *data++;
			*dst++ = *data++;
		}
		else
			*dst++ = *data++;
	}

	*dst = 0x0;
}

void tolower_utf8(char* data)
{
	if ( !data )
		return;

	while ( *data )
	{
		unsigned char c = *data;
		if ( c<0x80 )
			*data = tolower(c);
		else if ( c==0xc3 && *(data+1) )
		{
			data++;

			c = *data;
			if ( c>=0x80 && c<=0x9e )
				*data = c | 0x20;
		}
		data++;
	}
}
