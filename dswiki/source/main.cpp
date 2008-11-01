#include <PA9.h>
#include <fat.h>
#include <sys/dir.h>
#include <vector>
#include <string>
#include <algorithm>

#include "main.h"
#include "api.h"
#include "struct.h"
#include "chrlib.h"
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
CharStat StatusbarCS;
CharStat StatErrorCS;
VirScreen PercentArea;
VirScreen Titlebar;
VirScreen ContentWin1;
VirScreen ContentWin2;
VirScreen StatusbarVS;

int main(int argc, char ** argv)
{
	// PAlib initialization
	PA_Init();
	PA_InitVBL();

	PA_Init16bitBg(0, 3);
	PA_Init16bitBg(1, 3);

	PA_InitText   (1, 2);

	PA_InitKeyboard(2);
	PA_KeyboardOut();

	PA_SetBgPalCol(0, 0, PA_RGB(31,31,31));
	PA_SetBgPalCol(1, 0, PA_RGB(31,31,31));

	PA_SetTextCol (0, 0, 0, 0);
	PA_SetTextCol (1, 0, 0, 0);

	// start of main program

	Globals* g = new Globals();

	Statusbar* sb = new Statusbar();
	g->setStatusbar(sb);

	PA_ClearTextBg(1);
	PA_OutputText(1,0,0,"Initializing FAT...");
	if (!PA_InitFat())
	{
		PA_OutputText(1,24,0,"%c1[Failed]");
		return 0;
	}
	else
	{
		PA_OutputText(1,28,0,"%c2[OK]");
	}

	// check for DSwiki's home directory
	PA_OutputText(1,0,1,"Checking \"/dswiki/\"...");
	DIR_ITER* dswikiDir = diropen ("fat:/dswiki/");
	if (dswikiDir == NULL)
	{
		PA_OutputText(1,24,1,"%c1[Failed]");
		return 0;
	}
	else
	{
		PA_OutputText(1,28,1,"%c2[OK]");
		dirclose(dswikiDir);
	}

	PA_OutputText(1,0,2,"Checking \"/dswiki/fonts/\"...");
	DIR_ITER* dswikiFontDir = diropen ("fat:/dswiki/fonts/");
	if (dswikiFontDir == NULL)
	{
		PA_OutputText(1,24,2,"%c1[Failed]");
		return 0;
	}
	else
	{
		PA_OutputText(1,28,2,"%c2[OK]");
		dirclose(dswikiFontDir);
	}

	PA_OutputText(1,0,3,"Initializing fonts...");

	Font CompleteFont;

	if (CompleteFont.initOK())
	{
		PA_OutputText(1,28,3,"%c2[OK]");
	}
	else
	{
		PA_OutputText(1,24,3,"%c1[Failed]");
	}

	PA_OutputText(1,0,4,"Gathering installed wikis...");

	Dumps* d = new Dumps();
	g->setDumps(d);

	vector<string> possibleWikis = g->getDumps()->getPossibleWikis();

	if (possibleWikis.size()==0)
	{
		PA_OutputText(1,26,4,"%c1[None]");
		return 0;
	}
	else
	{
		if (possibleWikis.size()<10)
			PA_OutputText(1,29,4,"%c2[%d]",possibleWikis.size());
		else
			PA_OutputText(1,28,4,"%c2[%d]",possibleWikis.size());
		PA_Sleep(60);
	}

// 	for (int i=0;i<possibleWikis.size();i++)
// 	{
// 		PA_ClearTextBg(0);
// 		PA_OutputText(0,0,0,"%s",possibleWikis[i].c_str());
// 		PA_OutputText(0,0,1,"%s",d.get_ifo(possibleWikis[i]).c_str());
// 		PA_OutputText(0,0,2,"%s",d.get_idx(possibleWikis[i]).c_str());
// 		PA_OutputText(0,0,3,"%s",d.get_ao1(possibleWikis[i]).c_str());
// 		PA_OutputText(0,0,4,"%s",d.get_ao2(possibleWikis[i]).c_str());
// 		vector<string> dbs = d.get_dbs(possibleWikis[i]);
// 		for (int j=0;j<dbs.size();j++)
// 		{
// 			PA_OutputText(0,1,5+j,"%s",dbs[j].c_str());
// 		}
// 		PA_WaitFor(Pad.Anykey.Newpress);
// 		PA_Sleep(120);
// 	}
// 	PA_OutputText(1,0,23,"Press any key...");
// 	PA_WaitFor(Pad.Anykey.Newpress);

// 	PA_ClearTextBg(0);
	PA_ClearTextBg(1);

	//  TODO: Use graphical interface from now on


	int currentSelectedWiki = 0;

// 	if ( possibleWikis.size() > 1 )
// 	{
// 		int loopi = 0;
// 		PA_OutputText(1,1,0,"Choose Wiki\n-----------");
// 		u8 updateSelectedWiki = 1;
// 		while(1)
// 		{
// 			if (Pad.A.Newpress)
// 			{
// 				break;
// 			}
// 			if (Pad.Up.Newpress||Pad.Down.Newpress)
// 			{
// 				currentSelectedWiki += Pad.Down.Newpress-Pad.Up.Newpress;
// 				if (currentSelectedWiki<0) currentSelectedWiki = 0;
// 				if (currentSelectedWiki>possibleWikis.size()-1) currentSelectedWiki = possibleWikis.size()-1;
// 				updateSelectedWiki = 1;
// 				PA_Sleep(10);
// 			}
// 			if (updateSelectedWiki)
// 			{
// 				for (loopi=0;loopi<possibleWikis.size();loopi++) {
// 					if (loopi==currentSelectedWiki)
// 					{
// 						PA_OutputText(1,2,2+loopi,"%c1%s",possibleWikis[loopi].c_str());
// 					}
// 					else
// 					{
// 						PA_OutputText(1,2,2+loopi,"%s",possibleWikis[loopi].c_str());
// 					}
// 				}
// 				updateSelectedWiki = 0;
// 			}
// 			PA_WaitForVBL();
// 		}
// 		PA_ClearTextBg(1);
// 	}


	// important variables

	PercentIndicator* p = new PercentIndicator();
	g->setPercentIndicator(p);


	// Initialization of global variables
	UpScreen    = (Device)    { "U", 1, (u16*)PA_DrawBg[1], 256, 192};
	DnScreen    = (Device)    { "D", 0, (u16*)PA_DrawBg[0], 256, 192};
	NormalCS    = (CharStat)  { &CompleteFont, REGULAR, 0, 0, PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), DEG0,   HARDWRAP,     NONE, 0 };
	StatusbarCS = (CharStat)  { &CompleteFont, REGULAR, 1, 1, PA_RGB( 5, 5, 5), PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), DEG0,   HARDWRAP,     NONE, 0 };
	StatErrorCS = (CharStat)  { &CompleteFont, REGULAR, 1, 1, PA_RGB(27, 4, 4), PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), DEG0,   HARDWRAP,     NONE, 0 };
	PercentArea = (VirScreen) { 229, 176,  27,  16, {{0,0},{0,0}}, &DnScreen}; InitVS(&PercentArea);
	Titlebar    = (VirScreen) {   0,   0, 256,  16, {{0,0},{0,0}}, &UpScreen}; InitVS(&Titlebar);
	ContentWin1 = (VirScreen) {   2,  18, 252, 172, {{0,0},{0,0}}, &UpScreen}; InitVS(&ContentWin1);
	ContentWin2 = (VirScreen) {   2,   2, 252, 172, {{0,0},{0,0}}, &DnScreen}; InitVS(&ContentWin2);
	StatusbarVS = (VirScreen) {   0, 176, 229,  16, {{0,0},{0,0}}, &DnScreen}; InitVS(&StatusbarVS);
	// End of global variables

	VirScreen  Searchbar   = {  47,  37, 162,  22, {{0,0},{0,0}}, &DnScreen}; InitVS(&Searchbar);

	CharStat       TitlebarCS = { &CompleteFont, REGULAR, 1, 1, PA_RGB(31,31,31), PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), DEG0,   HARDWRAP,     NONE, 0 };
	CharStat        ContentCS = { &CompleteFont, REGULAR, 0, 0, PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), DEG0, NORMALWRAP,     NONE, 0 };
	CharStat SearchResultsCS1 = { &CompleteFont, REGULAR, 0, 0, PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), DEG0,     NOWRAP,     NONE, 0 };
	CharStat SearchResultsCS2 = { &CompleteFont, REGULAR, 0, 0, PA_RGB(31, 0, 0), PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), DEG0,     NOWRAP,     NONE, 0 };
	CharStat SearchResultsCS3 = { &CompleteFont, REGULAR, 0, 0, PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), DEG0,     NOWRAP, SIMULATE, 0 };

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

	BLOCK CharArea = {{  0, 0},{  0, 0}};

	const u32 ollipolli[] = {0x0398,0x03B8,0x0399,0x03B9,0x039A,0x03BA,0x039B,0x03BB,0x039C,0x03BC};

	string olli = "STRAẞE ABCÄÖ\u1e9eßẞ Ŧ¥ØĦŊJÐ ΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡΣΤΥΦΧΨΩΪΫ";
	string olli2 = lowerPhraseNeu(olli);
	string olli3 = "ABCDEFGHIJKL";
	char olli4[50] = "123456789";
	olli3.replace(4,2,olli4,6);
	iPrint(olli+"\n"+olli2,&ContentWin1,&NormalCS,&CharArea,PA_RGB(0,0,0),UTF8);
// 	while(1);

	ArticleSearchResult* suchergebnis = NULL;
	ArticleSearchResult* redirection  = NULL;

	string markupstr;
	string suchtitel;
	string currentTitle;

	Markup* markup = NULL;

	u8  updateTitle       = 0;
	u8  updateContent     = 0;
	u8  updateStatusbarVS   = 0;
	u8  updatePercent     = 0;
	u8  updateInRealTime  = 1;

	s32 forcedLine        = 0;
	u8  setNewHistoryItem = 1;
	u8  loadArticle       = 1;

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

		if ((Pad.Left.Newpress||Pad.Left.Held))
		{
			if (markup->scrollPageUp())
			{
				h->updateCurrentLine(markup->currentLine());
				updateContent = 1;
				PA_Sleep(10);
			}
		}

		if (Pad.Right.Newpress||Pad.Right.Held)
		{
			if (markup->scrollPageDown())
			{
				h->updateCurrentLine(markup->currentLine());
				updateContent = 1;
				PA_Sleep(10);
			}
		}

		if (Pad.Up.Newpress||Pad.Up.Held)
		{
			if (markup->scrollLineUp())
			{
				h->updateCurrentLine(markup->currentLine());
				updateContent = 1;
				PA_Sleep(10);
			}
		}

		if ((Pad.Down.Newpress||Pad.Down.Held))
		{
			if (markup->scrollLineDown())
			{
				h->updateCurrentLine(markup->currentLine());
				updateContent = 1;
				PA_Sleep(10);
			}
		}

		if (Pad.A.Newpress)
		{
			suchtitel.clear();
			forcedLine = 0;
			setNewHistoryItem = 1;
			loadArticle = 1;
		}

		if (Pad.B.Newpress)
		{
		}

		if (Pad.X.Newpress)
		{
			PA_Clear16bitBg(1);
			PA_Clear16bitBg(0);
			PA_KeyboardIn(24,72);

			DrawBlock(&DnScreen,Btn_ToggleReal,	PA_RGB(24,24,24),0);

			DrawBlock(&DnScreen,Btn_Cancel,		PA_RGB(31, 0, 0),1);
			DrawBlock(&DnScreen,Btn_OK,			PA_RGB( 0,31, 0),1);

			DrawBlock(&DnScreen,Btn_CursorLeft,	PA_RGB(24,24,24),1);
			DrawBlock(&DnScreen,Btn_CursorRight,PA_RGB(24,24,24),1);
			DrawBlock(&DnScreen,Btn_Clear,		PA_RGB(31,15,15),1);

			DrawBlock(&DnScreen,Btn_PageUp,		PA_RGB(24,24,24),1);
			DrawBlock(&DnScreen,Btn_LineUp,		PA_RGB(24,24,24),1);
			DrawBlock(&DnScreen,Btn_LineDown,	PA_RGB(24,24,24),1);
			DrawBlock(&DnScreen,Btn_PageDown,	PA_RGB(24,24,24),1);

			FillVS(&StatusbarVS, PA_RGB(18,22,28));
			CharArea = (BLOCK) {{5,2},{0,0}};
			iPrint(currentTitle,&StatusbarVS,&TitlebarCS,&CharArea,-1,UTF8);

			char letter = 0;
			u8 updateSearchbar   = 1;
			u8 updateSuggestions = 1;
			u8 searchSuggestions = 1;
			u8 updateCursor      = 1;
			s16 cursorPosition = suchtitel.length();
			s32 countdown = 0;

			if (updateInRealTime)
			{
				CharArea = (BLOCK) {{5,2},{0,0}};
				iPrint("✓",&Scr_ToogleReal,&ContentCS,&CharArea,-1,UTF8);
			}
			else
			{
				DrawBlock(&DnScreen,Btn_Reload,	PA_RGB(15,15,31),1);
			}

			while(1)
			{
				letter = PA_CheckKeyboard();
				if (letter > 0) PA_OutputText(1,29,23,"%d  ",letter);

				if (letter > 31) { // there is a new letter
					suchtitel.insert(suchtitel.begin()+cursorPosition,letter);
					cursorPosition++;
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
					suchtitel.erase(cursorPosition-1,1); // Erase the last letter
					cursorPosition--;
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

				if ( (letter == '\n') || (Pad.A.Newpress) )
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
						cursorPosition = suchtitel.length();
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
						CharArea = (BLOCK) {{5,2},{0,0}};
						if (updateInRealTime)
						{
							iPrint("✓",&Scr_ToogleReal,&ContentCS,&CharArea,-1,UTF8);
							DrawBlock(&DnScreen,Btn_Reload,	PA_RGB(31,31,31),1);
						}
						else
						{
							iPrint("✓",&Scr_ToogleReal,&TitlebarCS,&CharArea,-1,UTF8);
							DrawBlock(&DnScreen,Btn_Reload,	PA_RGB(15,15,31),1);
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
						if (cursorPosition<suchtitel.length())
						{
							cursorPosition++;
							updateSearchbar = 1;
						}
					}
				}

				if ((Pad.Up.Newpress||Pad.Up.Held))
				{
					if (s->scrollLineUp())
					{
						PA_Sleep(10);
						updateSuggestions = 1;
					}
				}

				if ((Pad.Down.Newpress||Pad.Down.Held))
				{
					if (s->scrollLineDown())
					{
						PA_Sleep(10);
						updateSuggestions = 1;
					}
				}

				if ((Pad.Left.Newpress||Pad.Left.Held))
				{
					if (s->scrollPageUp())
					{
						PA_Sleep(10);
						updateSuggestions = 1;
					}
				}

				if ((Pad.Right.Newpress||Pad.Right.Held))
				{
					if (s->scrollPageDown())
					{
						PA_Sleep(10);
						updateSuggestions = 1;
					}
				}

				if ((Pad.L.Newpress||Pad.L.Held))
				{
					if (s->scrollLongUp())
					{
						updateSuggestions = 1;
					}
				}

				if ((Pad.R.Newpress||Pad.R.Held))
				{
					if (s->scrollLongDown())
					{
						updateSuggestions = 1;
					}
				}


				if (Pad.X.Newpress)
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

				if (updateCursor)
				{
					CharArea = (BLOCK) {{2,5},{0,0}};
					iPrint(suchtitel.substr(0,cursorPosition),&Searchbar,&SearchResultsCS3,&CharArea,-1,UTF8);
					BLOCK temp = {{CharArea.Start.x-1,2},{CharArea.Start.x-1,19}};
					DrawBlock(&Searchbar,temp,PA_RGB(20,20,20),1);
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

			PA_KeyboardOut();
			PA_Clear16bitBg(1);
			PA_Clear16bitBg(0);

			updateTitle = 1;
			updateContent = 1;
			updateStatusbarVS = 1;
		}

		if (Pad.Y.Newpress)
		{
		}

		if (Pad.L.Newpress)
		{
			if (h->back())
			{
				suchtitel = h->currentTitle();
				forcedLine = h->currentLine();
				setNewHistoryItem = 0;
				loadArticle = 1;
			}
		}

		if (Pad.R.Newpress)
		{
			if (h->forward())
			{
				suchtitel = h->currentTitle();
				forcedLine = h->currentLine();
				setNewHistoryItem = 0;
				loadArticle = 1;
			}
		}

		if (Pad.Start.Newpress)
		{
		}

		if (Pad.Select.Newpress)
		{
			int currentSelectedWikiBackup = currentSelectedWiki;
			if (possibleWikis.size()>1)
			{
				PA_Clear16bitBg(1);
				PA_OutputText(1,1,0,"Choose Wiki\n-----------");
				int i;
				u8 updateSelectedWiki = 1;
				while(1)
				{
					if (Pad.A.Newpress)
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
					if (Pad.B.Newpress)
					{
						currentSelectedWiki = currentSelectedWikiBackup;
						break;
					}
					if (Pad.Up.Newpress||Pad.Down.Newpress)
					{
						currentSelectedWiki += Pad.Down.Newpress-Pad.Up.Newpress;
						if (currentSelectedWiki<0) currentSelectedWiki = 0;
						if (currentSelectedWiki>possibleWikis.size()-1) currentSelectedWiki = possibleWikis.size()-1;
						updateSelectedWiki = 1;
						PA_Sleep(10);
					}
					if (updateSelectedWiki)
					{
						for (i=0;i<possibleWikis.size();i++) {
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
				suchergebnis = t->findArticle(suchtitel);
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
					c->insert(suchergebnis->TitleInArchive(),markupstr);
				}

				string redirectMessage = "";
				u8 numberOfRedirections = 0;
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
						c->insert(suchergebnis->TitleInArchive(),markupstr);
					}
				}
				currentTitle = suchergebnis->TitleInArchive();
				markupstr = redirectMessage + markupstr;

				g->getStatusbar()->display("Formatiere Markup...");
				delete markup;
				markup = new Markup(markupstr, &ContentCS, t);
				g->setMarkup(markup);
				markup->setGlobals(g);

				g->getStatusbar()->displayClearAfter("Formatierung abgeschlossen",30);

				markupstr.clear();

				markup->setCurrentLine(forcedLine);

				if (setNewHistoryItem)
				{
					h->insert(suchergebnis->TitleInArchive(),0);
				}

				updateTitle = 1;
				updateContent = 1;
			}
			else
			{
				g->getStatusbar()->displayErrorClearAfter("\""+suchtitel+"\" nicht gefunden...",60);
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
