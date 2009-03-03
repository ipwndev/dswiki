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
#include "CachingFont.h"
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
#include "WIKI2XML.h"
#include "WIKI2XML_global.h"

bool debug;

Device   UpScreen;
Device   DnScreen;
CharStat NormalCS;

VirScreen Titlebar;
VirScreen ContentWin1;
VirScreen ContentWin2;
VirScreen StatusbarVS;
VirScreen PercentArea;

#define DEBUG 1
#define DEBUG_WIKI_NR 1
#define STRESSTEST 0

int getFreeRAM()
{
	int q = 2*1024*1024;
	int size = q;
	void *ptr;

	do
	{
		ptr = malloc(size);
		if(ptr) free(ptr);
		else size -= q;
		q /= 2;
		size += q;
	} while(q > 0);

	return size;
}

int main(int argc, char ** argv)
{
	// PAlib initialization
	PA_Init();
	PA_InitVBL();
	PA_InitGHPad();
	PA_SetAutoUpdateGHPadTimes(1);
	PA_SetAutoUpdatePadTimes(1);
	PA_UpdateUserInfo();
	KT_Init();
	KT_UseEFS();

	string markupstr;
	markupstr.reserve(1048576); // Reserve 1.0 MiB for the markup, all transformations MUST be made in-place

	string suchtitel = "Chester W. Nimitz";
// 	string suchtitel;

	PA_Init16bitBg(0, 3);
	PA_Init16bitBg(1, 3);
	PA_InitText(1,2);
#if DEBUG
	PA_InitText(0,2);
#endif

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

	PA_Sleep(30);
	debug = (Pad.Held.L || Pad.Held.R);
	if (debug)
	{
		PA_OutputText(1,11,11,"debug-mode");
		PA_OutputText(1,11,12,"  active  ");
		PA_Sleep(60);
		PA_ClearTextBg(1);
	}

#if !DEBUG
	PA_SetBrightness(0,-31);
	PA_SetBrightness(1,-31);
	// intro screens from EFS
	bool breakIntro = false;
	KT_LoadGif(0, "dswiki/splash/dswiki", 0, 0);
	KT_LoadGif(1, "dswiki/splash/neo", 0, 0);
	for (int i=-31;i<=0;i++)
	{
		PA_SetBrightness(0,i);
		PA_SetBrightness(1,i);
		if (Pad.Newpress.Anykey || Stylus.Newpress)
			breakIntro = true;
		PA_WaitForVBL();
		PA_WaitForVBL();
	}
	for (int i=0;i<120;i++)
	{
		if (Pad.Newpress.Anykey || Stylus.Newpress)
			breakIntro = true;
		if ((i>30) && breakIntro)
			break;
		PA_WaitForVBL();
	}
	for (int i=0;i<32;i++)
	{
		PA_SetBrightness(1,i);
		if (Pad.Newpress.Anykey || Stylus.Newpress)
			breakIntro = true;
		PA_WaitForVBL();
	}
	KT_LoadGif(1, "dswiki/splash/splash1", 0, 0);
	for (int i=31;i>=0;i--)
	{
		PA_SetBrightness(1,i);
		if (Pad.Newpress.Anykey || Stylus.Newpress)
			breakIntro = true;
		PA_WaitForVBL();
	}
	for (int i=0;i<90;i++)
	{
		if (Pad.Newpress.Anykey || Stylus.Newpress)
			breakIntro = true;
		if ((i>15) && breakIntro)
			break;
		PA_WaitForVBL();
	}
	for (int i=0;i<32;i++)
	{
		PA_SetBrightness(1,i);
		if (Pad.Newpress.Anykey || Stylus.Newpress)
			breakIntro = true;
		PA_WaitForVBL();
	}
	KT_LoadGif(1, "dswiki/splash/splash2_l",   0, 0);
	KT_LoadGif(1, "dswiki/splash/splash2_r", 128, 0);
	for (int i=31;i>=0;i--)
	{
		PA_SetBrightness(1,i);
		if (Pad.Newpress.Anykey || Stylus.Newpress)
			breakIntro = true;
		PA_WaitForVBL();
	}
	for (int i=0;i<90;i++)
	{
		if (Pad.Newpress.Anykey || Stylus.Newpress)
			breakIntro = true;
		if ((i>15) && breakIntro)
			break;
		PA_WaitForVBL();
	}
	for (int i=0;i<32;i++)
	{
		PA_SetBrightness(0,i);
		PA_SetBrightness(1,i);
		if (Pad.Newpress.Anykey || Stylus.Newpress)
			breakIntro = true;
		PA_WaitForVBL();
		PA_WaitForVBL();
	}
#endif

	// initializing things while not visible
	PA_SetBgPalCol(0, 0, PA_RGB(31,31,31));
	PA_SetBgPalCol(1, 0, PA_RGB(31,31,31));

	PA_Clear16bitBg(1);
	PA_Clear16bitBg(0);

	PA_SetTextCol (0, 0, 0, 0);
	PA_SetTextCol (1, 0, 0, 0);
#if !DEBUG
	PA_InitKeyboard(2);
	PA_KeyboardOut();

	// visible again
	PA_SetBrightness(0,0);
	PA_SetBrightness(1,0);
#endif


	// check important things
	DIR_ITER* dswikiDir = diropen ("fat:/dswiki/");
	if (dswikiDir)
	{
		dirclose(dswikiDir);
	}
	else
	{
		PA_OutputText(1,0,0,"Checking \"/dswiki/\" %c1failed%c0!");
		return 1;
	}

	DIR_ITER* dswikiFontDir = diropen ("efs:/dswiki/fonts/");
	if (dswikiFontDir)
	{
		dirclose(dswikiFontDir);
	}
	else
	{
		PA_OutputText(1,0,0,"Checking EFS font dir %c1failed%c0!");
		return 1;
	}

	Font* frankenstein_r  = new Font("efs:/dswiki/fonts/font_r.dat");
	Font* frankenstein_b  = new Font("efs:/dswiki/fonts/font_b.dat");
	Font* frankenstein_o  = new Font("efs:/dswiki/fonts/font_o.dat");
	Font* frankenstein_bo = new Font("efs:/dswiki/fonts/font_bo.dat");
	if (!frankenstein_r->initOK() || !frankenstein_b->initOK() || !frankenstein_o->initOK() || !frankenstein_bo->initOK())
	{
		PA_OutputText(1,0,0,"Initializing fonts %c1failed%c0!");
		return 1;
	}
	g->setFont(frankenstein_r, FONT_R);
	g->setFont(frankenstein_b, FONT_B);
	g->setFont(frankenstein_o, FONT_O);
	g->setFont(frankenstein_bo, FONT_BO);

	Dumps* d = new Dumps();
	g->setDumps(d);
	vector<string> possibleWikis = d->getPossibleWikis();
	if (possibleWikis.size()==0)
	{
		PA_OutputText(1,0,0,"%c1No dumps %c0were found!");
		PA_Sleep(120);
		PA_ClearTextBg(1);
	}

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
	g->setStatusbar(sb);

	PercentIndicator* p = new PercentIndicator();
	g->setPercentIndicator(p);

	// Initialization of global variables
	UpScreen = (Device)   { "U", 1, (unsigned short int*)PA_DrawBg[1], 256, 192};
	DnScreen = (Device)   { "D", 0, (unsigned short int*)PA_DrawBg[0], 256, 192};
	NormalCS = (CharStat) { g->getFont(FONT_R), 0, 0, g->textColor(), PA_RGB( 0, 0, 0), g->backgroundColor(), DEG0, NORMALWRAP, NONE, 0 };

	Titlebar    = (VirScreen) {   0,   0, 256,  16, {{0,0},{0,0}}, &UpScreen}; InitVS(&Titlebar);
	ContentWin1 = (VirScreen) {   2,  18, 252, 172, {{0,0},{0,0}}, &UpScreen}; InitVS(&ContentWin1);
	ContentWin2 = (VirScreen) {   2,   2, 252, 172, {{0,0},{0,0}}, &DnScreen}; InitVS(&ContentWin2);
	StatusbarVS = (VirScreen) {   0, 176, 256,  16, {{0,0},{0,0}}, &DnScreen}; InitVS(&StatusbarVS);
	PercentArea = (VirScreen) { 229, 176,  27,  16, {{0,0},{0,0}}, &DnScreen}; InitVS(&PercentArea);
	// End of global variables

	VirScreen  Searchbar   =  {  47,  37, 162,  22, {{0,0},{0,0}}, &DnScreen}; InitVS(&Searchbar);

	BLOCK CharArea = {{ 0, 0},{ 0, 0}};

	// use graphical interface from now on

	ArticleSearchResult* suchergebnis = NULL;
	ArticleSearchResult* redirection  = NULL;
	Markup* markup = NULL;

	string currentTitle = "";
	int currentSelectedWiki;

	TextBox* WikiChooser = new TextBox(possibleWikis);
	WikiChooser->setTitle("Choose your Wiki");
	WikiChooser->allowCancel(0);

	bool loadInternalWiki = false;

#if !DEBUG
	if ( possibleWikis.size() > 1 )
	{
		currentSelectedWiki = WikiChooser->run();
	}
	else if ( possibleWikis.size() == 1 )
	{
		currentSelectedWiki = 0;
	}
	else
	{
		loadInternalWiki = true;
		possibleWikis = d->getPossibleWikis(true);
		vector<string>::iterator pos_it = find(possibleWikis.begin(), possibleWikis.end(), "manual");
		if (pos_it != possibleWikis.end())
		{
			currentSelectedWiki = pos_it - possibleWikis.begin();
			suchtitel = "Test";
		}
		else
		{
			PA_OutputText(1,0,0,"%c1No internal dumps %c0present!");
			return 1;
		}
	}
#else
	currentSelectedWiki = DEBUG_WIKI_NR;
#endif

	bool updateTitle       = false;
	bool updateContent     = false;
	bool updateStatusbarVS = false;
	bool updatePercent     = false;
	bool updateInRealTime  = true;
	bool search            = false;
	bool setNewHistoryItem = true;
	bool loadArticle       = true;
	int forcedLine         = 0;

	FillVS(&Titlebar, PA_RGB( 9,16,28));

	g->getStatusbar()->clear();
	g->getPercentIndicator()->clear();
	g->getStatusbar()->displayClearAfter("Loading "+possibleWikis[currentSelectedWiki]+"...",60);

	TitleIndex* t = new TitleIndex();
	g->setTitleIndex(t);
	t->setGlobals(g);
	t->load(possibleWikis[currentSelectedWiki],loadInternalWiki);

	g->getStatusbar()->display("Initializing MarkupGetter...");
	WikiMarkupGetter* wmg = new WikiMarkupGetter();
	g->setWikiMarkupGetter(wmg);
	wmg->setGlobals(g);
	wmg->load(possibleWikis[currentSelectedWiki],loadInternalWiki);

	g->getStatusbar()->clearAfter(30);

	History* h = new History();

	Search*  s = new Search();
	s->setGlobals(g);
	g->setSearch(s);

	PA_SetSpriteXY(0, SPRITE_CONFIGURE, 0, 176);
// 	PA_SetSpriteXY(0, SPRITE_BOOKMARKADD, 32, 176);
	PA_SetSpriteXY(0, SPRITE_BOOKMARK, 64, 176);
	PA_SetSpriteXY(0, SPRITE_VIEWMAG, 96, 176);

	while(1) // main loop
	{
		if (Stylus.Newpress)
		{
			if (PA_SpriteTouched(SPRITE_CONFIGURE))
			{
				g->setOptions();
				updateContent = true;
			}
			else if (PA_SpriteTouched(SPRITE_VIEWMAG))
			{
				search = true;
			}
			else if (PA_SpriteTouched(SPRITE_BOOKMARK))
			{
				g->getStatusbar()->displayClearAfter("Loading Bookmarks",45);
				string bookmark = g->loadBookmark();
				updateContent = true;
				if (!bookmark.empty())
				{
					suchtitel = bookmark;
					forcedLine = 0;
					setNewHistoryItem = true;
					loadArticle = true;
				}
			}
// 			else if (PA_SpriteTouched(SPRITE_BOOKMARKADD))
// 			{
// 				g->getStatusbar()->displayClearAfter("Adding Bookmark",45);
// 				if (!currentTitle.empty())
// 				{
// 					g->saveBookmark(currentTitle);
// 				}
// 			}
		}
		else if (Stylus.Held)
		{
		}
		else if (Stylus.Released)
		{
		}
		else
		{
		}

// 		if (Stylus.Newpress)
// 		{
// 			string markupClick = markup->evaluateClick(Stylus.X,Stylus.Y);
// 			if (!markupClick.empty())
// 			{
// 				suchtitel = markupClick;
// 				forcedLine = 0;
// 				setNewHistoryItem = true;
// 				loadArticle = true;
// 			}
// 		}

		if (Pad.Newpress.Left || Pad.Held.Left || GHPad.Newpress.Blue || GHPad.Held.Blue)
		{
// 			if (markup->scrollPageUp())
// 			{
// 				h->updateCurrentLine(markup->currentLine());
// 				updateContent = true;
// 				PA_Sleep(10);
// 			}
		}

		if (Pad.Newpress.Right || Pad.Held.Right || GHPad.Newpress.Green || GHPad.Held.Green)
		{
// 			if (markup->scrollPageDown())
// 			{
// 				h->updateCurrentLine(markup->currentLine());
// 				updateContent = true;
// 				PA_Sleep(10);
// 			}
		}

		if (Pad.Newpress.Up||Pad.Held.Up)
		{
// 			if (markup->scrollLineUp())
// 			{
// 				h->updateCurrentLine(markup->currentLine());
// 				updateContent = true;
// 				PA_Sleep(10);
// 			}
		}

		if ((Pad.Newpress.Down||Pad.Held.Down))
		{
// 			if (markup->scrollLineDown())
// 			{
// 				h->updateCurrentLine(markup->currentLine());
// 				updateContent = true;
// 				PA_Sleep(10);
// 			}
		}

		if (Pad.Newpress.A)
		{
			suchtitel.clear();
			forcedLine = 0;
			setNewHistoryItem = true;
			loadArticle = true;
		}

		if ( Pad.Released.B && (Pad.Downtime.B<60) ) // TODO in the morning
		{
			if (markup)
			{
// 				markup->unselect();
			}
		}
		else if ( Pad.Held.B && ( Pad.Downtime.B >= 60 ) )
		{
			// TODO switch scrolling type
		}

		if (Pad.Newpress.X)
		{
			search = true;
		}

		if (Pad.Newpress.Y)
		{
			if (markup)
			{
				if (markup->toggleIndex())
				{
					updateContent = true;
				}
			}
		}

		if ( (Pad.Released.R) && (Pad.Downtime.R<60) )
		{
			if (h->forward())
			{
				suchtitel = h->currentTitle();
				forcedLine = h->currentLine();
				setNewHistoryItem = false;
				loadArticle = true;
			}
		}
		else if ( (Pad.Released.L) && (Pad.Downtime.L<60) )
		{
			if (h->back())
			{
				suchtitel = h->currentTitle();
				forcedLine = h->currentLine();
				setNewHistoryItem = false;
				loadArticle = true;
			}
		}
		else if ( ( h->size() > 1 ) && ( ( (Pad.Held.R) && (Pad.Downtime.R>=60) ) || ( (Pad.Held.L) && (Pad.Downtime.L>=60) ) ) )
		{
			vector<string> history_vec = h->get();
			TextBox histChoice(history_vec);
			histChoice.setGlobals(g);
			histChoice.setTitle("History of pages");
			histChoice.allowCancel(1);
			histChoice.setCurrentPosition(h->getCurrentPosition());

			int chosenHistItem = histChoice.run();

			if ( (chosenHistItem>=0) && (chosenHistItem != h->getCurrentPosition()) )
			{
				h->setCurrentPosition(chosenHistItem);
				setNewHistoryItem = false;
				suchtitel = history_vec[chosenHistItem];
				loadArticle = true;
			}
			updateContent = true;
		}

		if (Pad.Newpress.Start)
		{
			g->setOptions();
			updateContent = true;
		}

		if (Pad.Newpress.Select && (possibleWikis.size()>1))
		{
			int currentSelectedWikiBackup = currentSelectedWiki;
			WikiChooser->allowCancel(1);
			WikiChooser->setCurrentPosition(currentSelectedWiki);
			currentSelectedWiki = WikiChooser->run();
			if ((currentSelectedWiki>=0) && (currentSelectedWiki != currentSelectedWikiBackup))
			{
				g->getStatusbar()->displayClearAfter("Loading "+possibleWikis[currentSelectedWiki] + "...",60);
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

				loadArticle = true;
			}
			else
			{
				currentSelectedWiki = currentSelectedWikiBackup;
			}
			updateTitle = true;
			updateContent = true;
		}

#if STRESSTEST
		loadArticle = true;
#endif

		if (loadArticle)
		{
			delete suchergebnis;
			suchergebnis = NULL;

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

			if (suchergebnis)
			{
				if (markup)
				{
					delete markup;
					markup = NULL;
				}

				suchtitel.clear();
				g->getStatusbar()->display("Loading \""+suchergebnis->TitleInArchive()+"\"");

				markupstr.clear();
				g->getStatusbar()->display("Getting markup...");
				g->getWikiMarkupGetter()->getMarkup(markupstr, suchergebnis->TitleInArchive());

				string redirectMessage = "";
				unsigned char numberOfRedirections = 0;
				while ((numberOfRedirections<MAX_NUMBER_OF_REDIRECTIONS) && (redirection = t->isRedirect(markupstr)))
				{
					numberOfRedirections++;
					ArticleSearchResult* temp = suchergebnis;
					redirectMessage += "(\u2192 "+temp->TitleInArchive()+")\n";
					suchergebnis = redirection;

					markupstr.clear();
					g->getStatusbar()->display("Following redirection...");
					g->getWikiMarkupGetter()->getMarkup(markupstr, suchergebnis->TitleInArchive());
				}

				currentTitle = suchergebnis->TitleInArchive();
				markupstr.insert(0,redirectMessage);

				FillVS(&Titlebar, PA_RGB( 16,16,16));
				CharArea = (BLOCK) {{5,2},{0,0}};
				CharStat TitlebarCS = NormalCS;
				TitlebarCS.Color = PA_RGB(31,31,31);
				TitlebarCS.Wrap = NOWRAP;

				iPrint(currentTitle,&Titlebar,&TitlebarCS,&CharArea);
				g->getStatusbar()->display("Formatting \""+currentTitle+"\"...");

				markup = new Markup();
				g->setMarkup(markup);
				markup->setGlobals(g);
				markup->parse(markupstr);

				if (markup->LoadOK())
				{
// 					PA_OutputText(1,0,2,"%c2XML-Parsing OK    ");
					PA_OutputText(1,0,2,"%c2                  ");
				}
				else
				{
					PA_OutputText(1,0,2,"%c1XML-Parsing failed");
				}

				g->getStatusbar()->displayClearAfter("Formatting complete",30);

// 				markup->setCurrentLine(forcedLine);

				if (setNewHistoryItem)
				{
					h->insert(currentTitle,0);
				}

				updateTitle = true;
				updateContent = true;
			}
			else
			{
				g->getStatusbar()->displayErrorClearAfter("\""+suchtitel+"\" not found...",90);
				updatePercent = true;
			}

			loadArticle = false;
		}

		if (updateTitle)
		{
			FillVS(&Titlebar, PA_RGB( 9,16,28));
			CharArea = (BLOCK) {{5,2},{0,0}};
			CharStat TitlebarCS = NormalCS;
			TitlebarCS.Color = PA_RGB(31,31,31);
			iPrint(currentTitle,&Titlebar,&TitlebarCS,&CharArea);
			updateTitle = false;
		}

		if (updateStatusbarVS)
		{
			g->getStatusbar()->clear();
			updatePercent = true;
			updateStatusbarVS = false;
		}

		if (updateContent)
		{
			if (markup->LoadOK())
			{
				g->getMarkup()->draw();
			}
			updatePercent = true;
			updateContent = false;
		}

		if (updatePercent)
		{
// 			g->getPercentIndicator()->update(markup->currentPercent());
			g->getPercentIndicator()->redraw();
			updatePercent = false;
		}

		if (search)
		{
			search = false;
			PA_Clear16bitBg(1);
			PA_Clear16bitBg(0);

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
			PA_SetSpriteXY(0, SPRITE_CONFIGURE,   -16,-16);
// 			PA_SetSpriteXY(0, SPRITE_BOOKMARKADD, -16,-16);
			PA_SetSpriteXY(0, SPRITE_BOOKMARK,    -16,-16);
			PA_SetSpriteXY(0, SPRITE_VIEWMAG,     -16,-16);
			PA_ScrollKeyboardXY(24,72);

			FillVS(&StatusbarVS, PA_RGB(18,22,28));
			CharArea = (BLOCK) {{5,2},{0,0}};
			iPrint(currentTitle,&StatusbarVS,&NormalCS,&CharArea);

			char letter = 0;
			bool updateSearchbar   = true;
			bool updateSuggestions = true;
			bool searchSuggestions = true;
			bool updateCursor      = true;
			int cursorPosition = 0;
			unsigned char* Str;
			unsigned int Skip;
			unsigned int Uni;
			vector<int> offsetsUTF;

			offsetsUTF.push_back(0);
			if (!suchtitel.empty())
			{
				Str = (unsigned char*) suchtitel.c_str();
				Skip = 0;
				while(Str[Skip])
				{
					cursorPosition++;
					Skip += ToUTF(&Str[Skip],&Uni);
					offsetsUTF.push_back(Skip);
				}
			}

			int countdown = 0;

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
						Str = (unsigned char*) suchtitel.c_str();
						Skip = 0;
						while(Str[Skip])
						{
							Skip += ToUTF(&Str[Skip],&Uni);
							offsetsUTF.push_back(Skip);
						}
					}

					updateSearchbar = true;
					if (updateInRealTime)
					{
						searchSuggestions = true;
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
						Str = (unsigned char*) suchtitel.c_str();
						Skip = 0;
						while(Str[Skip])
						{
							Skip += ToUTF(&Str[Skip],&Uni);
							offsetsUTF.push_back(Skip);
						}
					}
					updateSearchbar = true;
					if (updateInRealTime)
					{
						searchSuggestions = true;
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
					setNewHistoryItem = true;
					loadArticle = true;
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
							updateSearchbar = true;
							if (updateInRealTime)
							{
								searchSuggestions = true;
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
							setNewHistoryItem = true;
							loadArticle = true;
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
								Str = (unsigned char*) suchtitel.c_str();
								Skip = 0;
								while(Str[Skip])
								{
									cursorPosition++;
									Skip += ToUTF(&Str[Skip],&Uni);
									offsetsUTF.push_back(Skip);
								}
							}
							updateSearchbar = true;
							if (updateInRealTime)
							{
								searchSuggestions = true;
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
								updateSuggestions = true;
							}
						}
						else if (PA_SpriteTouched(SPRITE_1UPARROW))
						{
							if (s->scrollLineUp())
							{
								PA_Sleep(10);
								updateSuggestions = true;
							}
						}
						else if (PA_SpriteTouched(SPRITE_1DOWNARROW))
						{
							if (s->scrollLineDown())
							{
								PA_Sleep(10);
								updateSuggestions = true;
							}
						}
						else if (PA_SpriteTouched(SPRITE_2DOWNARROW))
						{
							if (s->scrollPageDown())
							{
								PA_Sleep(10);
								updateSuggestions = true;
							}
						}
						else if (PA_SpriteTouched(SPRITE_HISTORY) || PA_SpriteTouched(SPRITE_HISTORYX))
						{
							updateInRealTime = !updateInRealTime;
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
							searchSuggestions = true;
							countdown = 0;
						}
						else if (PA_SpriteTouched(SPRITE_1LEFTARROW))
						{
							if (cursorPosition>0)
							{
								cursorPosition--;
								updateSearchbar = true;
							}
						}
						else if (PA_SpriteTouched(SPRITE_1RIGHTARROW))
						{
							if (offsetsUTF[cursorPosition]< (int) suchtitel.length())
							{
								cursorPosition++;
								updateSearchbar = true;
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
						updateSuggestions = true;
					}
				}

				if ((Pad.Newpress.Down||Pad.Held.Down))
				{
					if (s->scrollLineDown())
					{
						PA_Sleep(10);
						updateSuggestions = true;
					}
				}

				if ((Pad.Newpress.Left||Pad.Held.Left))
				{
					if (s->scrollPageUp())
					{
						PA_Sleep(10);
						updateSuggestions = true;
					}
				}

				if ((Pad.Newpress.Right||Pad.Held.Right))
				{
					if (s->scrollPageDown())
					{
						PA_Sleep(10);
						updateSuggestions = true;
					}
				}

				if ((Pad.Newpress.L||Pad.Held.L))
				{
					if (s->scrollLongUp())
					{
						updateSuggestions = true;
					}
				}

				if ((Pad.Newpress.R||Pad.Held.R))
				{
					if (s->scrollLongDown())
					{
						updateSuggestions = true;
					}
				}


				if (Pad.Newpress.X)
				{
					break;
				}

				if (updateSearchbar)
				{
					FillVS(&Searchbar,PA_RGB(28,28,28));
					CharStat tmpCS = NormalCS;
					tmpCS.Wrap = NOWRAP;
					CharArea = (BLOCK) {{2,5},{0,0}};
					iPrint(suchtitel,&Searchbar,&tmpCS,&CharArea);
					updateCursor = true;
					updateSearchbar = false;
				}

				if (updateCursor) // TODO
				{

					CharStat tmpCS = NormalCS;
					tmpCS.Wrap = NOWRAP;
					tmpCS.Fx = SIMULATE;
					CharArea = (BLOCK) {{2,5},{0,0}};
					iPrint(suchtitel.substr(0,offsetsUTF[cursorPosition]),&Searchbar,&tmpCS,&CharArea);
					BLOCK temp = {{CharArea.Start.x-1,2},{CharArea.Start.x-1,19}};
					DrawBlock(&Searchbar,temp,PA_RGB(20,20,20),0);
				}

				if (searchSuggestions) // load current searchstring, this is the bottleneck
				{
					s->load(suchtitel);
					updateSuggestions = true;
					searchSuggestions = false;
				}

				if (updateSuggestions) // update display
				{
					s->display();
					updateSuggestions = false;
				}

				if (countdown > 0)
				{
					countdown--;
					if (countdown==0)
					{
						searchSuggestions = true;
					}
				}
				PA_CheckLid();
				PA_WaitForVBL();
			}

			offsetsUTF.clear();

			PA_Clear16bitBg(1);
			PA_Clear16bitBg(0);

			PA_ScrollKeyboardXY(24,200);
			for (int i=SPRITE_HISTORY;i<=SPRITE_CLEARLEFT;i++)
				PA_SetSpriteXY(0,i,-16,-16);
			PA_SetSpriteXY(0, SPRITE_CONFIGURE, 0, 176);
// 			PA_SetSpriteXY(0, SPRITE_BOOKMARKADD, 32, 176);
			PA_SetSpriteXY(0, SPRITE_BOOKMARK, 64, 176);
			PA_SetSpriteXY(0, SPRITE_VIEWMAG, 96, 176);

			updateTitle = true;
			updateContent = true;
			updateStatusbarVS = true;
		}

		PA_CheckLid();
		PA_WaitForVBL();

	} // end of main loop

	return 0;
}
