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
CharStat SearchResultsCS1;
CharStat SearchResultsCS2;

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
	PA_UpdateUserInfo();
	KT_Init();
	KT_UseEFS();

	PA_Init16bitBg(0, 3);
	PA_Init16bitBg(1, 3);

	PA_SetBrightness(0,-31);
	PA_SetBrightness(1,-31);

	Globals* g = new Globals();
	g->setLanguage(PA_UserInfo.Language);

	if (!PA_InitFat())
	{
		PA_OutputText(1,24,0,"Failed initializing FAT");
		return 1;
	}

	if(!EFS_Init(EFS_ONLY | EFS_DEFAULT_DEVICE, NULL))
	{
		PA_OutputText(1,24,0,"Failed initializing EFS");
		return 1;
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

	for (int i=0;i<120;i++)
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

	// check important things
	PA_OutputText(1,0,2,"Checking \"/dswiki/\"...");
	DIR_ITER* dswikiDir = diropen ("fat:/dswiki/");
	if (dswikiDir == NULL)
	{
		PA_OutputText(1,24,2,"%c1[Failed]");
		return 1;
	}
	else
	{
		PA_OutputText(1,28,2,"%c2[OK]");
		dirclose(dswikiDir);
	}
	PA_OutputText(1,0,3,"Checking EFS-fonts...");
	DIR_ITER* dswikiFontDir = diropen ("efs:/dswiki/fonts/");
	if (dswikiFontDir == NULL)
	{
		PA_OutputText(1,24,3,"%c1[Failed]");
		return 1;
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
		return 1;
	}
	PA_OutputText(1,0,5,"Gathering installed wikis...");
	Dumps* d = new Dumps();
	vector<string> possibleWikis = d->getPossibleWikis();
	if (possibleWikis.size()==0)
	{
		PA_OutputText(1,26,5,"%c1[None]");
		return 1;
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

	enum {
		SPRITE_HISTORY, SPRITE_HISTORYX, SPRITE_RELOAD, SPRITE_CANCEL, SPRITE_OK, SPRITE_2UPARROW, SPRITE_1UPARROW, SPRITE_1DOWNARROW, SPRITE_2DOWNARROW, SPRITE_1LEFTARROW, SPRITE_1RIGHTARROW, SPRITE_CLEARLEFT, SPRITE_CONFIGURE,  SPRITE_BOOKMARKADD, SPRITE_BOOKMARK, SPRITE_VIEWMAG
	};

	// important variables
	KT_CreateSprite(0, SPRITE_HISTORY,     "dswiki/icons/history",       OBJ_SIZE_16X16, 1, 0, 1, -16, -16);
	KT_CreateSprite(0, SPRITE_HISTORYX,    "dswiki/icons/history_clear", OBJ_SIZE_16X16, 1, 0, 0, -16, -16);
	KT_CreateSprite(0, SPRITE_RELOAD,      "dswiki/icons/reload",        OBJ_SIZE_16X16, 1, 0, 0, -16, -16);
	KT_CreateSprite(0, SPRITE_CANCEL,      "dswiki/icons/cancel",        OBJ_SIZE_16X16, 1, 0, 0, -16, -16);
	KT_CreateSprite(0, SPRITE_OK,          "dswiki/icons/ok",            OBJ_SIZE_16X16, 1, 0, 0, -16, -16);
	KT_CreateSprite(0, SPRITE_2UPARROW,    "dswiki/icons/2uparrow",      OBJ_SIZE_16X16, 1, 0, 0, -16, -16);
	KT_CreateSprite(0, SPRITE_1UPARROW,    "dswiki/icons/1uparrow",      OBJ_SIZE_16X16, 1, 0, 0, -16, -16);
	KT_CreateSprite(0, SPRITE_1DOWNARROW,  "dswiki/icons/1downarrow",    OBJ_SIZE_16X16, 1, 0, 0, -16, -16);
	KT_CreateSprite(0, SPRITE_2DOWNARROW,  "dswiki/icons/2downarrow",    OBJ_SIZE_16X16, 1, 0, 0, -16, -16);
	KT_CreateSprite(0, SPRITE_1LEFTARROW,  "dswiki/icons/1leftarrow",    OBJ_SIZE_16X16, 1, 0, 0, -16, -16);
	KT_CreateSprite(0, SPRITE_1RIGHTARROW, "dswiki/icons/1rightarrow",   OBJ_SIZE_16X16, 1, 0, 0, -16, -16);
	KT_CreateSprite(0, SPRITE_CLEARLEFT,   "dswiki/icons/clear_left",    OBJ_SIZE_16X16, 1, 0, 0, -16, -16);
	KT_CreateSprite(0, SPRITE_CONFIGURE,   "dswiki/icons/configure",     OBJ_SIZE_16X16, 1, 0, 0, -16, -16);
	KT_CreateSprite(0, SPRITE_BOOKMARKADD, "dswiki/icons/bookmark_add",  OBJ_SIZE_16X16, 1, 0, 0, -16, -16);
	KT_CreateSprite(0, SPRITE_BOOKMARK,    "dswiki/icons/bookmark",      OBJ_SIZE_16X16, 1, 0, 0, -16, -16);
	KT_CreateSprite(0, SPRITE_VIEWMAG,     "dswiki/icons/viewmag",       OBJ_SIZE_16X16, 1, 0, 0, -16, -16);

	Statusbar* sb = new Statusbar();
	PercentIndicator* p = new PercentIndicator();
	g->setDumps(d);
	g->setFont(CompleteFont);
	g->setStatusbar(sb);
	g->setPercentIndicator(p);

	// Initialization of global variables
	UpScreen         = (Device)   { "U", 1, (unsigned short int*)PA_DrawBg[1], 256, 192};
	DnScreen         = (Device)   { "D", 0, (unsigned short int*)PA_DrawBg[0], 256, 192};
	NormalCS         = (CharStat) { CompleteFont, REGULAR, 0, 0, PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), DEG0,   HARDWRAP, NONE, 0 };
	ContentCS        = (CharStat) { CompleteFont, REGULAR, 0, 0, PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), DEG0, NORMALWRAP, NONE, 0 };
	ErrorCS          = (CharStat) { CompleteFont, REGULAR, 0, 0, PA_RGB(27, 0, 0), PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), DEG0, NORMALWRAP, NONE, 0 };
	StatusbarCS      = (CharStat) { CompleteFont, REGULAR, 1, 1, PA_RGB( 5, 5, 5), PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), DEG0,   HARDWRAP, NONE, 0 };
	StatErrorCS      = (CharStat) { CompleteFont, REGULAR, 1, 1, PA_RGB(27, 4, 4), PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), DEG0,   HARDWRAP, NONE, 0 };
	SearchResultsCS1 = (CharStat) { CompleteFont, REGULAR, 0, 0, PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), DEG0,     NOWRAP, NONE, 0 };
	SearchResultsCS2 = (CharStat) { CompleteFont, REGULAR, 0, 0, PA_RGB(31, 0, 0), PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), DEG0,     NOWRAP, NONE, 0 };
	PercentArea = (VirScreen) { 229, 176,  27,  16, {{0,0},{0,0}}, &DnScreen}; InitVS(&PercentArea);
	Titlebar    = (VirScreen) {   0,   0, 256,  16, {{0,0},{0,0}}, &UpScreen}; InitVS(&Titlebar);
	ContentWin1 = (VirScreen) {   2,  18, 252, 172, {{0,0},{0,0}}, &UpScreen}; InitVS(&ContentWin1);
	ContentWin2 = (VirScreen) {   2,   2, 252, 172, {{0,0},{0,0}}, &DnScreen}; InitVS(&ContentWin2);
	StatusbarVS = (VirScreen) {   0, 176, 229,  16, {{0,0},{0,0}}, &DnScreen}; InitVS(&StatusbarVS);
	// End of global variables

	VirScreen  Searchbar   = {  47,  37, 162,  22, {{0,0},{0,0}}, &DnScreen}; InitVS(&Searchbar);

	CharStat       TitlebarCS = { CompleteFont, BOLD,    0, 0, PA_RGB(31,31,31), PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), DEG0,   HARDWRAP,     NONE, 0 };
	CharStat SearchResultsCS3 = { CompleteFont, REGULAR, 0, 0, PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), DEG0,     NOWRAP, SIMULATE, 0 };

	BLOCK CharArea = {{ 0, 0},{ 0, 0}};

	// use graphical interface from now on

	ArticleSearchResult* suchergebnis = NULL;
	ArticleSearchResult* redirection  = NULL;

	string markupstr;
	string suchtitel = "";
	string currentTitle;

	Markup* markup = NULL;

	string currentSelectedWiki = "";
	TextBox* WikiChooser = new TextBox(possibleWikis);
	WikiChooser->setTitle("Choose your Wiki");
	WikiChooser->allowCancel(0);

#if !DEBUG
	if ( possibleWikis.size() > 1 )
	{
		currentSelectedWiki = WikiChooser->run();
	}
	else
	{
#endif
		currentSelectedWiki = possibleWikis[0];
#if !DEBUG
	}
#endif

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
	g->getStatusbar()->displayClearAfter("Loading "+currentSelectedWiki+"...",60);

	TitleIndex* t = new TitleIndex();
	g->setTitleIndex(t);
	t->setGlobals(g);
	t->load(currentSelectedWiki);


	g->getStatusbar()->display("Initializing MarkupGetter...");
	WikiMarkupGetter* wmg = new WikiMarkupGetter();
	g->setWikiMarkupGetter(wmg);
	wmg->setGlobals(g);
	wmg->load(currentSelectedWiki);
	g->getStatusbar()->clearAfter(30);

	History* h = new History();
	Cache*   c = new Cache();
	Search*  s = new Search();

	g->setSearch(s);
	s->setGlobals(g);

	PA_SetSpriteXY(0, SPRITE_CONFIGURE,  0, 176);
	PA_SetSpriteXY(0, SPRITE_BOOKMARKADD, 32, 176);
	PA_SetSpriteXY(0, SPRITE_BOOKMARK, 64, 176);
	PA_SetSpriteXY(0, SPRITE_VIEWMAG, 96, 176);

	while(1) // main loop
	{
		PA_CheckLid();

		if (Stylus.Held)
		{
			if (Stylus.Newpress)
			{
				if (PA_SpriteTouched(SPRITE_CONFIGURE))
				{
					g->setOptions();
					updateContent = 1;
				};

				if (PA_SpriteTouched(SPRITE_VIEWMAG))
				{
					Pad.Newpress.X = 1; // TODO: This is sooo ugly!!!
				};

				if (PA_SpriteTouched(SPRITE_BOOKMARK))
				{
					g->getStatusbar()->displayClearAfter("Loading Bookmarks",45);
					string bookmark = g->loadBookmark();
					updateContent = 1;
					if (!bookmark.empty())
					{
						suchtitel = bookmark;
						forcedLine = 0;
						setNewHistoryItem = 1;
						loadArticle = 1;
					}
				}

				if (PA_SpriteTouched(SPRITE_BOOKMARKADD))
				{
					g->getStatusbar()->displayClearAfter("Adding Bookmark",45);
					if (!currentTitle.empty())
					{
						g->saveBookmark(currentTitle);
					}
				}

				string markupClick = markup->evaluateClick(Stylus.X,Stylus.Y);
				if (!markupClick.empty())
				{
					suchtitel = markupClick;
					forcedLine = 0;
					setNewHistoryItem = 1;
					loadArticle = 1;
				}
			}
			else
			{
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

			PA_SetSpriteXY(0, SPRITE_HISTORYX,      3,  3);
			PA_SetSpriteXY(0, SPRITE_CANCEL,       67,  9);
			PA_SetSpriteXY(0, SPRITE_OK,          167,  9);
			PA_SetSpriteXY(0, SPRITE_2UPARROW,    234, 72);
			PA_SetSpriteXY(0, SPRITE_1UPARROW,    234, 97);
			PA_SetSpriteXY(0, SPRITE_1DOWNARROW,  234,122);
			PA_SetSpriteXY(0, SPRITE_2DOWNARROW,  234,147);
			PA_SetSpriteXY(0, SPRITE_1LEFTARROW,   31, 39);
			PA_SetSpriteXY(0, SPRITE_1RIGHTARROW, 209, 39);
			PA_SetSpriteXY(0, SPRITE_CLEARLEFT,   234, 40);


			if (!updateInRealTime)
			{
				PA_SetSpriteXY(0,SPRITE_HISTORYX,3,3);
				PA_SetSpriteXY(0,SPRITE_RELOAD,117,9);
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
				if (Stylus.Held)
				{
					if (Stylus.Newpress)
					{
						POINT S = {Stylus.X,Stylus.Y};
						if (PA_SpriteTouched(SPRITE_CLEARLEFT) && (!suchtitel.empty()) )
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
						else if (PA_SpriteTouched(SPRITE_OK))
						{
							suchtitel = s->currentHighlightedItem();
							forcedLine = 0;
							setNewHistoryItem = 1;
							loadArticle = 1;
							break;
						}
						else if (IsInArea(StatusbarVS.AbsoluteBound,S))
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
						else if (PA_SpriteTouched(SPRITE_CANCEL))
						{
							break;
						}
						else if (PA_SpriteTouched(SPRITE_2UPARROW))
						{
							if (s->scrollPageUp())
							{
								PA_Sleep(10);
								updateSuggestions = 1;
							}
						}
						else if (PA_SpriteTouched(SPRITE_1UPARROW))
						{
							if (s->scrollLineUp())
							{
								PA_Sleep(10);
								updateSuggestions = 1;
							}
						}
						else if (PA_SpriteTouched(SPRITE_1DOWNARROW))
						{
							if (s->scrollLineDown())
							{
								PA_Sleep(10);
								updateSuggestions = 1;
							}
						}
						else if (PA_SpriteTouched(SPRITE_2DOWNARROW))
						{
							if (s->scrollPageDown())
							{
								PA_Sleep(10);
								updateSuggestions = 1;
							}
						}
						else if (PA_SpriteTouched(SPRITE_HISTORY) || PA_SpriteTouched(SPRITE_HISTORYX))
						{
							updateInRealTime = 1 - updateInRealTime;
							if (updateInRealTime)
							{
								PA_SetSpriteXY(0,SPRITE_HISTORY,-16,-16);
								PA_SetSpriteXY(0,SPRITE_RELOAD,-16,-16);
							}
							else
							{
								PA_SetSpriteXY(0,SPRITE_HISTORY,3,3);
								PA_SetSpriteXY(0,SPRITE_RELOAD,117,9);
							}
						}
						else if (PA_SpriteTouched(SPRITE_RELOAD))
						{
							searchSuggestions = 1;
							countdown = 0;
						}
						else if (PA_SpriteTouched(SPRITE_1LEFTARROW))
						{
							if (cursorPosition>0)
							{
								cursorPosition--;
								updateSearchbar = 1;
							}
						}
						else if (PA_SpriteTouched(SPRITE_1RIGHTARROW))
						{
							if (offsetsUTF[cursorPosition]<suchtitel.length())
							{
								cursorPosition++;
								updateSearchbar = 1;
							}
						}
					}
					else
					{
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
			for (int i=SPRITE_HISTORY;i<=SPRITE_CLEARLEFT;i++)
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
			g->setOptions();
			updateContent = 1;
		}

		if (Pad.Newpress.Select && (possibleWikis.size()>1))
		{
			WikiChooser->allowCancel(1);
			string currentSelectedWikiBackup = currentSelectedWiki;
			currentSelectedWiki = WikiChooser->run();
			if (!currentSelectedWiki.empty() && (currentSelectedWiki != currentSelectedWikiBackup))
			{
				g->getStatusbar()->displayClearAfter("Loading "+currentSelectedWiki+"...",60);
				delete t;
				t = new TitleIndex();
				g->setTitleIndex(t);
				t->setGlobals(g);
				t->load(currentSelectedWiki);

				delete wmg;
				wmg = new WikiMarkupGetter();
				g->setWikiMarkupGetter(wmg);
				wmg->setGlobals(g);
				wmg->load(currentSelectedWiki);

				h->clear();
				c->clear();

				loadArticle = 1;
			}
			else
			{
				currentSelectedWiki = currentSelectedWikiBackup;
			}
			updateTitle = 1;
			updateContent = 1;
		}

		if (loadArticle)
		{
			if (suchtitel.empty())
			{
				g->getStatusbar()->display("Searching random article...");
				suchergebnis = t->getRandomArticle();
			}
			else
			{
				g->getStatusbar()->display("Searching "+suchtitel+"...");
				suchergebnis = t->findArticle(suchtitel,currentTitle);
			}

			if (suchergebnis!=NULL)
			{
				suchtitel.clear();
				g->getStatusbar()->display("Loading \""+suchergebnis->TitleInArchive()+"\"");

				if (c->isInCache(suchergebnis->TitleInArchive()))
				{
					g->getStatusbar()->display("Getting markup from cache...");
					markupstr = c->getMarkup(suchergebnis->TitleInArchive());
				}
				else
				{
					g->getStatusbar()->display("Getting markup from disk...");
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
						g->getStatusbar()->display("Following redirection from cache...");
						markupstr = c->getMarkup(suchergebnis->TitleInArchive());
					}
					else
					{
						g->getStatusbar()->display("Following redirection from disk...");
						markupstr = g->getWikiMarkupGetter()->getMarkup(suchergebnis->TitleInArchive());
// 						c->insert(suchergebnis->TitleInArchive(),markupstr);
					}
				}
				currentTitle = suchergebnis->TitleInArchive();
				markupstr = redirectMessage + markupstr;

				g->getStatusbar()->display("Formatting \""+currentTitle+"\"...");
				delete markup;
				markup = new Markup();
				g->setMarkup(markup);
				markup->setGlobals(g);

				markup->parse(markupstr);
				g->getStatusbar()->displayClearAfter("Formatting complete",30);

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
				g->getStatusbar()->displayErrorClearAfter("\""+suchtitel+"\" not found...",90);
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

	} // end of main loop

	return 0;
}
