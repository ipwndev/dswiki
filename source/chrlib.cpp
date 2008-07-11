#include "chrlib.h"

Font  terminus12regular;

const CharStat CS   ={ PA_RGB(0,0,0), PA_RGB(23,23,23), 0, Deg0, NONE, 0, 0, 0, &terminus12regular};

// char U[255];
// char N[255];

// char* U2A(char* Ui)
// {
// 	u8    t;
// 	char* R=Ui;
// 	char  S[]="0123456789ABCDEF";
//
// 	for(t=0;t<4;t++)
// 	{
// 		U[2*t]  =S[((*R&0xF0)>>4)];
// 		U[2*t+1]=S[ (*R&0xF)     ];
// 		R++;
// 		//t++;
// 	}
// 	U[2*(t+1)]=0;
// 	return U;
// }

// u32 UTFLen(char* Str)
// {
// 	char* Ptr   =Str;
// 	u32  Length=0;
// 	while(Ptr[0]|Ptr[1])
// 	{
// 		Ptr+=2;
// 		Length++;
// 	}
// 	return Length;
// }


// u16 WordWidth(char* Word, CharStat *CStat, Lid Lang)
// {
// 	u16* ptrW=(u16*) Word;
// 	u8*  ptr =(u8*) Word;
// 	u16  Width=0;
//
// 	switch(Lang)
// 	{
// 		case UTF:
// 			while(ptrW[0])
// 			{
// 				if(*ptrW<128)
// //					Width+=(CStat->FONT->semiWidth + CStat->W_Space);
// 					Width+=0;
// 				else
// 					Width+=0;
// //					Width+=(CStat->FONT->Width + CStat->W_Space);
// 				ptrW++;
// 			}
// 			break;
// 		case UTF8:
// 			while(ptr[0])
// 			{
// 				if(*ptr<128)
// 				{
// //					Width+=(CStat->FONT->semiWidth + CStat->W_Space);
// 					ptr++;
// 				}
// 				else
// 				{
// 					if(*ptr>0xE0)
// 					{
// //             Width+=(CStat->FONT->Width + CStat->W_Space);
// 						ptr+=3;
// 					}
// 					else
// 					{
// //             Width+=(CStat->FONT->Width + CStat->W_Space);
// 						ptr+=2;
// 					}
// 				}
// 			}
// 			break;
// 		case BIG5:
// 		case GBK:
// 		case JIS:
// 			while(ptr[0])
// 			{
// 				if(*ptr<128)
// 				{
// //           Width+=(CStat->FONT->semiWidth + CStat->W_Space);
// 					ptr++;
// 				}
// 				else
// 				{
// //           Width+=(CStat->FONT->Width + CStat->W_Space);
// 					ptr+=2;
// 				}
// 			}
// 			break;
// 	}
// 	return Width;
// }


u8 FontInit(Font* FONT,const u8* ptr)
{
	FONT->Name       = (u16*)ptr;
	FONT->Height     = ptr[32];
	FONT->W_Space    = 0;
	FONT->Index      = (u32*)(ptr+64);
	FONT->Data       = (u8*)ptr+0x40040;
	FONT->Ptr        = (u8*)ptr;
	return 1;
}


u8 ToUTF(char* Chr,u16* UTF16, const u16* Table, Lid LID)
{
	u16   Row = 0 ;
	u16   Col = 0 ;
	u16   Line = 0 ;
	u8    Length=2;

	switch(LID)
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

// u32 UTF2UTF8(u16* Uni,u8* Utf)
// {
// 	u32 Length=0;
// 	u32 i=0;
//
// 	while(Uni[Length])
// 	{
// 		if((Uni[Length]>0)&&(Uni[Length]<=0x7F))
// 		{
// 			Utf[i++]=  Uni[Length++];
// 		}
// 		else if((Uni[Length]>0x7F)&&(Uni[Length]<=0x7FF))
// 		{
// 			Utf[i++]= (Uni[Length  ] >>6 ) | 0xC0;
// 			Utf[i++]= (Uni[Length++]&0x3F) | 0x80;
// 		}
// 		else
// 		{
// 			Utf[i++]= (Uni[Length  ] >>12) | 0xE0;
// 			Utf[i++]=((Uni[Length  ] >>6 ) & 0x3F) | 0x80 ;
// 			Utf[i++]= (Uni[Length++]       & 0x3F) | 0x80 ;
// 		}
// 	}
// 	Utf[i]=0;
// 	Utf[i+1]=0;
// 	return Length;
// }

// u32 UTF82UTF(u8* U8,u16* U)
// {
// 	u32 i=0;
// 	u32 Length=0;
// 	char* U8c = (char*) U8;
// 	while(U8c[i])
// 	{
// 		i+=ToUTF(&U8c[i],&U[Length++],0,UTF8);
// 	}
// 	U[Length]=0;
// 	return Length;
// }

// u8 STR[16];
//
// u8* ValueStr(u32 Value)
// {
// 	u8 t=15;
// 	STR[t]=0;
// 	while(Value)
// 	{
// 		STR[--t]=(Value%10)+48;
// 		Value/=10;
// 	}
// 	return &STR[t];
// }
//
// u32 len(char* Str)
// {
// 	char* Ptr=Str;
// 	u32 Length=0;
// 	u8 Below=1;
// 	while((*Ptr)&&Below)
// 	{
// 		Length++;
// 		Ptr++;
// 		if(Length==0x10000)
// 			Below=0;
// 	}
// 	return Length;
// }

void SwitchNewLine(CharStat *Status, BLOCK* CharArea, s16 Origin, u8 Height)
{
	switch(Status->Rotate)
	{
		case Deg0:
			CharArea->Start.x =Origin;
			CharArea->Start.y+=(Height+Status->H_Space);
			break;
		case Deg90:
			CharArea->End.y   =Origin;
			CharArea->Start.x+=(Height+Status->H_Space);
			break;
		case Deg180:
			CharArea->End.x   =Origin;
			CharArea->End.y  -=(Height+Status->H_Space);
			break;
		case Deg270:
			CharArea->Start.y =Origin;
			CharArea->End.x  -=(Height+Status->H_Space);
			break;
	}
}

u8 CheckLowerBound(BLOCK* PrintArea, BLOCK *CharArea,CharStat* CStat)
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
				if(CharArea->End.y > PrintArea->End.y+1)
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
				if(CharArea->End.x > PrintArea->End.x+1)
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
				if(CharArea->Start.y < PrintArea->Start.y-1)
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
				if(CharArea->Start.x < PrintArea->Start.x-1)
					OutScreen=1;
			}
			break;
	}
	return OutScreen;
}

/** Modifiziert 'CharArea', basierend auf der linken oberen Ecke, derart, dass ein Zeichen der Breite 'Width' und Höhe 'Height' in die 'PrintArea' geschrieben werden kann. Notfalls wird der Block in die nächste Zeile umgebrochen. Die rechte untere Ecke wird angepasst, damit in 'CheckLowerBound' geprüft werden kann, ob das Zeichen die 'PrintArea' nach unten verließe. 'iDrawChar' richtet sich aber *nur* nach der oberen linken Ecke.
  */
u8 CheckWrap(CharStat* Status, BLOCK* PrintArea , BLOCK* CharArea, s16 Origin, u8 Width, u8 Height)
{
	u8 wrap = 0;
	switch(Status->Rotate)
	{
		case Deg0:
			if((CharArea->Start.x+Width)>PrintArea->End.x+1)
			{
				CharArea->Start.x=Origin;
				CharArea->Start.y+=(Height+Status->H_Space);
				wrap = 1;
			}
			CharArea->End.x = CharArea->Start.x + Width  + Status->W_Space;
			CharArea->End.y = CharArea->Start.y + Height + Status->H_Space;
			break;
		case Deg90:
			if(CharArea->End.y<PrintArea->Start.y+Width-1)
			{
				CharArea->End.y   = Origin;
				CharArea->Start.x+=(Height+Status->H_Space);
				wrap = 1;
			}
			CharArea->End.x   = CharArea->Start.x + Height + Status->H_Space;
			CharArea->Start.y = CharArea->End.y   - Width  - Status->W_Space;
			break;
		case Deg180:
			if(CharArea->End.x<PrintArea->Start.x+Width-1)
			{
				CharArea->End.x = Origin;
				CharArea->End.y-=(Height+Status->H_Space);
				wrap = 1;
			}
			CharArea->Start.x = CharArea->End.x - Width - Status->W_Space;
			CharArea->Start.y = CharArea->End.y - Height- Status->H_Space;
			break;
		case Deg270:
			if(CharArea->Start.y+Width>PrintArea->End.y+1)
			{
				CharArea->Start.y=Origin;
				CharArea->End.x-=(Height+Status->H_Space);
				wrap = 1;
			}
			CharArea->End.y=CharArea->Start.y + Width + Status->W_Space;
			CharArea->Start.x=CharArea->End.x - Height- Status->H_Space;
			break;
	}
	return wrap;
}


void iDrawChar(u16* Uni, VirScreen* Screen, CharStat* CS, BLOCK CharArea)
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
	CharStat CopyCS=*CS;
	DATA=&CS->FONT->Data[CS->FONT->Index[*Uni]];

	idx=0;

	xoff = (DATA[2]>>4);
	yoff = (DATA[2]&0xF);
	bbx_w = (DATA[1]>>4)+1;
	bbx_h = (DATA[1]&0xF)+1;

	switch(CS->Fx)
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
						switch(CS->Rotate)
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
						switch(CS->Fx)
						{
							case NONE:
								DrawPoint(Screen ,X , Y, CS->Color);
								break;
							case HOLLOW:
								DrawPoint(Screen ,X-1 , Y,   CS->BgColor);
								DrawPoint(Screen ,X   , Y-1, CS->BgColor);
								DrawPoint(Screen ,X+1 , Y,   CS->BgColor);
								DrawPoint(Screen ,X   , Y+1, CS->BgColor);
								break;
							case SHADOW:
								switch(CS->Rotate)
								{
									case Deg0:
										DrawPoint(Screen ,X+1 , Y+1, CS->BgColor);
										break;
									case Deg90:
										DrawPoint(Screen ,X+1 , Y-1, CS->BgColor);
										break;
									case Deg180:
										DrawPoint(Screen ,X-1 , Y-1, CS->BgColor);
										break;
									case Deg270:
										DrawPoint(Screen ,X-1 , Y+1, CS->BgColor);
								}
								DrawPoint(Screen ,X , Y, CS->Color);
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
			DrawBlock(Screen,CharArea, CS->BgColor, 1);
			break;
	}
	switch(CS->Fx)
	{
		case NONE:
		case SHADOW:
			break;
		case HOLLOW:
		case BACKGR:
			CopyCS.Fx=NONE;
			iDrawChar(Uni,Screen,&CopyCS,CharArea);
			break;
	}
}

u32 iPrint(char* Str, VirScreen* Screen, CharStat* CStat, u32 Num, Lid Lang)
{
	u16   Uni=0;
	s16   Origin=0;
	u8    Width;
	u8    Height = CStat->FONT->Height;
	u32       Skip = 0;
	u32   SaveSkip = Skip;
	u8*   DATA;
	u8    ForceInnerWordWrap = 1;

	BLOCK PrintArea = {{0,0},{Screen->Width-1,Screen->Height-1}};
	BLOCK     CharArea  = {{0,0},{0,0}};
	BLOCK SaveCharArea  = {{0,0},{0,0}};

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
	while(Uni)
	{
		if (ForceInnerWordWrap) // Writing
		{
			if(Uni==0x0D)
			{
				Skip+=ToUTF(&Str[Skip],&Uni,B2U16,Lang);
				if(Uni==0x0A)
				{
					Skip+=ToUTF(&Str[Skip],&Uni,B2U16,Lang);
				}
				SwitchNewLine(CStat,&CharArea,Origin,Height);
				continue;
			}
			if(Uni==0x0A)
			{
				SwitchNewLine(CStat,&CharArea,Origin,Height);
				Skip+=ToUTF(&Str[Skip],&Uni,B2U16,Lang);
				continue;
			}
			if(Uni==0x20)
			{
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
				SaveSkip = Skip;
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
			if(CheckLowerBound(&PrintArea,&CharArea,CStat))
				break;

			iDrawChar(&Uni,Screen,CStat,CharArea);

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
			Skip+=ToUTF(&Str[Skip],&Uni,B2U16,Lang);
		}
		else // Collecting
		{
			if((Uni==0x0D)||(Uni==0x0A)||(Uni==0x20))
			{
				Skip = SaveSkip;
				CharArea.Start.x = SaveCharArea.Start.x;
				CharArea.Start.y = SaveCharArea.Start.y;
				CharArea.End.x   = SaveCharArea.End.x;
				CharArea.End.y   = SaveCharArea.End.y;
				Skip+=ToUTF(&Str[Skip],&Uni,B2U16,Lang);
				ForceInnerWordWrap = 1;
				continue;
			}

			DATA=&CStat->FONT->Data[CStat->FONT->Index[Uni]];
			Width = DATA[0];

			if (CheckWrap(CStat,&PrintArea,&CharArea,Origin,Width,Height))
			{
				ForceInnerWordWrap = 1;
				Skip = SaveSkip;
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
	return Skip-1;
}

u32 SimPrint(char* Str, Device* Dev, s32 x, s32 y, u16 Color, Lid Lang)
{
	VirScreen vscr={x,y,Dev->Width-x,Dev->Height-y,{{0,0},{0,0}},Deg0,Dev};
	InitVS(&vscr);
	CharStat C=CS;
	C.Color=Color;
	return iPrint(Str, &vscr, &C, 0, Lang);
}
