#include "main.h"
#include "chrlib.h"
#include "char_convert.h"


void SwitchNewLine(const CharStat* CStat, BLOCK* CharArea, s16 Origin, unsigned char Height)
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

unsigned char CheckLowerBound(const CharStat* CStat, BLOCK* PrintArea, BLOCK* CharArea, unsigned char Height)
{
	unsigned char OutScreen=0;

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

unsigned char CheckWrap(const CharStat* CStat, BLOCK* PrintArea, BLOCK* CharArea, s16 Origin, unsigned char Width, unsigned char Height, unsigned char doWrap)
{
	unsigned char wrap = 0;
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

void iDrawChar(unsigned int* Uni, const VirScreen* VScreen, const CharStat* CStat, BLOCK CharArea)
{
	unsigned char*  DATA;
	unsigned char   idx = 0;
	unsigned char   ptr;
	unsigned char   msk;
	s8   xoff=0;
	s8   yoff=0;
	unsigned char   w, h, bbx_w, bbx_h, Width, Height;
	short int  X=0, Y=0;
	CharStat CopyCStat=*CStat;

	DATA = CStat->FONT->getCharacterData(*Uni);

	bbx_w = (DATA[1]>>4)+1;
	bbx_h = (DATA[1]&0xF)+1;
	xoff = (DATA[2]>>4);
	yoff = (DATA[2]&0xF);
	Width = DATA[0];
	Height = CStat->FONT->Height();

	switch(CStat->Rotate)
	{
		case DEG0:
			CharArea.End.x = CharArea.Start.x + ( (Width - 1) + CStat->W_Space );
			CharArea.End.y = CharArea.Start.y + ( (Height - 1) + CStat->H_Space );
			break;
		case DEG90:
			CharArea.End.x = CharArea.Start.x + ( (Height - 1) + CStat->H_Space );
			CharArea.Start.y = CharArea.End.y - ( (Width - 1) + CStat->W_Space );
			break;
		case DEG180:
			CharArea.Start.x = CharArea.End.x - ( (Width - 1) + CStat->W_Space );
			CharArea.Start.y = CharArea.End.y - ( (Height - 1) + CStat->H_Space );
			break;
		case DEG270:
			CharArea.Start.x = CharArea.End.x - ( (Height - 1) + CStat->H_Space );
			CharArea.End.y = CharArea.Start.y + ( (Width - 1) + CStat->W_Space );
			break;
	}

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
								X = CharArea.Start.x + w + xoff;
								Y = CharArea.Start.y + h + yoff;
								break;
							case DEG90:
								X = CharArea.Start.x + h + yoff;
								Y = CharArea.End.y - ( w + xoff );
								break;
							case DEG180:
								X = CharArea.End.x - ( w + xoff );
								Y = CharArea.End.y - ( h + yoff );
								break;
							case DEG270:
								X = CharArea.End.x - ( h + yoff );
								Y = CharArea.Start.y + ( w + xoff );
								break;
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
							case SIMULATE:
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
unsigned int iPrint(const char* St, const VirScreen* VScreen, const CharStat* CStat, BLOCK* CharArea, int Limit, Lid Lang)
{
	unsigned char* Str                = (unsigned char*) St;
	// 	global
	unsigned char  Height             = CStat->FONT->Height();
	s16            Origin             = 0;
	// 	per character
	unsigned char* DATA;
	unsigned int   Uni                = 0;
	unsigned char  Width              = 0;
	// 	local variables
	unsigned int   Skip               = 0;
	unsigned int   SaveSkipWord       = 0;
	unsigned int   SaveSkipLetter     = 0;
	int            GlyphsPrinted      = 0;
	unsigned char  ForceInnerWordWrap = 0;
	unsigned char  HardWrap           = 0;

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

	Skip+=ToUTF(&Str[Skip],&Uni);

	while((Limit==-1)||(GlyphsPrinted < Limit))
	{
		if (ForceInnerWordWrap||HardWrap) // Writing
		{
			if(Uni==0x00)
			{
				break;
			}
			else if(Uni==0x0D)
			{
				GlyphsPrinted++;
				SaveSkipLetter = Skip;
				Skip+=ToUTF(&Str[Skip],&Uni);
				if(Uni==0x0A)
				{
					GlyphsPrinted++;
					SaveSkipLetter = Skip;
					Skip+=ToUTF(&Str[Skip],&Uni);
				}
				SwitchNewLine(CStat,CharArea,Origin,Height);
				continue;
			}
			else if(Uni==0x0A)
			{
				GlyphsPrinted++;
				SwitchNewLine(CStat,CharArea,Origin,Height);
				SaveSkipLetter = Skip;
				Skip+=ToUTF(&Str[Skip],&Uni);
				continue;
			}
			else if(Uni==0x20)
			{
				GlyphsPrinted++;
				DATA = CStat->FONT->getCharacterData(Uni);
				Width = DATA[0];
				iDrawChar(&Uni,VScreen,CStat,*CharArea);
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
				Skip+=ToUTF(&Str[Skip],&Uni);
				continue;
			}

			// writing a normal character
			DATA = CStat->FONT->getCharacterData(Uni);
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
			Skip+=ToUTF(&Str[Skip],&Uni);
		}
		else // Collecting
		{
			if((Uni==0x00)||(Uni==0x0D)||(Uni==0x0A)||(Uni==0x20)) // Rewind
			{
				Skip = SaveSkipWord;
				*CharArea = SaveCharArea;
				Skip+=ToUTF(&Str[Skip],&Uni);
				ForceInnerWordWrap = 1;
				continue;
			}

			// collecting a normal character
			DATA = CStat->FONT->getCharacterData(Uni);
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
			Skip+=ToUTF(&Str[Skip],&Uni);
		}
	}
	return SaveSkipLetter;
}

unsigned int SimPrint(const char* Str, Device* Dev, unsigned short int Color, Lid Lang)
{
	Font stdFont("efs:/dswiki/fonts/font_r.dat");
	VirScreen VScreen = {0, 0, Dev->Width, Dev->Height, {{0,0},{0,0}}, Dev}; InitVS(&VScreen);
	CharStat CharStat = { &stdFont, 0, 0, Color, 0, 0, DEG0, NORMALWRAP, NONE, 0};
	BLOCK CharArea = {{0,0},{0,0}};
	return iPrint(Str, &VScreen, &CharStat, &CharArea, -1, Lang);
}

unsigned int iPrint(const string Str, const VirScreen* VScreen, const CharStat* CStat, BLOCK* CharArea, int Limit, Lid Lang)
{
	if (!Str.empty())
		return iPrint(Str.c_str(), VScreen, CStat, CharArea, Limit, Lang);
	else
		return 0;
}

unsigned int SimPrint(const string Str, Device* Dev, unsigned short int Color, Lid Lang)
{
	if (!Str.empty())
		return SimPrint(Str.c_str(), Dev, Color, Lang);
	else
		return 0;
}

