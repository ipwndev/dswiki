#include "chrlib.h"

Font terminus12regular;

u8 FontInit(Font* FONT, const u8* ptr)
{
	FONT->Name       = (u16*)ptr;
	FONT->Height     = ptr[32];
	FONT->Index      = (u32*)(ptr+64);
	FONT->Data       = (u8*)ptr+0x40040;
	FONT->Ptr        = (u8*)ptr;
	return 1;
}

u8 ToUTF(char* Chr, u16* UTF16, const u16* Table, Lid Lang)
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
			else
			{
				if(Chr[0]&0x20)
				{
					UTF16[0]=((Chr[0]&0xF)<<4)|((Chr[1]&0x3C)>>2);
					UTF16[0]=(UTF16[0]<<8)|((Chr[1]&0x3)<<6)|(Chr[2]&0x3F);
					Length=3;
				}
				else
				{
					UTF16[0]=((Chr[0]&0x1C)>>2);
					UTF16[0]=(UTF16[0]<<8)|((Chr[0]&0x3)<<6)|(Chr[1]&0x3F);
				}
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

u32 UTF2UTF8(u16* Uni, char* U8)
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

u32 UTF82UTF(char* U8, u16* Uni)
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

void SwitchNewLine(CharStat* CStat, BLOCK* CharArea, s16 Origin, u8 Height)
{
	switch(CStat->Rotate)
	{
		case Deg0:
			CharArea->Start.x  = Origin;
			CharArea->Start.y += (Height+CStat->H_Space);
			break;
		case Deg90:
			CharArea->End.y    = Origin;
			CharArea->Start.x += (Height+CStat->H_Space);
			break;
		case Deg180:
			CharArea->End.x    = Origin;
			CharArea->End.y   -= (Height+CStat->H_Space);
			break;
		case Deg270:
			CharArea->Start.y  = Origin;
			CharArea->End.x   -= (Height+CStat->H_Space);
			break;
	}
}

u8 CheckLowerBound(CharStat* CStat, BLOCK* PrintArea, BLOCK* CharArea, u8 Height)
{
	u8 OutScreen=0;

	switch(CStat->Rotate)
	{
		case Deg0:
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
		case Deg90:
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
		case Deg180:
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
		case Deg270:
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

u8 CheckWrap(CharStat* CStat, BLOCK* PrintArea, BLOCK* CharArea, s16 Origin, u8 Width, u8 Height)
{
	u8 wrap = 0;
	switch(CStat->Rotate)
	{
		case Deg0:
			if((CharArea->Start.x+Width)>PrintArea->End.x+1)
			{
				SwitchNewLine(CStat,CharArea,Origin,Height);
				wrap = 1;
			}
			break;
		case Deg90:
			if(CharArea->End.y<PrintArea->Start.y+Width-1)
			{
				SwitchNewLine(CStat,CharArea,Origin,Height);
				wrap = 1;
			}
			break;
		case Deg180:
			if(CharArea->End.x<PrintArea->Start.x+Width-1)
			{
				SwitchNewLine(CStat,CharArea,Origin,Height);
				wrap = 1;
			}
			break;
		case Deg270:
			if(CharArea->Start.y+Width>PrintArea->End.y+1)
			{
				SwitchNewLine(CStat,CharArea,Origin,Height);
				wrap = 1;
			}
			break;
	}
	return wrap;
}

void iDrawChar(u16* Uni, VirScreen* Screen, CharStat* CStat, BLOCK CharArea)
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
							case Deg0:
								X=CharArea.Start.x + w + xoff;
								Y=CharArea.Start.y + h + yoff;
								break;
							case Deg90:
								X=CharArea.Start.x + h + yoff;
								Y=CharArea.End.y   -(w + xoff);
								break;
							case Deg180:
								X=CharArea.End.x   -(w + xoff);
								Y=CharArea.End.y   -(h + yoff);
								break;
							case Deg270:
								X=CharArea.End.x   -(h + yoff);
								Y=CharArea.Start.y +(w + xoff);
						}
						switch(CStat->Fx)
						{
							case NONE:
								DrawPoint(Screen ,X , Y, CStat->Color);
								break;
							case HOLLOW:
								DrawPoint(Screen ,X-1 , Y,   CStat->BgColor);
								DrawPoint(Screen ,X   , Y-1, CStat->BgColor);
								DrawPoint(Screen ,X+1 , Y,   CStat->BgColor);
								DrawPoint(Screen ,X   , Y+1, CStat->BgColor);
								break;
							case SHADOW:
								switch(CStat->Rotate)
								{
									case Deg0:
										DrawPoint(Screen ,X+1 , Y+1, CStat->BgColor);
										break;
									case Deg90:
										DrawPoint(Screen ,X+1 , Y-1, CStat->BgColor);
										break;
									case Deg180:
										DrawPoint(Screen ,X-1 , Y-1, CStat->BgColor);
										break;
									case Deg270:
										DrawPoint(Screen ,X-1 , Y+1, CStat->BgColor);
								}
								DrawPoint(Screen ,X , Y, CStat->Color);
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
			DrawBlock(Screen,CharArea, CStat->BgColor, 1);
			break;
	}
	switch(CStat->Fx)
	{
		case NONE:
		case SHADOW:
			break;
		case HOLLOW:
		case BACKGR:
			CopyCStat.Fx=NONE;
			iDrawChar(Uni,Screen,&CopyCStat,CharArea);
			break;
	}
}

u32 iPrint(char* Str, VirScreen* Screen, CharStat* CStat, s32 Limit, Lid Lang)
{
	u8*   DATA;
	u16   Uni                = 0;
	s16   Origin             = 0;
	u8    Width              = 0;
	u8    Height             = CStat->FONT->Height;
	u32   Skip               = 0;
	u32   SaveSkipWord       = 0;
	u32   SaveSkipLetter     = 0;
	s32   GlyphsPrinted      = 0;
	u8    ForceInnerWordWrap = 1;
	u8    HardWrap           = 0;

	BLOCK    PrintArea = {{0,0},{Screen->Width-1,Screen->Height-1}};
	BLOCK     CharArea = {{0,0},{0,0}};
	BLOCK SaveCharArea = {{0,0},{0,0}};

	switch(CStat->Rotate)
	{
		case Deg0:
			Origin           = PrintArea.Start.x;
			CharArea.Start   = PrintArea.Start;
			break;
		case Deg90:
			Origin           = PrintArea.End.y;
			CharArea.Start.x = PrintArea.Start.x;
			CharArea.End.y   = PrintArea.End.y;
			break;
		case Deg180:
			Origin           = PrintArea.End.x;
			CharArea.End     = PrintArea.End;
			break;
		case Deg270:
			Origin           = PrintArea.Start.y;
			CharArea.Start.y = PrintArea.Start.y;
			CharArea.End.x   = PrintArea.End.x;
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
				SwitchNewLine(CStat,&CharArea,Origin,Height);
				continue;
			}
			if(Uni==0x0A)
			{
				GlyphsPrinted++;
				SwitchNewLine(CStat,&CharArea,Origin,Height);
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
					case Deg0:
						CharArea.Start.x += Width + CStat->W_Space;
						break;
					case Deg90:
						CharArea.End.y   -= Width + CStat->W_Space;
						break;
					case Deg180:
						CharArea.End.x   -= Width + CStat->W_Space;
						break;
					case Deg270:
						CharArea.Start.y += Width + CStat->W_Space;
						break;
				}
				ForceInnerWordWrap = 0;
				SaveSkipLetter = Skip;
				SaveSkipWord = Skip;
				SaveCharArea.Start.x = CharArea.Start.x;
				SaveCharArea.Start.y = CharArea.Start.y;
				SaveCharArea.End.x   = CharArea.End.x;
				SaveCharArea.End.y   = CharArea.End.y;
				Skip+=ToUTF(&Str[Skip],&Uni,B2U16,Lang);
				continue;
			}

			DATA=&CStat->FONT->Data[CStat->FONT->Index[Uni]];
			Width = DATA[0];

			CheckWrap(CStat,&PrintArea,&CharArea,Origin,Width,Height);
			if(CheckLowerBound(CStat,&PrintArea,&CharArea,Height))
				break;

			iDrawChar(&Uni,Screen,CStat,CharArea);
			GlyphsPrinted++;

			switch(CStat->Rotate)
			{
				case Deg0:
					CharArea.Start.x += Width + CStat->W_Space;
					break;
				case Deg90:
					CharArea.End.y   -= Width + CStat->W_Space;
					break;
				case Deg180:
					CharArea.End.x   -= Width + CStat->W_Space;
					break;
				case Deg270:
					CharArea.Start.y += Width + CStat->W_Space;
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
				CharArea.Start.x = SaveCharArea.Start.x;
				CharArea.Start.y = SaveCharArea.Start.y;
				CharArea.End.x   = SaveCharArea.End.x;
				CharArea.End.y   = SaveCharArea.End.y;
				Skip+=ToUTF(&Str[Skip],&Uni,B2U16,Lang);
				ForceInnerWordWrap = 1;
				continue;
			}

			// collecting a normal character
			DATA=&CStat->FONT->Data[CStat->FONT->Index[Uni]];
			Width = DATA[0];

			if (CheckWrap(CStat,&PrintArea,&CharArea,Origin,Width,Height))
			{
				ForceInnerWordWrap = 1;
				Skip = SaveSkipWord;
			}
			else
			{
				switch(CStat->Rotate)
				{
					case Deg0:
						CharArea.Start.x += Width + CStat->W_Space;
						break;
					case Deg90:
						CharArea.End.y   -= Width + CStat->W_Space;
						break;
					case Deg180:
						CharArea.End.x   -= Width + CStat->W_Space;
						break;
					case Deg270:
						CharArea.Start.y += Width + CStat->W_Space;
						break;
				}
			}
			Skip+=ToUTF(&Str[Skip],&Uni,B2U16,Lang);
		}
	}

	return SaveSkipLetter;
}

u32 SimPrint(char* Str, Device* Dev, s32 x, s32 y, u16 Color, Lid Lang)
{
	VirScreen vscr={x, y, Dev->Width-x, Dev->Height-y, {{0,0},{0,0}}, Deg0, Dev}; InitVS(&vscr);
	CharStat CS = { Color, PA_RGB(31,31,31), 0, Deg0, NONE, 0, 0, 0, &terminus12regular};
	return iPrint(Str, &vscr, &CS, -1, Lang);
}
