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

void replace_all(string & s, string pattern, string with)
{
	int a;
	a = s.find(pattern);
	while (a != (int) string::npos)
	{
		s.replace(a,pattern.length(),with);
		a = s.find(pattern,a+1);
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


void exchangeSGMLEntities(string & phrase, bool exclude_XML_entities)
{
	if ( phrase.empty() )
		return;

	int posStart = 0;
	int entityStart, entityLength;
	int posEnd;
	string entity;

	int i;
	while (posStart< (int) phrase.length())
	{
		posStart = phrase.find("&",posStart);
		if (posStart== (int) string::npos)
			return;
		posEnd = phrase.find(";",posStart);
		if (posEnd== (int) string::npos)
			return;

		entity       = phrase.substr(posStart,posEnd-posStart+1);
		entityStart  = posStart;
		entityLength = entity.length();

		// Skip the found "&"
		// Note: this also prevents double interpretations as in &amp;theta;
		posStart++;

		// each named entity has at least the form "&..;"
		// each number has at least the form "&#.;"
		// so we discard any shorter string
		if (entityLength<4)
			continue;

		// test for numeric entities
		if (entity.substr(1,1)=="#")
		{
			string number = entity.substr(2,entity.length()-3);
			if (number.empty())
				continue;

			unsigned int codepoint[] = {0,0};
			char replaced[32];

			if (number.substr(0,1)=="x")
			{
				// hexadecimal format found
				number.erase(0,1);
				int temp;
				if ((temp=number.find_first_not_of("0123456789aAbBcCdDeEfF"))!= (int) string::npos)
					continue;

				// conversion is possible
				for (i=0;i< (int) number.length();i++)
				{
					unsigned char c = number.at(i);
					if ((0x30<=c) && (c<=0x39))
						codepoint[0] = codepoint[0] * 16 + (c-0x30); // 0-9
					else if ((0x41<=c) && (c<=0x46))
						codepoint[0] = codepoint[0] * 16 + (c-0x37); // A-F
					else if ((0x61<=c) && (c<=0x66))
						codepoint[0] = codepoint[0] * 16 + (c-0x57); // a-f
				}
			}
			else
			{
				// decimal format found
				int temp;
				if ((temp=number.find_first_not_of("0123456789"))!= (int) string::npos)
					continue;

				// conversion is possible
				for (i=0;i< (int) number.length();i++)
				{
					unsigned char c = number.at(i);
					codepoint[0] = codepoint[0] * 10 + (c-0x30);
				}
			}

			// Don't interprete control characters
			if ( ((codepoint[0]<32) && ((codepoint[0]!=0x0A)||(codepoint[0]!=0x0D))) || (codepoint[0]==0xFFFE) || (codepoint[0]==0xFFFF) )
				continue;

			UTF2UTF8(codepoint,replaced);
			phrase.replace(entityStart,entityLength,replaced);

			// we can go to the next entity, because no named entity will start with "&#"
			continue;
		}

		// lastly, test for named entities, but at this point, we can check the length first
		if (entityLength>10)
			continue;


		for (i=0;i<MAX_NAMED_ENTITIES;i++)
		{
			if (entity==entities[i].entity)
			{
				if (exclude_XML_entities && (entity == "&amp;" || entity == "&lt;" || entity == "&gt;" || entity == "&apos;" || entity == "&quot;"))
					break;
				char replaced[32];
				unsigned int codepoint[] = {entities[i].codepoint,0};
				UTF2UTF8(codepoint,replaced);
				string neu = replaced;
				phrase.replace(entityStart,entity.length(),neu);
				break;
			}
		}

	}
}


string preparePhrase(string phrase, unsigned char indexNo, unsigned char indexVersion)
{
	if (indexNo==1)
		return lowerPhrase(exchangeDiacriticChars(lowerPhrase(phrase,indexVersion),indexVersion), indexVersion);
	else
		return lowerPhrase(phrase,indexVersion);
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

string FromUTF(unsigned int UTF32)
{
	char temp[16];
	unsigned int codepoint[] = {UTF32,0};
	UTF2UTF8(codepoint,temp);
	return string(temp);
}
