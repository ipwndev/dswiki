#include "api.h"

unsigned char IsInArea(BLOCK Area, POINT Point)
{
	if((Point.x>Area.Start.x)&&(Point.x<Area.End.x))
	{
		if((Point.y>Area.Start.y)&&(Point.y<Area.End.y))
		{
			return 1;
		}
	}
	return 0;
}

BLOCK CreateBlock(int xoff, int yoff, int w, int h)
{
	BLOCK result;
	result.Start.x = xoff;
	result.Start.y = yoff;
	result.End.x = xoff+w-1;
	result.End.y = yoff+h-1;
	return result;
}

BLOCK Intersection(BLOCK Area1,BLOCK Area2)
{
	BLOCK Result;
	Result.Start.x=((Area1.Start.x>Area2.Start.x)?Area1.Start.x:Area2.Start.x);
	Result.Start.y=((Area1.Start.y>Area2.Start.y)?Area1.Start.y:Area2.Start.y);
	Result.End  .x=((Area1.End  .x<Area2.End  .x)?Area1.End.  x:Area2.End.  x);
	Result.End  .y=((Area1.End  .y<Area2.End  .y)?Area1.End.  y:Area2.End.  y);
	return Result;
}

void DrawPoint(const VirScreen* VScreen, int X, int Y, unsigned short int Color)
{
	if((X>=0)&&(Y>=0)&&(X<VScreen->Width)&&(Y<VScreen->Height))
	{
		if((VScreen->Left+X>=0)&&(VScreen->Top+Y>=0)&&(VScreen->Left+X<VScreen->Screen->Width)&&(VScreen->Top+Y<VScreen->Screen->Height))
		{
			PA_Put16bitPixel(VScreen->Screen->DID,VScreen->Left+X,VScreen->Top+Y,Color);
		}
	}
}

void DrawBlock(const VirScreen* VScreen, BLOCK Area, unsigned short int Color, unsigned char Fill)
{
	int ASx = Area.Start.x;
	int AEx = Area.End.x;
	int ASy = Area.Start.y;
	int AEy = Area.End.y;

	if ((ASx<=AEx)&&(ASy<=AEy)) // Regular Block
	{
		int W = 0;
		int H = 0;

		if(Fill)
		{
			int Gw    = VScreen->Width;
			int Gh    = VScreen->Height;
			if (Area.Start.x                 < 0)                        Area.Start.x = 0;
			if (Area.Start.y                 < 0)                        Area.Start.y = 0;
			if (Area.End.x                   >= Gw)                      Area.End.x   = VScreen->Width-1;
			if (Area.End.y                   >= Gh)                      Area.End.y   = VScreen->Height-1;
			if (Area.Start.x + VScreen->Left < 0)                        Area.Start.x = -VScreen->Left;
			if (Area.Start.y + VScreen->Top  < 0)                        Area.Start.y = -VScreen->Top;
			if (Area.End.x   + VScreen->Left >= VScreen->Screen->Width)  Area.End.x   = VScreen->Screen->Width-VScreen->Left-1;
			if (Area.End.y   + VScreen->Top  >= VScreen->Screen->Height) Area.End.y   = VScreen->Screen->Height-VScreen->Top-1;

			unsigned short int* DISPLAY=(unsigned short int*)VScreen->Screen->Ptr;
			unsigned char First=1;

			for(H=Area.Start.y;H<=Area.End.y;H++)
			{
				if(First)
				{
					for(W=Area.Start.x;W<=Area.End.x;W++)
					{
						DrawPoint(VScreen,W,Area.Start.y,Color);
					}
					First=0;
				}
				else
				{
					DMA_Copy(&DISPLAY[(VScreen->Top+Area.Start.y) * VScreen->Screen->Width + Area.Start.x+VScreen->Left],
					         &DISPLAY[(VScreen->Top+H)            * VScreen->Screen->Width + Area.Start.x+VScreen->Left],
					         Area.End.x-Area.Start.x+1,
				             DMA_16NOW);
				}
			}
		}
		else
		{
			for(W=Area.Start.x;W<=Area.End.x;W++)
			{
				DrawPoint(VScreen,W,Area.Start.y,Color);
				DrawPoint(VScreen,W,Area.End.y,Color);
			}
			for(H=Area.Start.y+1;H<Area.End.y;H++)
			{
				DrawPoint(VScreen,Area.Start.x,H,Color);
				DrawPoint(VScreen,Area.End.x,  H,Color);
			}
		}
	}
}

void DrawBlock(const Device* Dev, BLOCK Area, unsigned short int Color, unsigned char Fill)
{
	VirScreen VScreen = {0, 0, Dev->Width, Dev->Height, {{0,0},{0,0}}, Dev}; InitVS(&VScreen);
	DrawBlock(&VScreen, Area, Color, Fill);
}

void DrawEmboss(const VirScreen* VScreen, BLOCK Area, unsigned short int Color)
{
	if ((Area.Start.x<=Area.End.x)&&(Area.Start.y<=Area.End.y)) // Regular Block
	{
		unsigned char R=(Color>>10)&0x1F;
		unsigned char G=(Color>>5 )&0x1F;
		unsigned char B=(Color    )&0x1F;
		unsigned short int TempColor;
		int W=0,H=0;

		R=R*7/10;
		G=G*7/10;
		B=B*7/10;
		TempColor=R<<10|G<<5|B;

		DrawBlock(VScreen,Area,Color,1);
		for(W=Area.Start.x+1;W<Area.End.x;W++)
		{
			DrawPoint(VScreen,W,Area.Start.y+1,0xFFFF);
			DrawPoint(VScreen,W,Area.End.y-1,TempColor);
		}
		for(W=Area.Start.x;W<Area.End.x;W++)
		{
			DrawPoint(VScreen,W,Area.End.y,0x8000);
		}
		for(H=Area.Start.y;H<Area.End.y+1;H++)
		{
			DrawPoint(VScreen,Area.End.x,H,0x8000);
		}
		for(H=Area.Start.y+1;H<Area.End.y-1;H++)
		{
			DrawPoint(VScreen,Area.Start.x+1,H,0xFFFF);
			DrawPoint(VScreen,Area.End.x-1,H,TempColor);
		}
	}
}

void DrawGroove(const VirScreen* VScreen, BLOCK Area, unsigned short int Color)
{
	if ((Area.Start.x<=Area.End.x)&&(Area.Start.y<=Area.End.y)) // Regular Block
	{
		unsigned char R=(Color>>10)&0x1F;
		unsigned char G=(Color>>5 )&0x1F;
		unsigned char B=(Color    )&0x1F;
		unsigned short int TempColor;
		int W=0,H=0;

		R>>=1;
		G>>=1;
		B>>=1;
		TempColor=R<<10|G<<5|B;

		DrawBlock(VScreen,Area,Color,1);
		for(W=Area.Start.x+1;W<Area.End.x;W++)
		{
			DrawPoint(VScreen,W,Area.Start.y+1,0x8000);
			DrawPoint(VScreen,W,Area.End.y-1,TempColor);
		}
		for(W=Area.Start.x;W<Area.End.x;W++)
		{
			DrawPoint(VScreen,W,Area.Start.y,TempColor);
		}
		for(H=Area.Start.y;H<Area.End.y+1;H++)
		{
			DrawPoint(VScreen,Area.Start.x,H,TempColor);
		}
		for(H=Area.Start.y+1;H<Area.End.y-1;H++)
		{
			DrawPoint(VScreen,Area.Start.x+1,H,0x8000);
			DrawPoint(VScreen,Area.End.x-1,H,TempColor);
		}
	}
}

void FillVS(VirScreen* VScreen, unsigned short int Color)
{
	BLOCK fullVS = {{0,0},{VScreen->Width-1,VScreen->Height-1}};
	DrawBlock(VScreen,fullVS,Color,1);
}

void InitVS(VirScreen* VScreen)
{
	VScreen->Bound.Start.x = VScreen->Left;
	VScreen->Bound.Start.y = VScreen->Top;
	VScreen->Bound.  End.x = VScreen->Left + VScreen->Width - 1;
	VScreen->Bound.  End.y = VScreen->Top  + VScreen->Height - 1;
}

void InitVS2(VirScreen* VScreen)
{
	VScreen->Left   = VScreen->Bound.Start.x;
	VScreen->Top    = VScreen->Bound.Start.y;
	VScreen->Width  = VScreen->Bound.End.x - VScreen->Left + 1;
	VScreen->Height = VScreen->Bound.End.y - VScreen->Top + 1;
}
