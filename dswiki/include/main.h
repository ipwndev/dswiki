#ifndef _DSWIKIMAIN_H
#define _DSWIKIMAIN_H

#include <PA9.h>
#include "struct.h"
#include "chrlib.h"

#define MAX_NUMBER_OF_REDIRECTIONS	  5
#define MAX_SUGGESTIONS               5
#define COUNTDOWN_START             120

#define PA_Sleep(n) { for (int sleeploopvar=0;sleeploopvar<( n );sleeploopvar++) { PA_WaitForVBL(); } }

using namespace std;

extern Device UpScreen;
extern Device DnScreen;
extern CharStat NormalCS;
extern VirScreen PercentArea;
extern VirScreen Titlebar;
extern VirScreen ContentWin1;
extern VirScreen ContentWin2;
extern VirScreen Statusbar;

#endif
