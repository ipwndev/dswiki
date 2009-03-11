#ifndef DSWIKIMAIN_H_
#define DSWIKIMAIN_H_

#include <PA9.h>
#include "struct.h"
#include "chrlib.h"

#define MAX_INDEX_VERSION			2
#define MAX_NUMBER_OF_REDIRECTIONS	5
#define COUNTDOWN_START				120

#define PA_Sleep(n) { for (int i=0;i<(n);i++) { PA_WaitForVBL(); } }
#define PA_DelayText(screen,x,y,str,delay) { PA_OutputText(screen,x,y,str); if(delay>0){PA_Sleep(delay);}else{PA_WaitFor(Pad.Newpress.Anykey);} for (int a=0;a<(int)strlen(str);a++) {PA_OutputText(screen,x+a,y," ");} PA_Sleep(delay); }

using namespace std;

enum {
	SPRITE_HISTORY, SPRITE_HISTORYX, SPRITE_RELOAD, SPRITE_CANCEL, SPRITE_OK, SPRITE_2UPARROW, SPRITE_1UPARROW, SPRITE_1DOWNARROW, SPRITE_2DOWNARROW, SPRITE_1LEFTARROW, SPRITE_1RIGHTARROW, SPRITE_CLEARLEFT, SPRITE_CONFIGURE, SPRITE_BOOKMARK, SPRITE_VIEWMAG, SPRITE_FILEOPEN
};

extern bool debug;

extern Device UpScreen;
extern Device DnScreen;
extern CharStat NormalCS;

extern VirScreen PercentArea;
extern VirScreen Titlebar;
extern VirScreen ContentWin1;
extern VirScreen ContentWin0;
extern VirScreen StatusbarVS;

#endif
