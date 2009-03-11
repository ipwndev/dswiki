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
#include "Dumps.h"
#include "FATBrowser.h"
#include "History.h"
#include "Markup.h"
#include "Globals.h"
#include "PercentIndicator.h"
#include "Statusbar.h"
#include "Search.h"
#include "TitleIndex.h"
#include "TextBox.h"
#include "WIKI2XML.h"
#include "WIKI2XML_global.h"
#include "WikiMarkupGetter.h"

bool debug;

Device   UpScreen;
Device   DnScreen;
CharStat NormalCS;

VirScreen Titlebar;
VirScreen ContentWin1;
VirScreen ContentWin0;
VirScreen StatusbarVS;
VirScreen PercentArea;

#define DEBUG 1
#define DEBUG_WIKI_NR 1
#define STRESSTEST 0

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

	string search_title    = "Temp";
// 	string search_title    = "";
	string search_anchor  = "";
	string currentTitle = "";

	vector<string> possibleWikis;
	string currentWiki;
	int    currentWikiNumber = -1;

	bool chooseNewWiki		= false; // the first choice is handled differently
	bool loadNewWiki		= true;
	bool loadInternalWiki	= false;
	bool loadArticle		= true;
	bool search				= false;
	bool showMenu			= false;

	bool updateTitle		= false;
	bool updateContent		= false;
	bool updateStatusbar	= false;
	bool updatePercent		= false;
	bool suggestInRealTime	= true;

	bool setNewHistoryItem	= true;
	bool textBrowserMode	= false;

	int forcedLine			= 0;

	ArticleSearchResult*	suchergebnis	= NULL;
	ArticleSearchResult*	redirection		= NULL;
	Markup*					markup			= NULL;
	TitleIndex*				t				= NULL;
	WikiMarkupGetter*		wmg				= NULL;
	TextBox*				WikiChooser		= NULL;
	Statusbar*				sb				= NULL;
	PercentIndicator*		p				= NULL;
	Dumps*					d				= NULL;
	History*				h				= NULL;
	Search*					s				= NULL;
	FATBrowser*				fb				= NULL;
	Globals*				g				= NULL;

	g = new Globals();
	g->setLanguage(PA_UserInfo.Language);

	PA_Init16bitBg(0, 3);
	PA_Init16bitBg(1, 3);
	PA_InitText(1,2);
#if DEBUG
	PA_InitText(0,2);
#endif

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

	PA_Sleep(30); // TODO
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

	d = new Dumps();
	g->setDumps(d);

	// get all installed dumps
	possibleWikis = d->getPossibleWikis(loadInternalWiki);

	if (possibleWikis.size() == 0)
	{
		PA_OutputText(1,0,0,"%c1No dumps %c0were found!");
		PA_Sleep(120);
		PA_ClearTextBg(1);
	}

	// all sprites for the interface
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
	KT_CreateSprite(0, SPRITE_BOOKMARK,    "dswiki/icons/bookmark",      OBJ_SIZE_16X16, 1, 0, 0, -16, -16);
	KT_CreateSprite(0, SPRITE_VIEWMAG,     "dswiki/icons/viewmag",       OBJ_SIZE_16X16, 1, 0, 0, -16, -16);
	KT_CreateSprite(0, SPRITE_FILEOPEN,    "dswiki/icons/fileopen",      OBJ_SIZE_16X16, 1, 0, 0, -16, -16);

	// Initialization of global variables
	UpScreen = (Device)   { "U", 1, (unsigned short int*)PA_DrawBg[1], 256, 192};
	DnScreen = (Device)   { "D", 0, (unsigned short int*)PA_DrawBg[0], 256, 192};
	NormalCS = (CharStat) { g->getFont(FONT_R), 0, 0, g->textColor(), PA_RGB( 0, 0, 0), g->backgroundColor(), DEG0, NORMALWRAP, NONE, 0 };

	Titlebar    = (VirScreen) {   0,   0, 256,  16, {{0,0},{0,0}}, &UpScreen}; InitVS(&Titlebar);
	ContentWin1 = (VirScreen) {   2,  18, 252, 172, {{0,0},{0,0}}, &UpScreen}; InitVS(&ContentWin1);
	ContentWin0 = (VirScreen) {   2,   2, 252, 172, {{0,0},{0,0}}, &DnScreen}; InitVS(&ContentWin0);
	StatusbarVS = (VirScreen) {   0, 176, 256,  16, {{0,0},{0,0}}, &DnScreen}; InitVS(&StatusbarVS);
	PercentArea = (VirScreen) { 229, 176,  27,  16, {{0,0},{0,0}}, &DnScreen}; InitVS(&PercentArea);
	// End of global variables

	VirScreen  Searchbar   =  {  47,  37, 162,  22, {{0,0},{0,0}}, &DnScreen}; InitVS(&Searchbar);

	BLOCK CharArea = {{ 0, 0},{ 0, 0}};

	sb = new Statusbar();
	p = new PercentIndicator();
	h = new History();
	s = new Search();
	fb = new FATBrowser();

	s->setGlobals(g);
	g->setStatusbar(sb);
	g->setPercentIndicator(p);

	// use graphical interface from now on
	WikiChooser = new TextBox(possibleWikis);
	WikiChooser->setTitle("Choose your Wiki");
	WikiChooser->allowCancel(false);

#if !DEBUG
	if ( possibleWikis.size() > 1 )
	{
		currentWikiNumber = WikiChooser->run();
		currentWiki = possibleWikis[currentWikiNumber];
	}
	else if ( possibleWikis.size() == 1 )
	{
		currentWikiNumber = 0;
		currentWiki = possibleWikis[0];
	}
	else
	{
		loadInternalWiki = true;
		currentWikiNumber = -1;
		currentWiki = "manual";
		search_title = "Troubleshooting";
	}
#else
	currentWikiNumber = DEBUG_WIKI_NR;
	currentWiki = possibleWikis[DEBUG_WIKI_NR];
#endif


	FillVS(&Titlebar, PA_RGB( 9,16,28));

	g->getStatusbar()->clear();
	g->getPercentIndicator()->clear();

	while(1) // main loop
	{
		if (chooseNewWiki)
		{
			// Pops up a dialog to select a new dump or a file from FAT.
			// Internal wikis are not included (eg. the manual can be loaded from the menu).
			int currentWikiNumber_Backup = currentWikiNumber;
			if (loadInternalWiki)
			{
				currentWikiNumber_Backup = -1;
			}

			possibleWikis = g->getDumps()->getPossibleWikis(false);
			possibleWikis.push_back("[Load ASCII/UTF8-file...]");

			if (WikiChooser)
				delete WikiChooser;

			WikiChooser = new TextBox(possibleWikis);
			WikiChooser->setTitle("Choose your Wiki");
			WikiChooser->allowCancel(true);
			WikiChooser->allowSingleElement();
			WikiChooser->setCurrentPosition(currentWikiNumber_Backup);

			currentWikiNumber = WikiChooser->run();

			if (currentWikiNumber >= 0)
			{
				if (currentWikiNumber == (int) possibleWikis.size()-1)
				{
					// load file from FAT
					string filename = fb->selectFile();
					if (!filename.empty())
					{
						FATBrowser::getFileContents(markupstr,filename);
						currentTitle = filename;
						if (markup)
							delete markup;
						markup = new Markup();
						g->setMarkup(markup);
						markup->setGlobals(g);
						markup->parse(markupstr,false);
						updateContent = true;
						updatePercent = true;
						updateStatusbar = true;
						updateTitle = true;
					}
				}
				else if (currentWikiNumber != currentWikiNumber_Backup)
				{
					// some new dump was chosen
					currentWiki = possibleWikis[currentWikiNumber];
					loadInternalWiki = false;
					loadNewWiki = true;
				}
			}
			else
			{
				// cancelled
				currentWikiNumber = currentWikiNumber_Backup;
			}

			updateTitle = true;
			updateContent = true;
			chooseNewWiki = false;
		}

		if (loadNewWiki)
		{
			// Loads a new wiki. The variables 'currentWiki' and 'loadInternalWiki' have to be set correctly
			g->getStatusbar()->display("Loading " + currentWiki + "...");
			if (t)
				delete t;
			t = new TitleIndex();
			g->setTitleIndex(t);
			t->setGlobals(g);
			t->load(currentWiki,loadInternalWiki);

			g->getStatusbar()->display("Initializing MarkupGetter...");
			if (wmg)
				delete wmg;
			wmg = new WikiMarkupGetter();
			g->setWikiMarkupGetter(wmg);
			wmg->setGlobals(g);
			wmg->load(currentWiki,loadInternalWiki);

			g->getStatusbar()->clear();

			h->clear();

			loadArticle = true;
			loadNewWiki = false;
		}

		if (Stylus.Newpress)
		{
			if (PA_SpriteTouched(SPRITE_CONFIGURE))
			{
				showMenu = true;
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
					search_title = bookmark;
					forcedLine = 0;
					setNewHistoryItem = true;
					loadArticle = true;
				}
			}
			else if (PA_SpriteTouched(SPRITE_FILEOPEN))
			{
				chooseNewWiki = true;
			}
			else
			{
				POINT p_click = {Stylus.X,Stylus.Y};
				if (IsInArea(ContentWin0.AbsoluteBound,p_click))
				{
					if (markup->evaluateClick(Stylus.X,Stylus.Y) )
					{
						markup->getCurrentLink(search_title,search_anchor);
						forcedLine = 0;
						setNewHistoryItem = true;
						loadArticle = true;
					}
				}
			}
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

		if (Pad.Newpress.Left || Pad.Held.Left || GHPad.Newpress.Blue || GHPad.Held.Blue)
		{
			markup->scrollPageUp();
			h->updateCurrentLine(markup->currentLine());
			PA_Sleep(10);
			updateContent = true;
		}

		if (Pad.Newpress.Right || Pad.Held.Right || GHPad.Newpress.Green || GHPad.Held.Green)
		{
			markup->scrollPageDown();
			h->updateCurrentLine(markup->currentLine());
			PA_Sleep(10);
			updateContent = true;
		}

		// line scrolling or link-jumping
		if (Pad.Newpress.Up||Pad.Held.Up)
		{
			if (textBrowserMode)
			{
				markup->selectPreviousLink();
			}
			else
			{
				markup->scrollLineUp();
			}
			h->updateCurrentLine(markup->currentLine());
			PA_Sleep(10);
			updateContent = true;
		}

		// line scrolling or link-jumping
		if ((Pad.Newpress.Down||Pad.Held.Down))
		{
			if (textBrowserMode)
			{
				markup->selectNextLink();
			}
			else
			{
				markup->scrollLineDown();
			}
			h->updateCurrentLine(markup->currentLine());
			PA_Sleep(10);
			updateContent = true;
		}

		if (Pad.Newpress.A)
		{
			search_title.clear();
			search_anchor.clear();
			if (markup)
			{
				markup->getCurrentLink(search_title, search_anchor);
			}
			forcedLine = 0;
			setNewHistoryItem = true;
			loadArticle = true;
		}

		if ( Pad.Released.B && (Pad.Downtime.B<60) )
		{
			markup->unselect();
			updateContent = true;
		}
		else if ( Pad.Held.B && ( Pad.Downtime.B >= 60 ) )
		{
			textBrowserMode = !textBrowserMode;
			if (textBrowserMode)
			{
				g->getStatusbar()->displayClearAfter("Up/Down navigate between links",40);
			}
			else
			{
				g->getStatusbar()->displayClearAfter("Up/Down scroll lines",40);
			}
		}

		if (Pad.Newpress.X)
		{
			search = true;
		}

		if (Pad.Newpress.Y)
		{
			markup->toggleIndex();
			updateContent = true;
		}

		if ( (Pad.Released.R) && (Pad.Downtime.R<60) )
		{
			if (h->forward())
			{
				search_title = h->currentTitle();
				forcedLine = h->currentLine();
				setNewHistoryItem = false;
				loadArticle = true;
			}
		}
		else if ( (Pad.Released.L) && (Pad.Downtime.L<60) )
		{
			if (h->back())
			{
				search_title = h->currentTitle();
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
				search_title = history_vec[chosenHistItem];
				loadArticle = true;
			}
			updateContent = true;
		}

		if (Pad.Newpress.Start)
		{
			showMenu = true;
		}

		if (Pad.Newpress.Select)
		{
			chooseNewWiki = true;
		}

#if STRESSTEST
		loadArticle = true;
#endif

		if (loadArticle)
		{
			delete suchergebnis;
			suchergebnis = NULL;

			if (search_title.empty())
			{
				g->getStatusbar()->display("Searching random article...");
				suchergebnis = t->getRandomArticle();
			}
			else
			{
				g->getStatusbar()->display("Searching "+search_title+"...");
				suchergebnis = t->findArticle(search_title,search_anchor,currentTitle);
			}

			if (suchergebnis)
			{
				if (markup)
				{
					delete markup;
					markup = NULL;
				}

				search_title.clear();
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
					redirectMessage += "("+temp->TitleInArchive()+" \u2192)\n";
					suchergebnis = redirection;

					markupstr.clear();
					g->getStatusbar()->display("Following redirection...");
					g->getWikiMarkupGetter()->getMarkup(markupstr, suchergebnis->TitleInArchive());
				}

				currentTitle = suchergebnis->TitleInArchive();
				markupstr.insert(0,redirectMessage);

				FillVS(&Titlebar, PA_RGB(16,16,16));
				CharArea = (BLOCK) {{5,2},{0,0}};
				CharStat TitlebarCS = NormalCS;
				TitlebarCS.Color = PA_RGB(31,31,31);
				TitlebarCS.Wrap = NOWRAP;

				iPrint(currentTitle,&Titlebar,&TitlebarCS,&CharArea);
				g->getStatusbar()->display("Processing \""+currentTitle+"\"...");

				markup = new Markup();
				g->setMarkup(markup);
				markup->setGlobals(g);
				markup->parse(markupstr);

				if (!markup->LoadOK())
				{
					delete markup;

					g->getStatusbar()->displayErrorClearAfter("XML-Parsing failed!",60);
					g->getStatusbar()->displayClearAfter("Printing the original markup...",60);

					markupstr.clear();
					g->getWikiMarkupGetter()->getMarkup(markupstr, currentTitle);
					markupstr.insert(0,redirectMessage);

					markup = new Markup();
					g->setMarkup(markup);
					markup->setGlobals(g);
					markup->parse(markupstr, false);
				}

				g->getStatusbar()->displayClearAfter("Processing complete",30);

				markup->scrollToLine(forcedLine); // TODO: or scroll to suchergebnis->Anchor()

				if (setNewHistoryItem)
				{
					h->insert(currentTitle, forcedLine);
				}

				updateTitle = true;
				updateContent = true;
			}
			else
			{
				g->getStatusbar()->displayErrorClearAfter("\""+search_title+"\" not found...",90);
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
			TitlebarCS.Wrap = NOWRAP;
			iPrint(currentTitle,&Titlebar,&TitlebarCS,&CharArea);
			updateTitle = false;
		}

		if (updateStatusbar)
		{
			g->getStatusbar()->clear();
			updatePercent = true;
			updateStatusbar = false;
		}

		if (updateContent)
		{
			bool force = true;
			g->getMarkup()->draw(force);
			updatePercent = true;
			updateContent = false;
		}

		if (updatePercent)
		{
			g->getPercentIndicator()->update(markup->currentPercent());
			g->getPercentIndicator()->redraw();
			updatePercent = false;
		}

		if (search)
		{
			PA_Clear16bitBg(1);
			PA_Clear16bitBg(0);

			Statusbar::removeIcons();
			Search::showIcons();
			Search::showUpdateIcons(suggestInRealTime);

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
			if (!search_title.empty())
			{
				Str = (unsigned char*) search_title.c_str();
				Skip = 0;
				while(Str[Skip])
				{
					cursorPosition++;
					Skip += ToUTF(&Str[Skip],&Uni);
					offsetsUTF.push_back(Skip);
				}
			}

			int countdown = 0;


			PA_WaitForVBL();


			while(1)
			{
				letter = PA_CheckKeyboard();

				if (letter > 31)
				{
					// there is a new letter
					search_title.insert(search_title.begin()+offsetsUTF[cursorPosition],letter);
					cursorPosition++;

					offsetsUTF.clear();
					offsetsUTF.push_back(0);
					if (!search_title.empty())
					{
						Str = (unsigned char*) search_title.c_str();
						Skip = 0;
						while(Str[Skip])
						{
							Skip += ToUTF(&Str[Skip],&Uni);
							offsetsUTF.push_back(Skip);
						}
					}

					updateSearchbar = true;
					if (suggestInRealTime)
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
					search_title.erase(offsetsUTF[cursorPosition-1],offsetsUTF[cursorPosition]-offsetsUTF[cursorPosition-1]); // Erase the last letter
					cursorPosition--;
					offsetsUTF.clear();
					offsetsUTF.push_back(0);
					if (!search_title.empty())
					{
						Str = (unsigned char*) search_title.c_str();
						Skip = 0;
						while(Str[Skip])
						{
							Skip += ToUTF(&Str[Skip],&Uni);
							offsetsUTF.push_back(Skip);
						}
					}
					updateSearchbar = true;
					if (suggestInRealTime)
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
					search_title = s->currentHighlightedItem();
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
						if (PA_SpriteTouched(SPRITE_CLEARLEFT) && (!search_title.empty()) )
						{
							search_title.clear();
							offsetsUTF.clear();
							offsetsUTF.push_back(0);
							cursorPosition = 0;
							updateSearchbar = true;
							if (suggestInRealTime)
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
							search_title = s->currentHighlightedItem();
							forcedLine = 0;
							setNewHistoryItem = true;
							loadArticle = true;
							break;
						}
						else if (IsInArea(StatusbarVS.AbsoluteBound,S))
						{
							search_title = currentTitle;
							offsetsUTF.clear();
							offsetsUTF.push_back(0);
							cursorPosition = 0;
							if (!search_title.empty())
							{
								Str = (unsigned char*) search_title.c_str();
								Skip = 0;
								while(Str[Skip])
								{
									cursorPosition++;
									Skip += ToUTF(&Str[Skip],&Uni);
									offsetsUTF.push_back(Skip);
								}
							}
							updateSearchbar = true;
							if (suggestInRealTime)
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
							suggestInRealTime = !suggestInRealTime;
							Search::showUpdateIcons(suggestInRealTime);
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
							if (offsetsUTF[cursorPosition]< (int) search_title.length())
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
					iPrint(search_title,&Searchbar,&tmpCS,&CharArea);
					updateCursor = true;
					updateSearchbar = false;
				}

				if (updateCursor)
				{

					CharStat tmpCS = NormalCS;
					tmpCS.Wrap = NOWRAP;
					tmpCS.Fx = SIMULATE;
					CharArea = (BLOCK) {{2,5},{0,0}};
					iPrint(search_title.substr(0,offsetsUTF[cursorPosition]),&Searchbar,&tmpCS,&CharArea);
					BLOCK temp = {{CharArea.Start.x-1,2},{CharArea.Start.x-1,19}};
					DrawBlock(&Searchbar,temp,PA_RGB(20,20,20),0);
				}

				if (searchSuggestions) // load current searchstring, this is the bottleneck
				{
					s->load(search_title);
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

			Search::removeIcons();
			Statusbar::showIcons();

			updateTitle = true;
			updateContent = true;
			updateStatusbar = true;
			search = false;
		}

		if (showMenu)
		{
			vector<string> menu;
			menu.push_back("Load Bookmark");		// 0
			menu.push_back("Add Bookmark");			// 1
			menu.push_back("Help/Manual");			// 2
			menu.push_back("Invert color scheme");	// 3

			TextBox Options(menu);
			Options.setTitle("Options");
			Options.allowCancel(true);
			Options.setCurrentPosition(0);

			int choice = Options.run();

			switch (choice)
			{
				case -1:
				default:
					break;
				case 0:
				{
					g->getStatusbar()->displayClearAfter("Loading Bookmarks",45);
					string bookmark = g->loadBookmark();
					if (!bookmark.empty())
					{
						search_title = bookmark;
						forcedLine = 0;
						setNewHistoryItem = true;
						loadArticle = true;
					}
					updateContent = true;
					break;
				}
				case 1:
				{
					g->saveBookmark(currentTitle);
					break;
				}
				case 2:
				{
					loadNewWiki = true;
					loadInternalWiki = true;
					currentWiki = "manual";
					search_title = "Introduction";
					break;
				}
				case 3:
				{
					g->toggleInverted();
					break;
				}
			}

			updateContent = true;
			showMenu = false;
		}

		PA_CheckLid();
		PA_WaitForVBL();

	} // end of main loop

	return 0;
}
