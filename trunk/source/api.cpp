#include "api.h"

u8 IsInArea(BLOCK Area, POINT Point)
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

BLOCK CreateBlock(s32 xoff, s32 yoff, s32 w, s32 h)
{
	BLOCK result;
	result.Start.x = xoff;
	result.Start.y = yoff;
	result.End.x = xoff+w-1;
	result.End.y = yoff+h-1;
	return result;
}

BLOCK Intersection(BLOCK BLK1,BLOCK BLK2)
{
	BLOCK Result;
	Result.Start.x=((BLK1.Start.x>BLK2.Start.x)?BLK1.Start.x:BLK2.Start.x);
	Result.Start.y=((BLK1.Start.y>BLK2.Start.y)?BLK1.Start.y:BLK2.Start.y);
	Result.End  .x=((BLK1.End  .x<BLK2.End  .x)?BLK1.End.  x:BLK2.End.  x);
	Result.End  .y=((BLK1.End  .y<BLK2.End  .y)?BLK1.End.  y:BLK2.End.  y);
	return Result;
}

void DrawPoint(VirScreen* Graphic, s32 X, s32 Y, u16 Color)
{
	if((X>=0)&&(Y>=0)&&(X<Graphic->Width)&&(Y<Graphic->Height))
	{
		if((Graphic->Left+X>=0)&&(Graphic->Top+Y>=0)&&(Graphic->Left+X<Graphic->Screen->Width)&&(Graphic->Top+Y<Graphic->Screen->Height))
		{
			PA_Put16bitPixel(Graphic->Screen->DID,Graphic->Left+X,Graphic->Top+Y,Color);
		}
	}
}

void DrawBlock(VirScreen* Graphic, BLOCK Area, u16 Color, u8 Fill)
{
	s32 ASx = Area.Start.x;
	s32 AEx = Area.End.x;
	s32 ASy = Area.Start.y;
	s32 AEy = Area.End.y;

	if ((ASx<AEx)&&(ASy<AEy)) // Regular Block
	{
		s32 W = 0;
		s32 H = 0;

		if(Fill)
		{
			s32 Gw    = Graphic->Width;
			s32 Gh    = Graphic->Height;
			if (Area.Start.x                 < 0)                        Area.Start.x = 0;
			if (Area.Start.y                 < 0)                        Area.Start.y = 0;
			if (Area.End.x                   >= Gw)                      Area.End.x   = Graphic->Width-1;
			if (Area.End.y                   >= Gh)                      Area.End.y   = Graphic->Height-1;
			if (Area.Start.x + Graphic->Left < 0)                        Area.Start.x = -Graphic->Left;
			if (Area.Start.y + Graphic->Top  < 0)                        Area.Start.y = -Graphic->Top;
			if (Area.End.x   + Graphic->Left >= Graphic->Screen->Width)  Area.End.x   = Graphic->Screen->Width-Graphic->Left-1;
			if (Area.End.y   + Graphic->Top  >= Graphic->Screen->Height) Area.End.y   = Graphic->Screen->Height-Graphic->Top-1;

			u16* DISPLAY=(u16*)Graphic->Screen->Ptr;
			u8 First=1;

			for(H=Area.Start.y;H<=Area.End.y;H++)
			{
				if(First)
				{
					for(W=Area.Start.x;W<=Area.End.x;W++)
					{
						DrawPoint(Graphic,W,Area.Start.y,Color);
					}
					First=0;
				}
				else
				{
					DMA_Copy(&DISPLAY[(Graphic->Top+Area.Start.y) * Graphic->Screen->Width + Area.Start.x+Graphic->Left],
					         &DISPLAY[(Graphic->Top+H)            * Graphic->Screen->Width + Area.Start.x+Graphic->Left],
					         Area.End.x-Area.Start.x+1,
				             DMA_16NOW);
				}
			}
		}
		else
		{
			for(W=Area.Start.x;W<=Area.End.x;W++)
			{
				DrawPoint(Graphic,W,Area.Start.y,Color);
				DrawPoint(Graphic,W,Area.End.y,Color);
			}
			for(H=Area.Start.y+1;H<Area.End.y;H++)
			{
				DrawPoint(Graphic,Area.Start.x,H,Color);
				DrawPoint(Graphic,Area.End.x,  H,Color);
			}
		}
	}
}

void DrawEmboss(VirScreen* Graphic, BLOCK Area, u16 Color)
{
	if ((Area.Start.x<=Area.End.x)&&(Area.Start.y<=Area.End.y)) // Regular Block
	{
		u8 R=(Color>>10)&0x1F;
		u8 G=(Color>>5 )&0x1F;
		u8 B=(Color    )&0x1F;
		u16 TempColor;
		s32 W=0,H=0;

		R=R*7/10;
		G=G*7/10;
		B=B*7/10;
		TempColor=R<<10|G<<5|B;

		DrawBlock(Graphic,Area,Color,1);
		for(W=Area.Start.x+1;W<Area.End.x;W++)
		{
			DrawPoint(Graphic,W,Area.Start.y+1,0xFFFF);
			DrawPoint(Graphic,W,Area.End.y-1,TempColor);
		}
		for(W=Area.Start.x;W<Area.End.x;W++)
		{
			DrawPoint(Graphic,W,Area.End.y,0x8000);
		}
		for(H=Area.Start.y;H<Area.End.y+1;H++)
		{
			DrawPoint(Graphic,Area.End.x,H,0x8000);
		}
		for(H=Area.Start.y+1;H<Area.End.y-1;H++)
		{
			DrawPoint(Graphic,Area.Start.x+1,H,0xFFFF);
			DrawPoint(Graphic,Area.End.x-1,H,TempColor);
		}
	}
}

void DrawGroove(VirScreen* Graphic, BLOCK Area, u16 Color)
{
	if ((Area.Start.x<=Area.End.x)&&(Area.Start.y<=Area.End.y)) // Regular Block
	{
		u8 R=(Color>>10)&0x1F;
		u8 G=(Color>>5 )&0x1F;
		u8 B=(Color    )&0x1F;
		u16 TempColor;
		s32 W=0,H=0;

		R>>=1;
		G>>=1;
		B>>=1;
		TempColor=R<<10|G<<5|B;

		DrawBlock(Graphic,Area,Color,1);
		for(W=Area.Start.x+1;W<Area.End.x;W++)
		{
			DrawPoint(Graphic,W,Area.Start.y+1,0x8000);
			DrawPoint(Graphic,W,Area.End.y-1,TempColor);
		}
		for(W=Area.Start.x;W<Area.End.x;W++)
		{
			DrawPoint(Graphic,W,Area.Start.y,TempColor);
		}
		for(H=Area.Start.y;H<Area.End.y+1;H++)
		{
			DrawPoint(Graphic,Area.Start.x,H,TempColor);
		}
		for(H=Area.Start.y+1;H<Area.End.y-1;H++)
		{
			DrawPoint(Graphic,Area.Start.x+1,H,0x8000);
			DrawPoint(Graphic,Area.End.x-1,H,TempColor);
		}
	}
}

void FillVS(VirScreen* Graphic, u16 Color) {
	BLOCK fullVS = {{0,0},{Graphic->Width-1,Graphic->Height-1}};
	DrawBlock(Graphic,fullVS,Color,1);
}

void InitVS(VirScreen* VS)
{
	VS->Bound.Start.x = VS->Left;
	VS->Bound.Start.y = VS->Top;
	VS->Bound.  End.x = VS->Left + VS->Width-1;
	VS->Bound.  End.y = VS->Top  + VS->Height-1;
}

void InitVS2(VirScreen* VS)
{
	VS->Left   = VS->Bound.Start.x;
	VS->Top    = VS->Bound.Start.y;
	VS->Width  = VS->Bound.End.x-VS->Left +1;
	VS->Height = VS->Bound.End.y-VS->Top +1;
}
