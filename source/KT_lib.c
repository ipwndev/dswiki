//-----------------------------------------------------------------//
//                                                                 //
//   +--+  /--/ +---------+          +-+  +-+  +-+                 //
//   |  | /  /  |         |          | |  | |  | |                 //
//   |  |/  /   +--+   +--+          | |  +-+  | |                 //
//   |     /       |   |             | |       | +-----+           //
//   |     \       |   |             | |  +-+  | +---+ |           //
//   |  |\  \      |   |             | |  | |  | |   | |           //
//   |  | \  \     |   |   +-------+ | |  | |  | +---+ |           //
//   +--+  \__\    +---+   +-------+ +-+  +-+  +-------+           //
//                                                                 //
//  Noob Lib for EFSv2                                             //
//  BY KRAM AND TOILETKING                                         //
//  04/08/2008                                                     //
//                                                                 //
//-----------------------------------------------------------------//


#include <PA9.h>
#include <nds.h>
#include "efs_lib.h"
#include "KT_lib.h"
#include <stdio.h>
#include <stdlib.h>


u16 spritepal[256];
char * bufory[8][4];
u8 * spritegfx[3][128];
char memtype[5];
int spriteUsed[3][128];
int bgUsed[8];


void KT_Init(void){
   int i,j;
   for(i=0;i<3;i++){
      for(j=0;j<128;j++){
         spritegfx[i][j]=NULL;
      }
	}
	for(i=0;i<8;i++){
	   for(j=0;j<4;j++){
			bufory[i][j]=NULL;
		}
	}
	sprintf ( memtype, "efs:/");
}

void KT_UseEFS(void){
	sprintf( memtype, "efs:/");
}

void KT_UseFAT(void){
	sprintf( memtype, "/");
}

//FONCTIONS AUDIO


//You cannot use AS_lib and MikMod all at once, therefore you must comment on one of the functions below.


/*//<======= If you don't use MikMod, just put '/ *' before this comment.

#include <mikmod9.h>

MODULE* module;

void KT_ModFree(){
   Player_Free(module);
}

void KT_PlayMyMod(const char* name){

   char filename[1024];
	sprintf ( filename, "%s%s", memtype, name);
   FILE *file = fopen(filename,"rb");
   fseek(file, 0, SEEK_END);
   int size = ftell(file);
   rewind (file);
	u32 bufferSize = size;
	u8* buffer = (u8*)malloc(size+1);
	fread(buffer, 1, size, file);
	buffer[size] = '\0';
	module = Player_LoadMemory(buffer, bufferSize, 64, 0);

   DC_FlushAll();

	Player_Start(module);
   free(buffer);
}

//Do not erase this comment*/



/*//<======= If you don't use AS_lib, just put '/ *' before this comment.

#include "as_lib9.h"

void KT_PlayMyMP3(const char* name){
   char filename[1024];
	sprintf ( filename, "%s%s.mp3", memtype, name);
   AS_MP3StreamPlay(filename);
}

void KT_PlayMyRaw(const char* name, u8 volume, u8 pan, u8 loop, u8 prio){
	char filename[1024];
	sprintf ( filename, "%s%s.raw", memtype, name);
	FILE* rawFile = fopen (filename,"rb");
 	if (rawFile) {
		u32 rawSize;
   	fseek (rawFile , 0 , SEEK_END);
   	rawSize =  ftell (rawFile);
   	fseek(rawFile ,0,SEEK_SET);

   	u8 * rawBuffer;
   	rawBuffer = (u8*) malloc (sizeof(char)*rawSize);
   	fread (rawBuffer,1,rawSize,rawFile);

	 	DC_FlushAll();

		AS_SoundDefaultPlay (rawBuffer, rawSize, volume, pan, loop, prio);
		free(rawBuffer);
	}

	fclose (rawFile);
}

//FONCTION VIDEO

struct VIDEO_HEADER
{
  char		magic[8];
  u32		framecount;
  u16		samplesize;
  u16		finalsample;
  u16		frametype;
  char		reserved[14];
} vidhead;

void KT_PlayMyVideo(char * name, u8 screen, int speed, int enable_sound)
{
  char* sndBuf = NULL;
  u16 pixSize;
  u8 startedaudio = 0;
  u16 sndBufPointer = 0;
  int ANM = 0 ;
  int last_top = 0 ;

  char filename[1024];
  char filename2[1024];
  sprintf ( filename, "%s%s.emc", memtype, name);
  sprintf ( filename2, "%s%s.mp3", memtype, name);
  if(enable_sound){
	 AS_MP3StreamPlay(filename2);
 	 AS_MP3Pause ();
  }
  FILE* vidFile = fopen (filename, "r");
  fread(&vidhead,1,32,vidFile);
  int u=0;
  if (vidhead.samplesize > 0)
  {
    sndBuf = (char*) malloc (sizeof(char)*vidhead.samplesize*4);
  }

  PA_VBLCounterStart(ANM) ;
  int play_on = 1;
  char* pixBuf = (char*) malloc(16000);
  while(play_on)
  {
     u+=64;
    if((play_on == 1) || (PA_VBLCounter[ANM] != last_top))
    {
      last_top = PA_VBLCounter[ANM];
      if (vidhead.samplesize != 0)
      {
        fread(&sndBuf[sndBufPointer * vidhead.samplesize],1,vidhead.samplesize,vidFile);
        sndBufPointer++;
        if (sndBufPointer == 4) sndBufPointer = 0;
      }
      fread(&pixSize,2,1,vidFile);
      fread(pixBuf, 1, pixSize, vidFile);
      PA_LoadJpeg(screen,pixBuf) ;
      if(startedaudio == 0)
      {
        if(enable_sound) AS_MP3Unpause ();
        startedaudio = 1;

      }
    }
    play_on ++ ;
    if(play_on == vidhead.framecount)
    {
      play_on = 0;
      PA_VBLCounterPause(ANM) ;
    }
    if(u>>8==speed+1){
    	PA_WaitForVBL();
    	u=0;
	 }
  }
  fclose(vidFile);
  if (vidhead.samplesize != 0) free(sndBuf);
  free(pixBuf);
  if(enable_sound) AS_MP3Stop ();
}

//Do not erase this comment*/



struct STREAM_VIDEO_HEADER
{
  char		magic[8];
  u32		framecount;
  u16		samplesize;
  u16		finalsample;
  u16		frametype;
  char		reserved[14];
} streamvidhead;

FILE* streamvidFile;

int playstreamvideo=0;

u16 streampixSize;
u8 streamstartedaudio = 0;
u16 streamsndBufPointer = 0;
int streamANM = 0 ;
int streamlast_top = 0 ;
int streamplay_on = 0;
int streamscreen;
char* streamsndBuf = NULL;
char streampixBuf[16000];
int VBLcount=0;
char streamfilename[1024];
int streampos;

void KT_StreamVideoUpdate(void){
   if(streamplay_on){
      streamvidFile = fopen (streamfilename, "r");
      fseek(streamvidFile, streampos, SEEK_SET);
		if((streamplay_on == 1) || (VBLcount != streamlast_top))
		{
			streamlast_top = VBLcount;
			VBLcount++;
			if (streamvidhead.samplesize != 0)
			{
				fread(&streamsndBuf[streamsndBufPointer * streamvidhead.samplesize],1,streamvidhead.samplesize,streamvidFile);
				streampos+=streamvidhead.samplesize;
				streamsndBufPointer++;
				if (streamsndBufPointer == 4) streamsndBufPointer = 0;
			}
			fread(&streampixSize,2,1,streamvidFile);
			streampos+=2;
			fread(streampixBuf, 1, streampixSize, streamvidFile);
			streampos+=streampixSize;
 			PA_LoadJpeg(streamscreen,streampixBuf) ;
		}
		streamplay_on ++ ;
 		if(streamplay_on == streamvidhead.framecount)
		{
  			streamplay_on = 0;
		}
		fclose(streamvidFile);
	}
	else{
		if (streamvidhead.samplesize != 0) free(streamsndBuf);
	}
}

void KT_StopStreamVideo(void){
   streamplay_on=0;
   if (streamvidhead.samplesize != 0) free(streamsndBuf);
}

void KT_StreamPlayMyVideo(char * name, u8 screen)
{
  streamplay_on=1;
  sprintf ( streamfilename, "%s%s.emc", memtype, name);
  streamvidFile = fopen (streamfilename, "r");
  fread(&streamvidhead,1,32,streamvidFile);
  fclose(streamvidFile);
  if (streamvidhead.samplesize > 0)
  {
    streamsndBuf = (char*) malloc (sizeof(char)*streamvidhead.samplesize*4);
  }
  streamscreen=screen;
  VBLcount=1;
  streampos=32;
}

int KT_IsStreamVideoPlaying(void){
   if(streamplay_on!=0) return 1;
   else return 0;
}

int KT_GetCurrentFrame(void){
   return streamplay_on;
}

void KT_SetCurrentFrame(int frame){
   if(frame!=streamplay_on){
	   streamvidFile = fopen (streamfilename, "r");
	   int i=0,temp=streamplay_on;

	   if(frame<streamplay_on){
	   	temp=streamplay_on=1;
  			VBLcount=1;
  			streampos=32;
		}

	   while(i!=frame-temp){
		   if(streamplay_on){
		      fseek(streamvidFile, streampos, SEEK_SET);
				if((streamplay_on == 1) || (VBLcount != streamlast_top))
				{
					streamlast_top = VBLcount;
					VBLcount++;
					if (streamvidhead.samplesize != 0)
					{
						fread(&streamsndBuf[streamsndBufPointer * streamvidhead.samplesize],1,streamvidhead.samplesize,streamvidFile);
						streampos+=streamvidhead.samplesize;
						streamsndBufPointer++;
						if (streamsndBufPointer == 4) streamsndBufPointer = 0;
					}
					fread(&streampixSize,2,1,streamvidFile);
					streampos+=2;
					fread(streampixBuf, 1, streampixSize, streamvidFile);
					streampos+=streampixSize;
				}
				streamplay_on ++ ;
		 		if(streamplay_on == streamvidhead.framecount)
				{
		  			streamplay_on = 0;
				}
			}
			i++;
		}


		fclose(streamvidFile);
	}

}

int KT_GetFrameCount(char * name){
   int count=0;
   char filename[1024];
   sprintf ( filename, "%s%s.emc", memtype, name);
   FILE * video = fopen(filename, "r");
   if(video!=NULL){
   	fseek(video, 8, SEEK_SET);
   	char buffer[2];
   	char temp[128];
   	int i=0,j;
   	fread (buffer,1,1,video);
   	while(buffer[0]!=0){
   	   temp[i]=buffer[0];
   	   i++;
   	   fread (buffer,1,1,video);
   	}
   	for(j=i-1;j>-1;j--) count+=temp[j]<<(8*j);
	}
   fclose(video);

   return count;
}


//FONCTIONS SAVE
void KT_DataSav(const char* name, const char* data){

   FILE * File;
  	char filename[1024];

	sprintf ( filename, "%s%s", memtype, name);

	File = fopen (filename , "w" );

  		fwrite(data,1,sizeof(data)+1,File);

  	fclose (File);
}

void KT_DataRead(const char* name, int origin, long int offset, int count){
  	FILE * File;
  	char * buffer;
  	char filename[1024];

	sprintf (filename, "%s%s", memtype, name);

	File = fopen (filename , "r" );

		fseek(File, offset, origin);
  		buffer = (char*) malloc (sizeof(char)*count+1);
  		fread (buffer,1,count,File);

  	fclose (File);

	free (buffer);
}


//FONCTIONS GRAPHIQUES
void KT_LoadBmp(int screen, const char* name){

   char filename[1024];
	sprintf ( filename, "%s%s.bmp", memtype, name);
	FILE* bmpFile = fopen (filename, "rb");

	u32 bmpSize;
	fseek (bmpFile, 0 , SEEK_END);
	bmpSize = ftell (bmpFile);
	rewind (bmpFile);

	char * buffer;
	buffer = (char*) malloc (sizeof(char)*bmpSize);
	fread (buffer, 1, bmpSize, bmpFile);

	fclose (bmpFile);

	DC_FlushAll();

	PA_LoadBmp(screen, (void *)buffer);
	free(buffer);
}


void KT_LoadJpeg(int screen, const char* name){

   char filename[1024];
	sprintf ( filename, "%s%s.jpg", memtype, name);
	FILE* jpgFile = fopen (filename, "rb");

	u32 jpgSize;
	fseek (jpgFile, 0 , SEEK_END);
	jpgSize = ftell (jpgFile);
	rewind (jpgFile);

	char * buffer;
	buffer = (char*) malloc (sizeof(char)*jpgSize);
	fread (buffer, 1, jpgSize, jpgFile);

	fclose (jpgFile);

	DC_FlushAll();

	PA_LoadJpeg(screen, (void *)buffer);
	free(buffer);

}


void KT_LoadGif(int screen, const char* name, s16 x, s16 y)
{
	char filename[1024];
	sprintf ( filename, "%s%s.gif", memtype, name);
	FILE* gifFile = fopen (filename, "rb");
	if (gifFile!=NULL)
	{
		u32 gifSize;
		fseek (gifFile, 0 , SEEK_END);
		gifSize = ftell (gifFile);
		rewind (gifFile);

		char * buffer;
		buffer = (char*) malloc (sizeof(char)*gifSize);
		fread (buffer, 1, gifSize, gifFile);

		fclose (gifFile);

		DC_FlushAll();

		PA_LoadGifXY (screen, x, y, (void *)buffer);
		free(buffer);
	}
}


void KT_CreateSprite(u8 screen, u8 sprite_number, const char* name, u8  obj_shape, u8 obj_size,  u8 color_mode, u8 palette, u8 erase_palette, s16 x, s16 y)
	{

	char filename[1024];
	sprintf ( filename, "efs:/%s.gif", name);

	FILE* file;
	file= fopen (filename, "rb");
 	int size;
	fseek(file, 0, SEEK_END);
   size = ftell(file);
   rewind (file);
   u8 GLOBAL_BUFFER[size];
	fread(GLOBAL_BUFFER, 1, size, file);
	fclose(file);
 	spritegfx[screen][sprite_number] = PA_GifToTiles((void*)GLOBAL_BUFFER, spritepal);

 	spriteUsed[screen][sprite_number]=1;

 	DC_FlushAll();

 	if(screen==2) PA_DualCreateSprite(sprite_number, spritegfx[screen][sprite_number], obj_shape, obj_size, color_mode, palette, x, y);
	else PA_CreateSprite(screen, sprite_number, spritegfx[screen][sprite_number], obj_shape, obj_size, color_mode, palette, x, y);

	if(erase_palette){
 		if(screen==2) PA_DualLoadSpritePal( palette, spritepal);
 		else PA_LoadSpritePal(screen, palette, spritepal);
	}
}

void KT_DeleteSprite(u8 screen, u8 sprite_number){
   PA_DeleteSprite (screen, sprite_number);
   free(spritegfx[screen][sprite_number]);
   spriteUsed[screen][sprite_number]=0;
}


void KT_ResetSpriteSys(void){
	int i,j;
	for(i=0;i<128;i++){
		for(j=0;j<3;j++){
			if(spriteUsed[j][i]==1){
				free(spritegfx[j][i]);
				spriteUsed[j][i] = 0;
			}
		}
	}
	PA_ResetSpriteSys();
}

void KT_ResetSpriteSysScreen(int screen){
	int i;
	for(i=0;i<128;i++){
		if(spriteUsed[screen][i]==1){
			free(spritegfx[screen][i]);
			spriteUsed[screen][i] = 0;
		}
	}
	if(screen==2) PA_ResetSpriteSys();
	else PA_ResetSpriteSysScreen(screen);
}

void KT_ShowBG(u16 screen, u16 bg, const char* name)
	{

	u16 i;
	FILE*	files[4];

	int size[4];
	char filename[1024];
	sprintf ( filename, "efs:/%s_Info.bin", name);
	files[0] = fopen(filename, "rb");
	sprintf ( filename, "efs:/%s_Map.bin", name);
	files[1] = fopen(filename, "rb");
	sprintf ( filename, "efs:/%s_Pal.bin", name);
	files[2] = fopen(filename, "rb");
	sprintf ( filename, "efs:/%s_Tiles.bin", name);
	files[3] = fopen(filename, "rb");

	for (i=0;i<4;i++){
	   fseek(files[i], 0, SEEK_END);
      size[i] = ftell(files[i]);
      rewind (files[i]);
		bufory[bg+(4*screen)][i] = (char*) malloc (sizeof(char)*size[i]);
		fread(bufory[bg+(4*screen)][i], size[i], 1, files[i]);
	}

	bgUsed[bg+(4*screen)]=1;

	DC_FlushAll();

	PA_EasyBgLoadEx(screen, bg, (u32*)bufory[bg+(4*screen)][0], bufory[bg+(4*screen)][3], size[3],  bufory[bg+(4*screen)][1], size[1], bufory[bg+(4*screen)][2]);

	for ( i=0;i<4;++i) fclose(files[i]);
}

void KT_DelBG(u16 screen, u16 bg){
   u16 i;

	PA_DeleteBg (screen, bg);

	bgUsed[bg+(4*screen)]=0;

	for (i=0;i<4;i++) free(bufory[bg+(4*screen)][i]);


}

void KT_ResetBgSys(void){
	int i,j,k;
	for(k=0;k<2;k++){
		for(i=0;i<4;i++){
			if(bgUsed[i+(4*k)]==1){
			   bgUsed[i+(4*k)] = 0;
				for(j=0;j<4;j++) free(bufory[i+(4*k)][j]);
			}
		}
	}
	PA_ResetBgSys();
}

void KT_ResetBgSysScreen(int screen){
	int i,j;
	for(i=0;i<4;i++){
		if(bgUsed[i+(4*screen)]==1){
		   bgUsed[i+(4*screen)] = 0;
			for(j=0;j<4;j++) free(bufory[i+(4*screen)][j]);
		}
	}
	PA_ResetBgSysScreen(screen);
}
