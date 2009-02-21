#include <PA9.h>
#include <fat.h>
#include "efs_lib.h"
#include "KT_lib.h"
#include <sys/dir.h>
#include <vector>
#include <string>
#include <algorithm>

#include "main.h"
#include "api.h"
#include "struct.h"
#include "chrlib.h"
#include "char_convert.h"
#include "Cache.h"
#include "History.h"
#include "Markup.h"
#include "Search.h"
#include "TitleIndex.h"
#include "WikiMarkupGetter.h"
#include "Dumps.h"
#include "Globals.h"
#include "PercentIndicator.h"
#include "Statusbar.h"
#include "TextBox.h"

Device UpScreen;
Device DnScreen;
CharStat NormalCS;
CharStat ContentCS;
CharStat ErrorCS;
CharStat StatusbarCS;
CharStat StatErrorCS;
VirScreen PercentArea;
VirScreen Titlebar;
VirScreen ContentWin1;
VirScreen ContentWin2;
VirScreen StatusbarVS;

#define DEBUG 0

int main(int argc, char ** argv)
{
	// PAlib initialization
	PA_Init();
	PA_InitVBL();
	KT_Init();
	KT_UseEFS();

	PA_Init16bitBg(0, 3);
	PA_Init16bitBg(1, 3);

	PA_SetBrightness(0,-31);
	PA_SetBrightness(1,-31);

	if (!PA_InitFat())
	{
		PA_OutputText(1,24,0,"Failed initializing FAT");
		return 0;
	}

	if(!EFS_Init(EFS_ONLY | EFS_DEFAULT_DEVICE, NULL))
	{
		PA_OutputText(1,24,0,"Failed initializing EFS");
		return 0;
	}

#if !DEBUG
	// intro screens from EFS
	unsigned char breakIntro = 0;
	KT_LoadGif(0, "dswiki/splash/dswiki", 0, 0);
	KT_LoadGif(1, "dswiki/splash/splash1", 0, 0);
	for (int i=-31;i<=0;i++)
	{
		PA_SetBrightness(0,i);
		PA_SetBrightness(1,i);
		if (Pad.Newpress.Anykey || Stylus.Newpress)
			breakIntro = 1;
		PA_WaitForVBL();
		PA_WaitForVBL();
	}

	for (int i=0;i<90;i++)
	{
		if (Pad.Newpress.Anykey || Stylus.Newpress)
			breakIntro = 1;
		if ((i>30) && breakIntro)
			break;
		PA_WaitForVBL();
	}
	for (int i=0;i<32;i++)
	{
		PA_SetBrightness(1,i);
		PA_WaitForVBL();
	}
	KT_LoadGif(1, "dswiki/splash/splash2_l",   0, 0);
	KT_LoadGif(1, "dswiki/splash/splash2_r", 128, 0);
	for (int i=31;i>=0;i--)
	{
		PA_SetBrightness(1,i);
		PA_WaitForVBL();
	}
	breakIntro = 0;
	for (int i=0;i<90;i++)
	{
		if (Pad.Newpress.Anykey || Stylus.Newpress)
			breakIntro = 1;
		if ((i>30) && breakIntro)
			break;
		PA_WaitForVBL();
	}
	for (int i=0;i<32;i++)
	{
		PA_SetBrightness(0,i);
		PA_SetBrightness(1,i);
		PA_WaitForVBL();
		PA_WaitForVBL();
	}
#endif

	// initializing things while not visible
	PA_SetBgPalCol(0, 0, PA_RGB(31,31,31));
	PA_SetBgPalCol(1, 0, PA_RGB(31,31,31));

	PA_Clear16bitBg(1);
	PA_Clear16bitBg(0);

	PA_InitText   (1, 2);
	PA_SetTextCol (0, 0, 0, 0);
	PA_SetTextCol (1, 0, 0, 0);
	PA_InitKeyboard(2);
	PA_KeyboardOut();

	// visible again
	PA_SetBrightness(0,0);
	PA_SetBrightness(1,0);

	// check for DSwiki's home directory
	PA_OutputText(1,0,2,"Checking \"/dswiki/\"...");
	DIR_ITER* dswikiDir = diropen ("fat:/dswiki/");
	if (dswikiDir == NULL)
	{
		PA_OutputText(1,24,2,"%c1[Failed]");
		return 0;
	}
	else
	{
		PA_OutputText(1,28,2,"%c2[OK]");
		dirclose(dswikiDir);
	}

	PA_OutputText(1,0,3,"Checking \"/dswiki/fonts/\"...");
	DIR_ITER* dswikiFontDir = diropen ("efs:/dswiki/fonts/");
	if (dswikiFontDir == NULL)
	{
		PA_OutputText(1,24,3,"%c1[Failed]");
		return 0;
	}
	else
	{
		PA_OutputText(1,28,3,"%c2[OK]");
		dirclose(dswikiFontDir);
	}

	PA_OutputText(1,0,4,"Initializing fonts...");

	Font* CompleteFont = new Font();

	if (CompleteFont->initOK())
	{
		PA_OutputText(1,28,4,"%c2[OK]");
	}
	else
	{
		PA_OutputText(1,24,4,"%c1[Failed]");
		return 0;
	}

	PA_OutputText(1,0,5,"Gathering installed wikis...");

	Dumps* d = new Dumps();
	vector<string> possibleWikis = d->getPossibleWikis();

	if (possibleWikis.size()==0)
	{
		PA_OutputText(1,26,5,"%c1[None]");
		return 0;
	}
	else
	{
		if (possibleWikis.size()<10)
			PA_OutputText(1,29,5,"%c2[%d]",possibleWikis.size());
		else
			PA_OutputText(1,28,5,"%c2[%d]",possibleWikis.size());
		PA_Sleep(60);
	}

	PA_ClearTextBg(1);

	//  TODO: Use graphical interface from now on

	int currentSelectedWiki = 0;

#if !DEBUG
	if ( possibleWikis.size() > 1 )
	{
		int loopi = 0;
		PA_OutputText(1,1,0,"Choose Wiki\n-----------");
		unsigned char updateSelectedWiki = 1;
		while(1)
		{
			if (Pad.Newpress.A)
			{
				break;
			}
			if (Pad.Newpress.Up||Pad.Newpress.Down)
			{
				currentSelectedWiki += Pad.Newpress.Down-Pad.Newpress.Up;
				if (currentSelectedWiki<0) currentSelectedWiki = 0;
				if (currentSelectedWiki>possibleWikis.size()-1) currentSelectedWiki = possibleWikis.size()-1;
				updateSelectedWiki = 1;
				PA_Sleep(10);
			}
			if (updateSelectedWiki)
			{
				for (loopi=0;loopi<possibleWikis.size();loopi++) {
					if (loopi==currentSelectedWiki)
					{
						PA_OutputText(1,2,2+loopi,"%c1%s",possibleWikis[loopi].c_str());
					}
					else
					{
						PA_OutputText(1,2,2+loopi,"%s",possibleWikis[loopi].c_str());
					}
				}
				updateSelectedWiki = 0;
			}
			PA_WaitForVBL();
		}
		PA_ClearTextBg(1);
	}
#endif

	// important variables
	KT_CreateSprite(0,0,"dswiki/icons/history",OBJ_SIZE_16X16,1,0,1,-16,-16);
	KT_CreateSprite(0,1,"dswiki/icons/history_clear",OBJ_SIZE_16X16,1,0,0,-16,-16);
	KT_CreateSprite(0,2,"dswiki/icons/reload",OBJ_SIZE_16X16,1,0,0,-16,-16);
	KT_CreateSprite(0,3,"dswiki/icons/cancel",OBJ_SIZE_16X16,1,0,0,-16,-16);
	KT_CreateSprite(0,4,"dswiki/icons/ok",OBJ_SIZE_16X16,1,0,0,-16,-16);
	KT_CreateSprite(0,5,"dswiki/icons/2uparrow",OBJ_SIZE_16X16,1,0,0,-16,-16);
	KT_CreateSprite(0,6,"dswiki/icons/1uparrow",OBJ_SIZE_16X16,1,0,0,-16,-16);
	KT_CreateSprite(0,7,"dswiki/icons/1downarrow",OBJ_SIZE_16X16,1,0,0,-16,-16);
	KT_CreateSprite(0,8,"dswiki/icons/2downarrow",OBJ_SIZE_16X16,1,0,0,-16,-16);
	KT_CreateSprite(0,9,"dswiki/icons/1leftarrow",OBJ_SIZE_16X16,1,0,0,-16,-16);
	KT_CreateSprite(0,10,"dswiki/icons/1rightarrow",OBJ_SIZE_16X16,1,0,0,-16,-16);
	KT_CreateSprite(0,11,"dswiki/icons/clear_left",OBJ_SIZE_16X16,1,0,0,-16,-16);

	Globals* g = new Globals();

	Statusbar* sb = new Statusbar();
	PercentIndicator* p = new PercentIndicator();
	g->setDumps(d);
	g->setFont(CompleteFont);
	g->setStatusbar(sb);
	g->setPercentIndicator(p);

	// Initialization of global variables
	UpScreen    = (Device)    { "U", 1, (unsigned short int*)PA_DrawBg[1], 256, 192};
	DnScreen    = (Device)    { "D", 0, (unsigned short int*)PA_DrawBg[0], 256, 192};
	NormalCS    = (CharStat)  { CompleteFont, REGULAR, 0, 0, PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), DEG0,   HARDWRAP,     NONE, 0 };
	ContentCS   = (CharStat)  { CompleteFont, REGULAR, 0, 0, PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), DEG0, NORMALWRAP,     NONE, 0 };
	ErrorCS     = (CharStat)  { CompleteFont, REGULAR, 0, 0, PA_RGB(27, 0, 0), PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), DEG0, NORMALWRAP,     NONE, 0 };
	StatusbarCS = (CharStat)  { CompleteFont, REGULAR, 1, 1, PA_RGB( 5, 5, 5), PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), DEG0,   HARDWRAP,     NONE, 0 };
	StatErrorCS = (CharStat)  { CompleteFont, REGULAR, 1, 1, PA_RGB(27, 4, 4), PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), DEG0,   HARDWRAP,     NONE, 0 };
	PercentArea = (VirScreen) { 229, 176,  27,  16, {{0,0},{0,0}}, &DnScreen}; InitVS(&PercentArea);
	Titlebar    = (VirScreen) {   0,   0, 256,  16, {{0,0},{0,0}}, &UpScreen}; InitVS(&Titlebar);
	ContentWin1 = (VirScreen) {   2,  18, 252, 172, {{0,0},{0,0}}, &UpScreen}; InitVS(&ContentWin1);
	ContentWin2 = (VirScreen) {   2,   2, 252, 172, {{0,0},{0,0}}, &DnScreen}; InitVS(&ContentWin2);
	StatusbarVS = (VirScreen) {   0, 176, 229,  16, {{0,0},{0,0}}, &DnScreen}; InitVS(&StatusbarVS);
	// End of global variables

	VirScreen  Searchbar   = {  47,  37, 162,  22, {{0,0},{0,0}}, &DnScreen}; InitVS(&Searchbar);

	CharStat       TitlebarCS = { CompleteFont, BOLD,    0, 0, PA_RGB(31,31,31), PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), DEG0,   HARDWRAP,     NONE, 0 };
	CharStat SearchResultsCS1 = { CompleteFont, REGULAR, 0, 0, PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), DEG0,     NOWRAP,     NONE, 0 };
	CharStat SearchResultsCS2 = { CompleteFont, REGULAR, 0, 0, PA_RGB(31, 0, 0), PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), DEG0,     NOWRAP,     NONE, 0 };
	CharStat SearchResultsCS3 = { CompleteFont, REGULAR, 0, 0, PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), DEG0,     NOWRAP, SIMULATE, 0 };

	BLOCK Btn_ToggleReal  = {{  3,  3},{ 18, 18}}; VirScreen Scr_ToogleReal = {0,0,0,0,Btn_ToggleReal,&DnScreen}; InitVS2(&Scr_ToogleReal);
	BLOCK Btn_Cancel      = {{ 67,  9},{ 88, 30}};
	BLOCK Btn_Reload      = {{117,  9},{138, 30}};
	BLOCK Btn_OK          = {{167,  9},{188, 30}};
	BLOCK Btn_CursorLeft  = {{ 29, 37},{ 46, 58}};
	BLOCK Btn_CursorRight = {{209, 37},{226, 58}};
	BLOCK Btn_Clear       = {{234, 37},{255, 58}};
	BLOCK Btn_PageUp      = {{234, 72},{255, 93}};
	BLOCK Btn_LineUp      = {{234, 97},{255,118}};
	BLOCK Btn_LineDown    = {{234,122},{255,143}};
	BLOCK Btn_PageDown    = {{234,147},{255,168}};

	BLOCK CharArea = {{  -20, 0},{ -20, 0}};

	ArticleSearchResult* suchergebnis = NULL;
	ArticleSearchResult* redirection  = NULL;

	string markupstr;
	string suchtitel;
	string currentTitle;

	Markup* markup = NULL;

// 	string olli = "A_aÄÆÐØÞßæĀĐĲŴǢǻȀΑαϒϔВвӨӪḀẞ⁴З";
// 	iPrint(olli+"\n" + preparePhrase(olli,0,0) + "\n"+preparePhrase(olli,0,1) + "\n"+preparePhrase(olli,0,2) + "\n\n"+preparePhrase(olli,1,0) + "\n"+preparePhrase(olli,1,1) + "\n"+preparePhrase(olli,1,2),&ContentWin1,&NormalCS,&CharArea,-1,UTF8);
// 	vector<int> olli;
// 	vector<int>::iterator it;
// 	olli.push_back(0);
// 	olli.push_back(10);
// 	olli.push_back(20);
// 	olli.push_back(30);
// 	olli.push_back(40);
// 	for (int i=0;i<olli.size();i++)
// 	{
// 		PA_OutputText(1,0,i,"%d",olli[i]);
// 	}
// 	it = olli.begin();
// 	it++;
// 	olli.insert(it,1);
// 	it = olli.end();
// 	it--;
// 	olli.insert(it++,2);
// 	olli.insert(it,3);
// 	for (int i=0;i<olli.size();i++)
// 	{
// 		PA_OutputText(1,10,i,"%d",olli[i]);
// 	}
// 	olli.erase(--it);
// 	for (int i=0;i<olli.size();i++)
// 	{
// 		PA_OutputText(1,20,i,"%d",olli[i]);
// 	}
// 	while(1);

	unsigned char  updateTitle       = 0;
	unsigned char  updateContent     = 0;
	unsigned char  updateStatusbarVS = 0;
	unsigned char  updatePercent     = 0;
	unsigned char  updateInRealTime  = 1;

	int forcedLine                   = 0;
	unsigned char  setNewHistoryItem = 1;
	unsigned char  loadArticle       = 1;

	FillVS(&Titlebar, PA_RGB( 9,16,28));

	g->getStatusbar()->clear();
	g->getPercentIndicator()->clear();
	g->getStatusbar()->displayClearAfter("Lade "+possibleWikis[currentSelectedWiki]+"...",30);

	TitleIndex* t = new TitleIndex();
	g->setTitleIndex(t);
	t->setGlobals(g);
	t->load(possibleWikis[currentSelectedWiki]);


	g->getStatusbar()->display("Initialisiere MarkupGetter...");
	WikiMarkupGetter* wmg = new WikiMarkupGetter();
	g->setWikiMarkupGetter(wmg);
	wmg->setGlobals(g);
	wmg->load(possibleWikis[currentSelectedWiki]);
	g->getStatusbar()->clearAfter(30);

	History* h = new History();
	Cache*   c = new Cache();

	Search*  s = new Search(&SearchResultsCS1,&SearchResultsCS2);
	g->setSearch(s);
	s->setGlobals(g);


	while(1)
	{
		PA_CheckLid();

		if (Stylus.Newpress)
		{
			suchtitel = markup->evaluateClick(Stylus.X,Stylus.Y);
			if (!suchtitel.empty())
			{
				forcedLine = 0;
				setNewHistoryItem = 1;
				loadArticle = 1;
			}
		}

		if ((Pad.Newpress.Left||Pad.Held.Left))
		{
			if (markup->scrollPageUp())
			{
				h->updateCurrentLine(markup->currentLine());
				updateContent = 1;
				PA_Sleep(10);
			}
		}

		if (Pad.Newpress.Right||Pad.Held.Right)
		{
			if (markup->scrollPageDown())
			{
				h->updateCurrentLine(markup->currentLine());
				updateContent = 1;
				PA_Sleep(10);
			}
		}

		if (Pad.Newpress.Up||Pad.Held.Up)
		{
			if (markup->scrollLineUp())
			{
				h->updateCurrentLine(markup->currentLine());
				updateContent = 1;
				PA_Sleep(10);
			}
		}

		if ((Pad.Newpress.Down||Pad.Held.Down))
		{
			if (markup->scrollLineDown())
			{
				h->updateCurrentLine(markup->currentLine());
				updateContent = 1;
				PA_Sleep(10);
			}
		}

		if (Pad.Newpress.A)
		{
			suchtitel.clear();
			forcedLine = 0;
			setNewHistoryItem = 1;
			loadArticle = 1;
		}

		if (Pad.Newpress.B)
		{
		}

		if (Pad.Newpress.X)
		{
			PA_Clear16bitBg(1);
			PA_Clear16bitBg(0);
			PA_ScrollKeyboardXY(24,72);

			FillVS(&StatusbarVS, PA_RGB(18,22,28));
			CharArea = (BLOCK) {{5,2},{0,0}};
			iPrint(currentTitle,&StatusbarVS,&NormalCS,&CharArea,-1,UTF8);

			char letter = 0;
			unsigned char updateSearchbar   = 1;
			unsigned char updateSuggestions = 1;
			unsigned char searchSuggestions = 1;
			unsigned char updateCursor      = 1;
			int cursorPosition = 0;
			unsigned char* Str;
			unsigned int Skip;
			unsigned int Uni;
			vector<int> offsetsUTF;

			offsetsUTF.push_back(0);
			if (!suchtitel.empty())
			{
				Str = (unsigned char*) &suchtitel.at(0);
				Skip = 0;
				while(Str[Skip])
				{
					cursorPosition++;
					Skip += ToUTF(&Str[Skip],&Uni);
					offsetsUTF.push_back(Skip);
				}
			}

			int countdown = 0;

			PA_SetSpriteXY(0, 1,  3,  3);
			PA_SetSpriteXY(0, 3, 67,  9);
			PA_SetSpriteXY(0, 4,167,  9);
			PA_SetSpriteXY(0, 5,234, 72);
			PA_SetSpriteXY(0, 6,234, 97);
			PA_SetSpriteXY(0, 7,234,122);
			PA_SetSpriteXY(0, 8,234,147);
			PA_SetSpriteXY(0, 9, 31, 39);
			PA_SetSpriteXY(0,10,209, 39);
			PA_SetSpriteXY(0,11,234, 40);


			if (!updateInRealTime)
			{
				PA_SetSpriteXY(0,0,3,3);
				PA_SetSpriteXY(0,2,117,9);
			}

			PA_WaitForVBL();

			while(1)
			{
				letter = PA_CheckKeyboard();

				if (letter > 31) { // there is a new letter
					suchtitel.insert(suchtitel.begin()+offsetsUTF[cursorPosition],letter);
					cursorPosition++;

					offsetsUTF.clear();
					offsetsUTF.push_back(0);
					if (!suchtitel.empty())
					{
						Str = (unsigned char*) &suchtitel.at(0);
						Skip = 0;
						while(Str[Skip])
						{
							Skip += ToUTF(&Str[Skip],&Uni);
							offsetsUTF.push_back(Skip);
						}
					}

					updateSearchbar = 1;
					if (updateInRealTime)
					{
						searchSuggestions = 1;
					}
					else
					{
						countdown = COUNTDOWN_START;
					}
				}

				if ((letter == PA_BACKSPACE) && (cursorPosition>0))
				{
					suchtitel.erase(offsetsUTF[cursorPosition-1],offsetsUTF[cursorPosition]-offsetsUTF[cursorPosition-1]); // Erase the last letter
					cursorPosition--;
					offsetsUTF.clear();
					offsetsUTF.push_back(0);
					if (!suchtitel.empty())
					{
						Str = (unsigned char*) &suchtitel.at(0);
						Skip = 0;
						while(Str[Skip])
						{
							Skip += ToUTF(&Str[Skip],&Uni);
							offsetsUTF.push_back(Skip);
						}
					}
					updateSearchbar = 1;
					if (updateInRealTime)
					{
						searchSuggestions = 1;
					}
					else
					{
						countdown = COUNTDOWN_START;
					}
				}

				if ( (letter == '\n') || (Pad.Newpress.A) )
				{
					suchtitel = s->currentHighlightedItem();
					forcedLine = 0;
					setNewHistoryItem = 1;
					loadArticle = 1;
					break;
				}

				if (Stylus.Newpress)
				{
					POINT S = {Stylus.X,Stylus.Y};
					if (IsInArea(Btn_Clear,S) && (!suchtitel.empty()) )
					{
						suchtitel.clear();
						offsetsUTF.clear();
						offsetsUTF.push_back(0);
						cursorPosition = 0;
						updateSearchbar = 1;
						if (updateInRealTime)
						{
							searchSuggestions = 1;
						}
						else
						{
							countdown = COUNTDOWN_START;
						}
					}
					else if (IsInArea(Btn_OK,S))
					{
						suchtitel = s->currentHighlightedItem();
						forcedLine = 0;
						setNewHistoryItem = 1;
						loadArticle = 1;
						break;
					}
					else if (IsInArea(StatusbarVS.Bound,S))
					{
						suchtitel = currentTitle;
						offsetsUTF.clear();
						offsetsUTF.push_back(0);
						cursorPosition = 0;
						if (!suchtitel.empty())
						{
							Str = (unsigned char*) &suchtitel.at(0);
							Skip = 0;
							while(Str[Skip])
							{
								cursorPosition++;
								Skip += ToUTF(&Str[Skip],&Uni);
								offsetsUTF.push_back(Skip);
							}
						}
						updateSearchbar = 1;
						if (updateInRealTime)
						{
							searchSuggestions = 1;
						}
						else
						{
							countdown = COUNTDOWN_START;
						}
					}
					else if (IsInArea(Btn_Cancel,S))
					{
						break;
					}
					else if (IsInArea(Btn_PageUp,S))
					{
						if (s->scrollPageUp())
						{
							PA_Sleep(10);
							updateSuggestions = 1;
						}
					}
					else if (IsInArea(Btn_LineUp,S))
					{
						if (s->scrollLineUp())
						{
							PA_Sleep(10);
							updateSuggestions = 1;
						}
					}
					else if (IsInArea(Btn_LineDown,S))
					{
						if (s->scrollLineDown())
						{
							PA_Sleep(10);
							updateSuggestions = 1;
						}
					}
					else if (IsInArea(Btn_PageDown,S))
					{
						if (s->scrollPageDown())
						{
							PA_Sleep(10);
							updateSuggestions = 1;
						}
					}
					else if (IsInArea(Btn_ToggleReal,S))
					{
						updateInRealTime = 1 - updateInRealTime;
						if (updateInRealTime)
						{
							PA_SetSpriteXY(0,0,-16,-16);
							PA_SetSpriteXY(0,2,-16,-16);
						}
						else
						{
							PA_SetSpriteXY(0,0,3,3);
							PA_SetSpriteXY(0,2,117,9);
						}
					}
					else if ((!updateInRealTime) && IsInArea(Btn_Reload,S))
					{
						searchSuggestions = 1;
						countdown = 0;
					}
					else if (IsInArea(Btn_CursorLeft,S))
					{
						if (cursorPosition>0)
						{
							cursorPosition--;
							updateSearchbar = 1;
						}
					}
					else if (IsInArea(Btn_CursorRight,S))
					{
						if (offsetsUTF[cursorPosition]<suchtitel.length())
						{
							cursorPosition++;
							updateSearchbar = 1;
						}
					}
				}

				if ((Pad.Newpress.Up||Pad.Held.Up))
				{
					if (s->scrollLineUp())
					{
						PA_Sleep(10);
						updateSuggestions = 1;
					}
				}

				if ((Pad.Newpress.Down||Pad.Held.Down))
				{
					if (s->scrollLineDown())
					{
						PA_Sleep(10);
						updateSuggestions = 1;
					}
				}

				if ((Pad.Newpress.Left||Pad.Held.Left))
				{
					if (s->scrollPageUp())
					{
						PA_Sleep(10);
						updateSuggestions = 1;
					}
				}

				if ((Pad.Newpress.Right||Pad.Held.Right))
				{
					if (s->scrollPageDown())
					{
						PA_Sleep(10);
						updateSuggestions = 1;
					}
				}

				if ((Pad.Newpress.L||Pad.Held.L))
				{
					if (s->scrollLongUp())
					{
						updateSuggestions = 1;
					}
				}

				if ((Pad.Newpress.R||Pad.Held.R))
				{
					if (s->scrollLongDown())
					{
						updateSuggestions = 1;
					}
				}


				if (Pad.Newpress.X)
				{
					break;
				}

				if (updateSearchbar)
				{
					FillVS(&Searchbar,PA_RGB(28,28,28));
					CharArea = (BLOCK) {{2,5},{0,0}};
					iPrint(suchtitel,&Searchbar,&SearchResultsCS1,&CharArea,-1,UTF8);
					updateCursor = 1;
					updateSearchbar = 0;
				}

				if (updateCursor) // TODO
				{
					CharArea = (BLOCK) {{2,5},{0,0}};
					iPrint(suchtitel.substr(0,offsetsUTF[cursorPosition]),&Searchbar,&SearchResultsCS3,&CharArea,-1,UTF8);
					BLOCK temp = {{CharArea.Start.x-1,2},{CharArea.Start.x-1,19}};
					DrawBlock(&Searchbar,temp,PA_RGB(20,20,20),0);
				}

				if (searchSuggestions) // load current searchstring, this is the bottleneck
				{
					s->load(suchtitel);
					updateSuggestions = 1;
					searchSuggestions = 0;
				}

				if (updateSuggestions) // update display
				{
					s->display();
					updateSuggestions = 0;
				}

				if (countdown > 0)
				{
					countdown--;
					if (countdown==0)
					{
						searchSuggestions = 1;
					}
				}
				PA_CheckLid();
				PA_WaitForVBL();
			}

			offsetsUTF.clear();

			PA_ScrollKeyboardXY(24,200);
			PA_Clear16bitBg(1);
			PA_Clear16bitBg(0);
			for (int i=0;i<12;i++)
				PA_SetSpriteXY(0,i,-16,-16);

			updateTitle = 1;
			updateContent = 1;
			updateStatusbarVS = 1;
		}

		if (Pad.Newpress.Y)
		{
		}

		if (Pad.Newpress.L)
		{
			if (h->back())
			{
				suchtitel = h->currentTitle();
				forcedLine = h->currentLine();
				setNewHistoryItem = 0;
				loadArticle = 1;
			}
		}

		if (Pad.Newpress.R)
		{
			if (h->forward())
			{
				suchtitel = h->currentTitle();
				forcedLine = h->currentLine();
				setNewHistoryItem = 0;
				loadArticle = 1;
			}
		}

		if (Pad.Newpress.Start)
		{
		}

		if (Pad.Newpress.Select)
		{
			int currentSelectedWikiBackup = currentSelectedWiki;
			if (possibleWikis.size()>1)
			{
				PA_Clear16bitBg(1);
				PA_OutputText(1,1,0,"Choose Wiki\n-----------");
				int i;
				unsigned char updateSelectedWiki = 1;
				while(1)
				{
					if (Pad.Newpress.A)
					{
						delete t;
						t = new TitleIndex();
						g->setTitleIndex(t);
						t->setGlobals(g);
						t->load(possibleWikis[currentSelectedWiki]);

						delete wmg;
						wmg = new WikiMarkupGetter();
						g->setWikiMarkupGetter(wmg);
						wmg->setGlobals(g);
						wmg->load(possibleWikis[currentSelectedWiki]);

						h->clear();
						c->clear();

						loadArticle = 1;
						break;
					}
					if (Pad.Newpress.B)
					{
						currentSelectedWiki = currentSelectedWikiBackup;
						break;
					}
					if (Pad.Newpress.Up||Pad.Newpress.Down)
					{
						currentSelectedWiki += Pad.Newpress.Down-Pad.Newpress.Up;
						if (currentSelectedWiki<0) currentSelectedWiki = 0;
						if (currentSelectedWiki>possibleWikis.size()-1) currentSelectedWiki = possibleWikis.size()-1;
						updateSelectedWiki = 1;
						PA_Sleep(10);
					}
					if (updateSelectedWiki)
					{
						for (i=0;i<possibleWikis.size();i++)
						{
							if (i==currentSelectedWiki)
								PA_OutputText(1,2,2+i,"%c1%s",possibleWikis[i].c_str());
							else
								PA_OutputText(1,2,2+i,"%s",possibleWikis[i].c_str());
						}
						updateSelectedWiki = 0;
					}
					PA_WaitForVBL();
				}
				PA_ClearTextBg(1);
				updateTitle = 1;
				updateContent = 1;
			}
		}

		if (loadArticle)
		{
			if (suchtitel.empty())
			{
				g->getStatusbar()->display("Suche zufälligen Artikel...");
				suchergebnis = t->getRandomArticle();
			}
			else
			{
				g->getStatusbar()->display("Suche "+suchtitel+"...");
				suchergebnis = t->findArticle(suchtitel,currentTitle);
			}

			if (suchergebnis!=NULL)
			{
				suchtitel.clear();
				g->getStatusbar()->display("Lade \""+suchergebnis->TitleInArchive()+"\"");

				if (c->isInCache(suchergebnis->TitleInArchive()))
				{
					g->getStatusbar()->display("Hole Markup aus dem Cache...");
					markupstr = c->getMarkup(suchergebnis->TitleInArchive());
				}
				else
				{
					g->getStatusbar()->display("Hole Markup von Disk...");
					markupstr = g->getWikiMarkupGetter()->getMarkup(suchergebnis->TitleInArchive());
// 					c->insert(suchergebnis->TitleInArchive(),markupstr);
				}

				string redirectMessage = "";
				unsigned char numberOfRedirections = 0;
				while ((numberOfRedirections<MAX_NUMBER_OF_REDIRECTIONS) && (redirection = t->isRedirect(markupstr)))
				{
					numberOfRedirections++;
					ArticleSearchResult* temp = suchergebnis;
					redirectMessage += "(\u2192 "+temp->TitleInArchive()+")\n";
					suchergebnis = redirection;
					if (c->isInCache(suchergebnis->TitleInArchive()))
					{
						g->getStatusbar()->display("Folge Umleitung aus dem Cache...");
						markupstr = c->getMarkup(suchergebnis->TitleInArchive());
					}
					else
					{
						g->getStatusbar()->display("Folge Umleitung von Disk...");
						markupstr = g->getWikiMarkupGetter()->getMarkup(suchergebnis->TitleInArchive());
// 						c->insert(suchergebnis->TitleInArchive(),markupstr);
					}
				}
				currentTitle = suchergebnis->TitleInArchive();
				markupstr = redirectMessage + markupstr;

				g->getStatusbar()->display("Formatiere Markup...");
				delete markup;
				markup = new Markup();
				g->setMarkup(markup);
				markup->setGlobals(g);

				markup->parse(markupstr);
				g->getStatusbar()->displayClearAfter("Formatierung abgeschlossen",30);

				markupstr.clear();

				markup->setCurrentLine(forcedLine);

				if (setNewHistoryItem)
				{
					h->insert(currentTitle,0);
				}

				updateTitle = 1;
				updateContent = 1;
			}
			else
			{
				g->getStatusbar()->displayErrorClearAfter("\""+suchtitel+"\" nicht gefunden...",90);
				updatePercent = 1;
			}

			loadArticle = 0;
		}

		if (updateTitle)
		{
			FillVS(&Titlebar, PA_RGB( 9,16,28));
			CharArea = (BLOCK) {{5,2},{0,0}};
			iPrint(currentTitle,&Titlebar,&TitlebarCS,&CharArea,-1,UTF8);
			updateTitle = 0;
		}

		if (updateStatusbarVS)
		{
			g->getStatusbar()->clear();
			updatePercent = 1;
			updateStatusbarVS = 0;
		}

		if (updateContent)
		{
			g->getMarkup()->draw();
			updatePercent = 1;
			updateContent = 0;
		}

		if (updatePercent)
		{
			g->getPercentIndicator()->update(markup->currentPercent());
			g->getPercentIndicator()->redraw();
			updatePercent = 0;
		}

		PA_CheckLid();
		PA_WaitForVBL();
	}
	return 0;
}