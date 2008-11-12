#include "char_convert.h"
#include <string>

using namespace std;

void tolower_v0(unsigned char* data)
{
	if ( !data )
		return;

	while ( *data )
	{
		unsigned char c = *data;
		if ( c<0x80 )
			*data = tolower(c);
		data++;
	}
}


// Exact copy of to_lower_utf8 from version 1 of the indexer
// THIS IS FOR BACKWARD COMPATIBILITY, DO NOT CHANGE THIS!!!
void tolower_v1(unsigned char* data)
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


string lowerPhrase(string phrase, unsigned char indexVersion)
{
	if (phrase.empty())
		return "";

	string dest = phrase;
	unsigned char* destStr = (unsigned char*) &dest.at(0);
	switch (indexVersion)
	{
		case 0:
			tolower_v0(destStr); // in this case, manipulating the chars on byte level preserves the length
			break;
		case 1:
			tolower_v1(destStr); // in this case, manipulating the chars on byte level preserves the length
			break;
		case 2:
		default:
			unsigned int Uni;
			unsigned int Skip = 0;
			unsigned int l;
			int lbound, rbound, index;
			char temp[10];       // enough to hold any UTF-8 representation
			while(destStr[Skip])
			{
				l = ToUTF(&destStr[Skip],&Uni);
				lbound = 0;
				rbound = 984 - 1;
				while (lbound<=rbound)
				{
					index = (lbound+rbound)>>1;
					if (toLowerTable[index].UTF32_from < Uni)
						lbound = index + 1;
					else if (toLowerTable[index].UTF32_from > Uni)
						rbound = index - 1;
					else
					{
						unsigned int codepoint[] = {toLowerTable[index].UTF32_to,0};
						UTF2UTF8(codepoint,temp);
						dest.replace(Skip,l,temp);
						l = strlen(temp);
						break;
					}
				}
				Skip+=l;
			}
			break;
	}
	return dest;
}


string exchangeDiacriticChars(string phrase, unsigned char indexVersion)
{
	if (phrase.empty())
		return "";

	if (indexVersion == 0)
		return phrase;

	string dest = phrase;
	unsigned char* Str = (unsigned char*) &dest.at(0);
	unsigned int Uni;
	unsigned int Skip = 0;
	unsigned int l;
	unsigned char found = 0;
	char temp[100];
	int lbound, rbound, index;

	// for all indexVersion from 1 on, do some replacement stuff
	while(Str[Skip])
	{
		l = ToUTF(&Str[Skip],&Uni);
		if (Uni>0x7F)
		{
			for (int round=1;(round<=indexVersion) && (!found) ;round++)
			{
				lbound = 0;
				for (int i=0;i<round;i++) lbound += diaReps[i];
				rbound = -1;
				for (int i=1;i<=round;i++) rbound += diaReps[i];
				while (lbound<=rbound)
				{
					index = (lbound+rbound)>>1;
					if (diacriticsTable[index].UTF32_from < Uni)
						lbound = index + 1;
					else if (diacriticsTable[index].UTF32_from > Uni)
						rbound = index - 1;
					else
					{
						found = 1;
						unsigned int* codepoint = (unsigned int*) diacriticsTable[index].UTF32_tos;
						UTF2UTF8(codepoint,temp);
						dest.replace(Skip,l,temp);
						l = strlen(temp);
						break;
					}
				}
			}
		}
		Skip+=l;
	}
	return dest;
}

unsigned char ToUTF(const unsigned char* Chr, unsigned int* UTF32)
{
	unsigned char Length = 2;

	if(Chr[0]<0x80)
	{
		UTF32[0]=Chr[0];
		Length=1;
	}
	else if ((Chr[0]&0xE0)==0xC0)
	{
		UTF32[0]=((Chr[0]&0x1C)>>2);
		UTF32[0]=(UTF32[0]<<8)|((Chr[0]&0x3)<<6)|(Chr[1]&0x3F);
	}
	else if ((Chr[0]&0xF0)==0xE0)
	{
		UTF32[0]=((Chr[0]&0xF)<<4)|((Chr[1]&0x3C)>>2);
		UTF32[0]=(UTF32[0]<<8)|((Chr[1]&0x3)<<6)|(Chr[2]&0x3F);
		Length=3;
	}
	else
	{
		UTF32[0]=0xFFFD;
		if ((Chr[0]&0xF8)==0xF0)
			Length=4;
		else if ((Chr[0]&0xFC)==0xF8)
			Length=5;
		else if ((Chr[0]&0xFE)==0xFC)
			Length=6;
		else if ((Chr[0]&0xFF)==0xFE)
			Length=7;
		else if (Chr[0]==0xFF)
			Length=8;
		else Length=1;
	}

	return Length;
}

unsigned int UTF2UTF8(unsigned int* Uni, char* U8)
{
	unsigned int Length=0;
	unsigned int i=0;

	while(Uni[Length])
	{
		if((Uni[Length]>0)&&(Uni[Length]<=0x7F))
		{
			U8[i++]=  Uni[Length++];
		}
		else if((Uni[Length]>0x7F)&&(Uni[Length]<=0x7FF))
		{
			U8[i++]= (Uni[Length  ] >>6 ) | 0xC0;
			U8[i++]= (Uni[Length++]&0x3F) | 0x80;
		}
		else
		{
			U8[i++]= (Uni[Length  ] >>12) | 0xE0;
			U8[i++]=((Uni[Length  ] >>6 ) & 0x3F) | 0x80 ;
			U8[i++]= (Uni[Length++]       & 0x3F) | 0x80 ;
		}
	}
	U8[i]=0;
	return Length;
}

unsigned int UTF82UTF(unsigned char* U8, unsigned int* Uni)
{
	unsigned int i=0;
	unsigned int Length=0;
	while(U8[i])
	{
		i+=ToUTF(&U8[i],&Uni[Length++]);
	}
	Uni[Length]=0;
	return Length;
}
