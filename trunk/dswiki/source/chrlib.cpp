// #include "frankenstein.h"
#include "chrlib.h"

string trimPhrase(string Str)
{
	int first = Str.find_first_not_of(" ");
	int last = Str.find_last_not_of(" ");
	if ((first==string::npos) || (last==string::npos))
		return "";
	Str = Str.substr(first,last-first+1);

	first = 0;
	while((first = Str.find("  ",first))!=string::npos)
	{
		Str.erase(first,1);
	}
	return Str;
}

string lowerPhrase(string phrase)
{
	if (phrase.empty())
		return "";
	string dest = phrase;
	tolower_utf8(&dest.at(0));
	return dest;
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

string exchangeSGMLEntities(string phrase)
{
	if ( phrase.empty() )
		return phrase;

	int posStart = 0;
	int entityStart, entityLength;
	int posEnd;
	string entity;

	int i;
	u32 basis;

	while (posStart<phrase.length())
	{
		posStart = phrase.find("&",posStart);
		if (posStart==string::npos)
			return phrase;
		posEnd = phrase.find(";",posStart);
		if (posEnd==string::npos)
			return phrase;

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

			u32 codepoint[] = {0,0};
			char replaced[32];

			if (number.substr(0,1)=="x")
			{
				// hexadecimal format found
				number.erase(0,1);
				int temp;
				if ((temp=number.find_first_not_of("0123456789aAbBcCdDeEfF"))!=string::npos)
					continue;

				// conversion is possible
				for (i=0;i<number.length();i++)
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
				if ((temp=number.find_first_not_of("0123456789"))!=string::npos)
					continue;

				// conversion is possible
				for (i=0;i<number.length();i++)
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
				char replaced[32];
				u32 codepoint[] = {entities[i].codepoint,0};
				UTF2UTF8(codepoint,replaced);
				string neu = replaced;
				int l = neu.length();
				phrase.replace(entityStart,entity.length(),neu);
				break;
			}
		}

	}
	return phrase;
}

string treatNowikiText(string phrase)
{
	if (phrase.empty())
		return phrase;

	phrase = exchangeSGMLEntities(phrase);

	int pos = 0;
	while((pos = phrase.find("\n",pos))!=string::npos)
	{
		phrase.replace(pos,1," ");
	}
	pos = 0;
	while((pos = phrase.find("  ",pos))!=string::npos)
	{
		phrase.erase(pos,1);
	}
	return phrase;
}

string treatPreText(string phrase)
{
	if (phrase.empty())
		return phrase;

	phrase = exchangeSGMLEntities(phrase);

	return phrase;
}

string exchangeDiacriticCharsUTF8Phrase(string phrase)
{
	if ( phrase.empty() )
		return phrase;

	string dst = phrase;

	int i = 0;
// 	int length = dst.length();

	while ( i < dst.length() )
	{
		unsigned char c = dst[i++];
		if ( (c&0xe0)==0xc0 ) // Start-Byte of a two byte sequence
		{
			int d = ((c&0x1f)<<6) + (((unsigned char) dst[i]) & 0x3f);
			if ( (d>=0x80) && (d<=0xff) )
			{
				string ex = diacriticExchangeTable[d-0x80];
				if ( ex!="" )
				{
					dst.replace(i-1,2,ex);
				}
			}
			else
			{
				i++;
			}
		}
	}

	return dst;
}


string preparePhrase(string phrase, u8 indexNo)
{
	if (indexNo==1)
		return exchangeDiacriticCharsUTF8Phrase(lowerPhrase(trimPhrase(phrase)));
	else
		return lowerPhrase(trimPhrase(phrase));
}


u8 ToUTF(const char* Chr, u32* UTF16, const u16* Table, Lid Lang)
{
	u16  Row    = 0;
	u16  Col    = 0;
	u16  Line   = 0;
	u8   Length = 2;

	switch(Lang)
	{
		case UTF:
			UTF16[0]=Chr[1];
			UTF16[0]=(UTF16[0]<<8)+Chr[0];
			break;
		case UTF8:
			if(Chr[0]<0x80)
			{
				UTF16[0]=Chr[0];
				Length=1;
			}
			else if ((Chr[0]&0xE0)==0xC0)
			{
				UTF16[0]=((Chr[0]&0x1C)>>2);
				UTF16[0]=(UTF16[0]<<8)|((Chr[0]&0x3)<<6)|(Chr[1]&0x3F);
			}
			else if ((Chr[0]&0xF0)==0xE0)
			{
				UTF16[0]=((Chr[0]&0xF)<<4)|((Chr[1]&0x3C)>>2);
				UTF16[0]=(UTF16[0]<<8)|((Chr[1]&0x3)<<6)|(Chr[2]&0x3F);
				Length=3;
			}
			else
			{	// Correct encoding with more than three bytes, or a bad byte
				UTF16[0]=0xFFFD;
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
			break;
		case BIG5:
			Line = 157; //(7E-40+FE-A1)
			if((Chr[0] >= 0xA1) && (Chr[0] <= 0xC6))
			{
				Row = Chr[0] - 0xA1 ;
				if((Chr[1] >= 0x40) && (Chr[1] <= 0x7E))
					Col = Chr[1] - 0x40 ;
				else
				{
					if((Chr[1] >= 0xA1) && (Chr[1] <= 0xFE))
						Col = Chr[1] - 0x62 ;
				}
				UTF16[0]=Table[(Row*Line+Col)];
			}
			else
			{
				if((Chr[0] >= 0xC9) && (Chr[0] <= 0xF9))
				{
					Row = Chr[0] - 0xA3 ;
					if((Chr[1] >= 0x40) && (Chr[1] <= 0x7E))
						Col = Chr[1] - 0x40 ;
					else
					{
						if((Chr[1] >= 0xA1) && (Chr[1] <= 0xFE))
							Col = Chr[1] - 0x62 ;
					}
					UTF16[0]=Table[(Row*Line+Col)];
				}
				else
				{
					if(Chr[0]<0x80)
					{
						UTF16[0]=Chr[0];
						Length=1;
					}
				}
			}
			break;
		case GBK:
			Line = 94 ; //(FE-A1+1)
			if((Chr[0] >= 0xA1) && (Chr[0] <=0xA9))
			{
				Row = Chr[0] - 0xA1 ;
				if((Chr[1] >= 0xA1) && (Chr[1] <= 0xFE))
					Col = Chr[1] - 0xA1 ;
				UTF16[0]=Table[(Row*Line+Col)];
			}
			else
			{
				if((Chr[0] >= 0xB0) && (Chr[0] <= 0xF7))
				{
					Row = Chr[0] - 0xA6;
					if((Chr[1] >= 0xA1) && (Chr[1] <= 0xFE))
						Col = Chr[1] - 0xA1 ;
					UTF16[0]=Table[(Row*Line+Col)];
				}
				else
				{
					if(Chr[0]<0x80)
					{
						UTF16[0]=Chr[0];
						Length=1;
					}
				}
			}
			break;
		case JIS:
			Line = 188; //(7E-40+1+FC-80+1)
			if((Chr[0] >= 0xA1) && (Chr[0] <= 0xC6))
			{
				Row = Chr[0] - 0xA1 ;
				if((Chr[1] >= 0x40) && (Chr[1] <= 0x7E))
					Col = Chr[1] - 0x40 ;
				else
				{
					if((Chr[1] >= 0xA1) && (Chr[1] <= 0xFE))
						Col = Chr[1] - 0x62 ;
				}
				UTF16[0]=Table[(Row*Line+Col)];
			}
			else
			{
				if((Chr[0] >= 0xC9) && (Chr[0] <= 0xF9))
				{
					Row = Chr[0] - 0xA3 ;
					if((Chr[1] >= 0x40) && (Chr[1] <= 0x7E))
						Col = Chr[1] - 0x40 ;
					else
					{
						if((Chr[1] >= 0xA1) && (Chr[1] <= 0xFE))
							Col = Chr[1] - 0x62 ;
					}
					UTF16[0]=Table[(Row*Line+Col)];
				}
				else
				{
					if(Chr[0]<0x80)
					{
						UTF16[0]=Chr[0];
						Length=1;
					}
				}
			}
			break;
	}

	return Length;
}

u32 UTF2UTF8(u32* Uni, char* U8) // TODO
{
	u32 Length=0;
	u32 i=0;

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

u32 UTF82UTF(char* U8, u32* Uni) //TODO
{
	u32 i=0;
	u32 Length=0;
	while(U8[i])
	{
		i+=ToUTF(&U8[i],&Uni[Length++],0,UTF8);
	}
	Uni[Length]=0;
	return Length;
}

void SwitchNewLine(const CharStat* CStat, BLOCK* CharArea, s16 Origin, u8 Height)
{
	switch(CStat->Rotate)
	{
		case DEG0:
			CharArea->Start.x  = Origin;
			CharArea->Start.y += (Height+CStat->H_Space);
			break;
		case DEG90:
			CharArea->End.y    = Origin;
			CharArea->Start.x += (Height+CStat->H_Space);
			break;
		case DEG180:
			CharArea->End.x    = Origin;
			CharArea->End.y   -= (Height+CStat->H_Space);
			break;
		case DEG270:
			CharArea->Start.y  = Origin;
			CharArea->End.x   -= (Height+CStat->H_Space);
			break;
	}
}

u8 CheckLowerBound(const CharStat* CStat, BLOCK* PrintArea, BLOCK* CharArea, u8 Height)
{
	u8 OutScreen=0;

	switch(CStat->Rotate)
	{
		case DEG0:
			if(CStat->CutChar)
			{
				if(CharArea->Start.y > PrintArea->End.y)
					OutScreen=1;
			}
			else
			{
				if(CharArea->Start.y + Height - 1 > PrintArea->End.y)
					OutScreen=1;
			}
			break;
		case DEG90:
			if(CStat->CutChar)
			{
				if(CharArea->Start.x > PrintArea->End.x)
					OutScreen= 1;
			}
			else
			{
				if(CharArea->Start.x + Height - 1 > PrintArea->End.x)
					OutScreen=1;
			}
			break;
		case DEG180:
			if(CStat->CutChar)
			{
				if(CharArea->End.y < PrintArea->Start.y)
					OutScreen= 1;
			}
			else
			{
				if(CharArea->End.y - Height + 1 < PrintArea->Start.y)
					OutScreen=1;
			}
			break;
		case DEG270:
			if(CStat->CutChar)
			{
				if(CharArea->End.x < PrintArea->Start.x)
					OutScreen= 1;
			}
			else
			{
				if(CharArea->End.x - Height + 1 < PrintArea->Start.x)
					OutScreen=1;
			}
			break;
	}
	return OutScreen;
}

u8 CheckWrap(const CharStat* CStat, BLOCK* PrintArea, BLOCK* CharArea, s16 Origin, u8 Width, u8 Height, u8 doWrap)
{
	u8 wrap = 0;
	switch(CStat->Rotate)
	{
		case DEG0:
			if((CharArea->Start.x+Width)>PrintArea->End.x+1)
			{
				if (doWrap)
					SwitchNewLine(CStat,CharArea,Origin,Height);
				wrap = 1;
			}
			break;
		case DEG90:
			if(CharArea->End.y<PrintArea->Start.y+Width-1)
			{
				if (doWrap)
					SwitchNewLine(CStat,CharArea,Origin,Height);
				wrap = 1;
			}
			break;
		case DEG180:
			if(CharArea->End.x<PrintArea->Start.x+Width-1)
			{
				if (doWrap)
					SwitchNewLine(CStat,CharArea,Origin,Height);
				wrap = 1;
			}
			break;
		case DEG270:
			if(CharArea->Start.y+Width>PrintArea->End.y+1)
			{
				if (doWrap)
					SwitchNewLine(CStat,CharArea,Origin,Height);
				wrap = 1;
			}
			break;
	}
	return wrap;
}

void iDrawChar(u32* Uni, const VirScreen* VScreen, const CharStat* CStat, BLOCK CharArea)
{
	u8*  DATA;
	u8   idx;
	u8   ptr;
	u8   msk;
	s8   xoff=0;
	s8   yoff=0;
	u8   h,w;
	u8   bbx_w, bbx_h;
	u16  X=0,Y=0;
	CharStat CopyCStat=*CStat;
	DATA=&CStat->FONT->Data[CStat->FONT->Index[*Uni]];

	idx=0;

	xoff = (DATA[2]>>4);
	yoff = (DATA[2]&0xF);
	bbx_w = (DATA[1]>>4)+1;
	bbx_h = (DATA[1]&0xF)+1;

	switch(CStat->Fx)
	{
		case NONE:
		case HOLLOW:
		case SHADOW:
			for(h=0;h<(bbx_h);h++)
			{
				for(w=0;w<bbx_w;w++)
				{
					ptr=3+(idx>>3);
					msk=0x80>>(idx & 7);
					if(DATA[ptr] & msk)
					{
						switch(CStat->Rotate)
						{
							case DEG0:
								X=CharArea.Start.x + w + xoff;
								Y=CharArea.Start.y + h + yoff;
								break;
							case DEG90:
								X=CharArea.Start.x + h + yoff;
								Y=CharArea.End.y   -(w + xoff);
								break;
							case DEG180:
								X=CharArea.End.x   -(w + xoff);
								Y=CharArea.End.y   -(h + yoff);
								break;
							case DEG270:
								X=CharArea.End.x   -(h + yoff);
								Y=CharArea.Start.y +(w + xoff);
						}
						switch(CStat->Fx)
						{
							case NONE:
								DrawPoint(VScreen ,X , Y, CStat->Color);
								break;
							case HOLLOW:
								DrawPoint(VScreen ,X-1 , Y,   CStat->BgColor);
								DrawPoint(VScreen ,X   , Y-1, CStat->BgColor);
								DrawPoint(VScreen ,X+1 , Y,   CStat->BgColor);
								DrawPoint(VScreen ,X   , Y+1, CStat->BgColor);
								break;
							case SHADOW:
								switch(CStat->Rotate)
								{
									case DEG0:
										DrawPoint(VScreen ,X+1 , Y+1, CStat->BgColor);
										break;
									case DEG90:
										DrawPoint(VScreen ,X+1 , Y-1, CStat->BgColor);
										break;
									case DEG180:
										DrawPoint(VScreen ,X-1 , Y-1, CStat->BgColor);
										break;
									case DEG270:
										DrawPoint(VScreen ,X-1 , Y+1, CStat->BgColor);
								}
								DrawPoint(VScreen ,X , Y, CStat->Color);
								break;
							case BACKGR:
								break;
						}
					}
					idx++;
				}
			}
			break;
		case BACKGR:
			DrawBlock(VScreen,CharArea, CStat->BgColor, 1);
			break;
		case SIMULATE:
			break;
	}
	switch(CStat->Fx)
	{
		case NONE:
		case SHADOW:
		case SIMULATE:
			break;
		case HOLLOW:
		case BACKGR:
			CopyCStat.Fx=NONE;
			iDrawChar(Uni,VScreen,&CopyCStat,CharArea);
			break;
	}
}

// TODO: auch ein geprintetes '\n' außerhalb des Screens soll Abbruch verursachen
u32 iPrint(const char* Str, const VirScreen* VScreen, const CharStat* CStat, BLOCK* CharArea, s32 Limit, Lid Lang)
{
	u8*   DATA;
	u32   Uni                = 0;
	s16   Origin             = 0;
	u8    Width              = 0;
	u8    Height             = CStat->FONT->Height;
	u32   Skip               = 0;
	u32   SaveSkipWord       = 0;
	u32   SaveSkipLetter     = 0;
	s32   GlyphsPrinted      = 0;
	u8    ForceInnerWordWrap = 0;
	u8    HardWrap           = 0;

	if ((CStat->Wrap==HARDWRAP)||(CStat->Wrap==NOWRAP))
		HardWrap = 1;

	BLOCK    PrintArea = {{0,0},{0,0}};
	BLOCK SaveCharArea = {{CharArea->Start.x,CharArea->Start.y},{CharArea->End.x,CharArea->End.y}};

	switch(CStat->Rotate)
	{
		case DEG0:
			PrintArea.Start.x = 0;
			PrintArea.Start.y = 0;
			PrintArea.End.x   = VScreen->Width-1;
			PrintArea.End.y   = VScreen->Height-1;
			Origin            = PrintArea.Start.x;
			if (CharArea->Start.x == Origin)
				ForceInnerWordWrap = 1;
			break;
		case DEG90:
			PrintArea.Start.x = 0;
			PrintArea.Start.y = 0;
			PrintArea.End.x   = VScreen->Width-1;
			PrintArea.End.y   = VScreen->Height-1;
			Origin            = PrintArea.End.y;
			if (CharArea->End.y == Origin)
				ForceInnerWordWrap = 1;
			break;
		case DEG180:
			PrintArea.Start.x = 0;
			PrintArea.Start.y = 0;
			PrintArea.End.x   = VScreen->Width-1;
			PrintArea.End.y   = VScreen->Height-1;
			Origin            = PrintArea.End.x;
			if (CharArea->End.x == Origin)
				ForceInnerWordWrap = 1;
			break;
		case DEG270:
			PrintArea.Start.x = 0;
			PrintArea.Start.y = 0;
			PrintArea.End.x   = VScreen->Width-1;
			PrintArea.End.y   = VScreen->Height-1;
			Origin            = PrintArea.Start.y;
			if (CharArea->Start.y == Origin)
				ForceInnerWordWrap = 1;
			break;
	}

	Skip+=ToUTF(&Str[Skip],&Uni,B2U16,Lang);

	while((Limit==-1)||(GlyphsPrinted < Limit)){
		if (ForceInnerWordWrap||HardWrap) // Writing
		{
			if(Uni==0x00)
			{
				break;
			}
			if(Uni==0x0D)
			{
				GlyphsPrinted++;
				SaveSkipLetter = Skip;
				Skip+=ToUTF(&Str[Skip],&Uni,B2U16,Lang);
				if(Uni==0x0A)
				{
					GlyphsPrinted++;
					SaveSkipLetter = Skip;
					Skip+=ToUTF(&Str[Skip],&Uni,B2U16,Lang);
				}
				SwitchNewLine(CStat,CharArea,Origin,Height);
				continue;
			}
			if(Uni==0x0A)
			{
				GlyphsPrinted++;
				SwitchNewLine(CStat,CharArea,Origin,Height);
				SaveSkipLetter = Skip;
				Skip+=ToUTF(&Str[Skip],&Uni,B2U16,Lang);
				continue;
			}
			if(Uni==0x20)
			{
				GlyphsPrinted++;
				DATA=&CStat->FONT->Data[CStat->FONT->Index[Uni]];
				Width = DATA[0];
				switch(CStat->Rotate)
				{
					case DEG0:
						CharArea->Start.x += Width + CStat->W_Space;
						break;
					case DEG90:
						CharArea->End.y   -= Width + CStat->W_Space;
						break;
					case DEG180:
						CharArea->End.x   -= Width + CStat->W_Space;
						break;
					case DEG270:
						CharArea->Start.y += Width + CStat->W_Space;
						break;
				}
				ForceInnerWordWrap = 0;
				SaveSkipLetter = Skip;
				SaveSkipWord = Skip;
				SaveCharArea.Start.x = CharArea->Start.x;
				SaveCharArea.Start.y = CharArea->Start.y;
				SaveCharArea.End.x   = CharArea->End.x;
				SaveCharArea.End.y   = CharArea->End.y;
				Skip+=ToUTF(&Str[Skip],&Uni,B2U16,Lang);
				continue;
			}

			// writing a normal character
			DATA=&CStat->FONT->Data[CStat->FONT->Index[Uni]];
			Width = DATA[0];

			if (CStat->Wrap==NOWRAP)
			{
				if (!CheckWrap(CStat,&PrintArea,CharArea,Origin,Width,Height,0))
				{
					if(CheckLowerBound(CStat,&PrintArea,CharArea,Height))
						break;

					iDrawChar(&Uni,VScreen,CStat,*CharArea);
					GlyphsPrinted++;
				}
			}
			else
			{
				CheckWrap(CStat,&PrintArea,CharArea,Origin,Width,Height,1);
				if(CheckLowerBound(CStat,&PrintArea,CharArea,Height))
					break;

				iDrawChar(&Uni,VScreen,CStat,*CharArea);
				GlyphsPrinted++;
			}

			switch(CStat->Rotate)
			{
				case DEG0:
					CharArea->Start.x += Width + CStat->W_Space;
					break;
				case DEG90:
					CharArea->End.y   -= Width + CStat->W_Space;
					break;
				case DEG180:
					CharArea->End.x   -= Width + CStat->W_Space;
					break;
				case DEG270:
					CharArea->Start.y += Width + CStat->W_Space;
					break;
			}
			SaveSkipLetter = Skip;
			Skip+=ToUTF(&Str[Skip],&Uni,B2U16,Lang);
		}
		else // Collecting
		{
			if((Uni==0x00)||(Uni==0x0D)||(Uni==0x0A)||(Uni==0x20)) // Rewind
			{
				Skip = SaveSkipWord;
				CharArea->Start.x = SaveCharArea.Start.x;
				CharArea->Start.y = SaveCharArea.Start.y;
				CharArea->End.x   = SaveCharArea.End.x;
				CharArea->End.y   = SaveCharArea.End.y;
				Skip+=ToUTF(&Str[Skip],&Uni,B2U16,Lang);
				ForceInnerWordWrap = 1;
				continue;
			}

			// collecting a normal character
			DATA=&CStat->FONT->Data[CStat->FONT->Index[Uni]];
			Width = DATA[0];

			if (CheckWrap(CStat,&PrintArea,CharArea,Origin,Width,Height,1))
			{
				ForceInnerWordWrap = 1;
				Skip = SaveSkipWord;
			}
			else
			{
				switch(CStat->Rotate)
				{
					case DEG0:
						CharArea->Start.x += Width + CStat->W_Space;
						break;
					case DEG90:
						CharArea->End.y   -= Width + CStat->W_Space;
						break;
					case DEG180:
						CharArea->End.x   -= Width + CStat->W_Space;
						break;
					case DEG270:
						CharArea->Start.y += Width + CStat->W_Space;
						break;
				}
			}
			Skip+=ToUTF(&Str[Skip],&Uni,B2U16,Lang);
		}
	}

	return SaveSkipLetter;
}

u32 SimPrint(const char* Str, Device* Dev, u16 Color, Lid Lang)
{
	Font stdFont;
// 	InitFont(&stdFont,frankenstein);
	VirScreen VScreen = {0, 0, Dev->Width, Dev->Height, {{0,0},{0,0}}, Dev}; InitVS(&VScreen);
	CharStat CharStat = { &stdFont, 0,0, Color, 0, 0, DEG0, NORMALWRAP, NONE, 0};
	BLOCK CharArea = {{0,0},{0,0}};
	return iPrint(Str, &VScreen, &CharStat, &CharArea, -1, Lang);
}

u32 iPrint(const string Str, const VirScreen* VScreen, const CharStat* CStat, BLOCK* CharArea, s32 Limit, Lid Lang)
{
	if (!Str.empty())
		return iPrint(&Str.at(0), VScreen, CStat, CharArea, Limit, Lang);
	else
		return 0;
}

u32 SimPrint(const string Str, Device* Dev, u16 Color, Lid Lang)
{
	if (!Str.empty())
		return SimPrint(&Str.at(0), Dev, Color, Lang);
	else
		return 0;
}

u8 InitFont(Font* FONT, const u8* ptr)
{
	FONT->Name       = (u16*)ptr;
	FONT->Height     = ptr[32];
	FONT->Index      = (u32*)(ptr+64);
	FONT->Data       = (u8*)ptr+0x40040;
	FONT->Ptr        = (u8*)ptr;
	return 1;
}
