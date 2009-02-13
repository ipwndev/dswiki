#ifndef DSWIKIMAIN_H_
#define DSWIKIMAIN_H_

#include <PA9.h>
#include "struct.h"
#include "chrlib.h"

#define MAX_INDEX_VERSION 2
#define MAX_NUMBER_OF_REDIRECTIONS	  5
#define COUNTDOWN_START             120

#define PA_Sleep(n) { for (int i=0;i<(n);i++) { PA_WaitForVBL(); } }

using namespace std;

extern Device UpScreen;
extern Device DnScreen;
extern CharStat NormalCS;
extern CharStat ErrorCS;
extern CharStat ContentCS;
extern CharStat StatusbarCS;
extern CharStat StatErrorCS;
extern CharStat SearchResultsCS1;
extern CharStat SearchResultsCS2;
extern VirScreen PercentArea;
extern VirScreen Titlebar;
extern VirScreen ContentWin1;
extern VirScreen ContentWin2;
extern VirScreen StatusbarVS;

#endif
