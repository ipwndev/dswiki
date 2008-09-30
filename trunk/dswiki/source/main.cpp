#include <PA9.h>
#include <fat.h>
#include <sys/dir.h>
#include <string>
#include <algorithm>

#include "main.h"

#include "api.h"
#include "Big52Uni16.h"
#include "Cache.h"
#include "chrlib.h"
#include "History.h"
#include "Markup.h"
#include "SearchResults.h"
#include "struct.h"
#include "frankenstein.h"
#include "TitleIndex.h"
#include "WikiMarkupGetter.h"
#include "TextList.h"
#include "minIni.h"
#include "GlobalSettings.h"


int main(int argc, char ** argv)
{
	// PAlib Initialization

	PA_Init();
	PA_InitVBL();
	PA_InitFat();
	PA_SetAutoCheckLid(1);

	PA_Init16bitBg(0, 3);
	PA_Init16bitBg(1, 3);

	PA_InitText   (0, 2);
	PA_InitText   (1, 2);

	PA_InitKeyboard(2);
	PA_KeyboardOut();

	PA_SetBgPalCol(0, 0, PA_RGB(31,31,31));
	PA_SetBgPalCol(1, 0, PA_RGB(31,31,31));

	PA_SetTextCol (0, 0, 0, 0);
	PA_SetTextCol (1, 0, 0, 0);

	// important variables

	Font stdFont;
	InitFont(&stdFont,frankenstein);

	Device	UpScreen = {"U", 1, (u16*)PA_DrawBg[1], 256, 192};
	Device	DnScreen = {"D", 0, (u16*)PA_DrawBg[0], 256, 192};

	VirScreen  Titlebar    = {   0,   0, 256,  16, {{0,0},{0,0}}, &UpScreen}; InitVS(&Titlebar);
	VirScreen  ContentWin1 = {   2,  18, 252, 172, {{0,0},{0,0}}, &UpScreen}; InitVS(&ContentWin1);
	VirScreen  ContentWin2 = {   2,   2, 252, 172, {{0,0},{0,0}}, &DnScreen}; InitVS(&ContentWin2);
	VirScreen  Statusbar   = {   0, 176, 256,  16, {{0,0},{0,0}}, &DnScreen}; InitVS(&Statusbar);
	VirScreen  PercentArea = { 226, 176,  30,  16, {{0,0},{0,0}}, &DnScreen}; InitVS(&Statusbar);
	VirScreen  Searchbar   = {  47,  37, 162,  22, {{0,0},{0,0}}, &DnScreen}; InitVS(&Searchbar);

	CharStat       TitlebarCS = { &stdFont, 1, 1, PA_RGB(31,31,31), PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), DEG0,   HARDWRAP, NONE, 0 };
	CharStat        ContentCS = { &stdFont, 0, 0, PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), DEG0, NORMALWRAP, NONE, 0 };
	CharStat      StatusbarCS = { &stdFont, 1, 1, PA_RGB( 5, 5, 5), PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), DEG0,   HARDWRAP, NONE, 0 };
	CharStat StatusbarErrorCS = { &stdFont, 1, 1, PA_RGB(27, 4, 4), PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), DEG0,   HARDWRAP, NONE, 0 };
	CharStat SearchResultsCS1 = { &stdFont, 0, 0, PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), DEG0,     NOWRAP, NONE, 0 };
	CharStat SearchResultsCS2 = { &stdFont, 0, 0, PA_RGB(31, 0, 0), PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), DEG0,     NOWRAP, NONE, 0 };
	CharStat SearchResultsCS3 = { &stdFont, 0, 0, PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), DEG0,     NOWRAP, SIMULATE, 0 };

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

	ArticleSearchResult* suchergebnis = NULL;
	ArticleSearchResult* redirection  = NULL;

	string markupstr;
	string suchtitel;
	string currentTitle;

	History          h;
	Cache            c;

	Markup* markup = NULL;

	// start of main program


	// check for DSwiki's home directory
	DIR_ITER* dswikiDir = diropen ("fat:/dswiki/");
	if (dswikiDir == NULL)
	{
		SimPrint("The directory \"/dswiki\" does not exist. Please create it and move all dump files into it.\n\n\nDas Verzeichnis \"/dswiki\" existiert nicht. Bitte erstelle es und verschiebe alle Dumps hinein.",&DnScreen,PA_RGB(31,0,0),UTF8);
		return 0;
	}

	vector<string> possibleWikis = TitleIndex::getPossibleWikis();

	TextList::show("Wiki wählen",possibleWikis,0,0,0);

	if (possibleWikis.size()==0)
	{
		PA_OutputText(1,5,5,"%c1Error: %c0No Wikis found!");
		return 0;
	}

	int currentSelectedWiki = 0;

	if (possibleWikis.size()>1)
	{
		sort(possibleWikis.begin(),possibleWikis.end());
		PA_OutputText(1,1,0,"Choose Wiki\n-----------");
		int i;
		u8 updateSelectedWiki = 1;
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
	}

	FillVS(&Titlebar, PA_RGB( 9,16,28));
	FillVS(&Statusbar,PA_RGB(26,26,26));

	CharArea = (BLOCK) {{2,2},{0,0}};
	iPrint("Lade dewiki.dat...",&Statusbar,&StatusbarCS,&CharArea,-1,UTF8);
	TitleIndex       t(possibleWikis[currentSelectedWiki]);
	FillVS(&Statusbar,PA_RGB(26,26,26));

// 	t.test(terminu1s2p);

	FillVS(&Statusbar,PA_RGB(26,26,26));
	CharArea = (BLOCK) {{2,2},{0,0}};
	iPrint("Initialisiere MarkupGetter...",&Statusbar,&StatusbarCS,&CharArea,-1,UTF8);
	WikiMarkupGetter m(possibleWikis[currentSelectedWiki]);
	FillVS(&Statusbar,PA_RGB(26,26,26));

	SearchResults s(&t,&ContentWin1,&SearchResultsCS1,&SearchResultsCS2);

	u8  updateTitle       = 0;
	u8  updateContent     = 0;
	u8  updateStatusbar   = 0;
	u8  updatePercent     = 0;
	u8  updateInRealTime  = 1;

	s32 forcedLine        = 0;
	u8  setNewHistoryItem = 1;
	u8  loadArticle       = 1;

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
				h.updateCurrentLine(markup->currentLine());
				updateContent = 1;
				PA_Sleep(10);
			}
		}

		if (Pad.Newpress.Right||Pad.Held.Right)
		{
			if (markup->scrollPageDown())
			{
				h.updateCurrentLine(markup->currentLine());
				updateContent = 1;
				PA_Sleep(10);
			}
		}

		if (Pad.Newpress.Up||Pad.Held.Up)
		{
			if (markup->scrollLineUp())
			{
				h.updateCurrentLine(markup->currentLine());
				updateContent = 1;
				PA_Sleep(10);
			}
		}

		if ((Pad.Newpress.Down||Pad.Held.Down))
		{
			if (markup->scrollLineDown())
			{
				h.updateCurrentLine(markup->currentLine());
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

			FillVS(&Statusbar, PA_RGB(18,22,28));
			CharArea = (BLOCK) {{5,2},{0,0}};
			iPrint(currentTitle,&Statusbar,&TitlebarCS,&CharArea,-1,UTF8);

			char letter = 0;
			u8 updateSearchbar   = 1;
			u8 updateSuggestions = 1;
			u8 searchSuggestions = 1;
			u8 updateCursor      = 1;
			s16 cursorPosition = suchtitel.length();

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

				if (letter > 31) { // there is a new letter
					suchtitel = suchtitel.substr(0,cursorPosition) + letter + suchtitel.substr(cursorPosition);
					cursorPosition++;
					updateSearchbar = 1;
					if (updateInRealTime)
						searchSuggestions = 1;
				}

				if ((letter == PA_BACKSPACE) && (cursorPosition>0))
				{
					suchtitel.erase(cursorPosition-1,1); // Erase the last letter
					cursorPosition--;
					updateSearchbar = 1;
					if (updateInRealTime)
						searchSuggestions = 1;
				}

				if ( (letter == '\n') || (Pad.Newpress.A) )
				{
					suchtitel = s.currentHighlightedItem();
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
							searchSuggestions = 1;
					}
					else if (IsInArea(Btn_OK,S))
					{
						suchtitel = s.currentHighlightedItem();
						forcedLine = 0;
						setNewHistoryItem = 1;
						loadArticle = 1;
						break;
					}
					else if (IsInArea(Statusbar.Bound,S))
					{
						suchtitel = currentTitle;
						cursorPosition = suchtitel.length();
						updateSearchbar = 1;
						if (updateInRealTime)
							searchSuggestions = 1;
					}
					else if (IsInArea(Btn_Cancel,S))
					{
						break;
					}
					else if (IsInArea(Btn_PageUp,S))
					{
						if (s.scrollPageUp())
						{
							PA_Sleep(10);
							updateSuggestions = 1;
						}
					}
					else if (IsInArea(Btn_LineUp,S))
					{
						if (s.scrollLineUp())
						{
							PA_Sleep(10);
							updateSuggestions = 1;
						}
					}
					else if (IsInArea(Btn_LineDown,S))
					{
						if (s.scrollLineDown())
						{
							PA_Sleep(10);
							updateSuggestions = 1;
						}
					}
					else if (IsInArea(Btn_PageDown,S))
					{
						if (s.scrollPageDown())
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

				if ((Pad.Newpress.Up||Pad.Held.Up))
				{
					if (s.scrollLineUp())
					{
						PA_Sleep(10);
						updateSuggestions = 1;
					}
				}

				if ((Pad.Newpress.Down||Pad.Held.Down))
				{
					if (s.scrollLineDown())
					{
						PA_Sleep(10);
						updateSuggestions = 1;
					}
				}

				if ((Pad.Newpress.Left||Pad.Held.Left))
				{
					if (s.scrollPageUp())
					{
						PA_Sleep(10);
						updateSuggestions = 1;
					}
				}

				if ((Pad.Newpress.Right||Pad.Held.Right))
				{
					if (s.scrollPageDown())
					{
						PA_Sleep(10);
						updateSuggestions = 1;
					}
				}

				if ((Pad.Newpress.L||Pad.Held.L))
				{
					if (s.scrollLongUp())
					{
						updateSuggestions = 1;
					}
				}

				if ((Pad.Newpress.R||Pad.Held.R))
				{
					if (s.scrollLongDown())
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

				if (updateCursor)
				{
					u16 cursorX = 0;
					int i;
					CharArea = (BLOCK) {{2,5},{0,0}};
					iPrint(suchtitel.substr(0,cursorPosition),&Searchbar,&SearchResultsCS1,&CharArea,-1,UTF8);
					BLOCK temp = {{CharArea.Start.x-1,2},{CharArea.Start.x-1,19}};
					DrawBlock(&Searchbar,temp,PA_RGB(20,20,20),1);
				}

				if (searchSuggestions) // load current searchstring
				{
					s.load(suchtitel);
					updateSuggestions = 1;
					searchSuggestions = 0;
				}

				if (updateSuggestions) // update display
				{
					s.display();
					updateSuggestions = 0;
				}

				PA_CheckLid();
				PA_WaitForVBL();
			}

			PA_KeyboardOut();
			PA_Clear16bitBg(1);
			PA_Clear16bitBg(0);

			updateTitle = 1;
			updateContent = 1;
			updateStatusbar = 1;
		}

		if (Pad.Newpress.Y)
		{
		}

		if (Pad.Newpress.L)
		{
			if (h.back())
			{
				suchtitel = h.currentTitle();
				forcedLine = h.currentLine();
				setNewHistoryItem = 0;
				loadArticle = 1;
			}
		}

		if (Pad.Newpress.R)
		{
			if (h.forward())
			{
				suchtitel = h.currentTitle();
				forcedLine = h.currentLine();
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
				u8 updateSelectedWiki = 1;
				while(1)
				{
					if (Pad.Newpress.A)
					{
						t.setNew(possibleWikis[currentSelectedWiki]);
						m.setNew(possibleWikis[currentSelectedWiki]);
						h.clear();
						c.clear();
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
				FillVS(&Statusbar,PA_RGB(26,26,26));
				CharArea = (BLOCK) {{2,2},{0,0}};
				iPrint("Suche zufälligen Artikel...",&Statusbar,&StatusbarCS,&CharArea,-1,UTF8);
				suchergebnis = t.getRandomArticle();
			}
			else
			{
				FillVS(&Statusbar,PA_RGB(26,26,26));
				CharArea = (BLOCK) {{2,2},{0,0}};
				iPrint("Suche Artikel...",&Statusbar,&StatusbarCS,&CharArea,-1,UTF8);
				suchergebnis = t.findArticle(suchtitel);
			}

			if (suchergebnis!=NULL)
			{
				suchtitel.clear();

				FillVS(&Statusbar,PA_RGB(26,26,26));
				CharArea = (BLOCK) {{2,2},{0,0}};
				iPrint("Lade \""+suchergebnis->TitleInArchive()+"\"",&Statusbar,&StatusbarCS,&CharArea,-1,UTF8);
				PA_Sleep(30);
				FillVS(&Statusbar,PA_RGB(26,26,26));
				CharArea = (BLOCK) {{2,2},{0,0}};

				if (c.isInCache(suchergebnis->TitleInArchive()))
				{
					iPrint("Hole Markup aus dem Cache...",&Statusbar,&StatusbarCS,&CharArea,-1,UTF8);
					markupstr = c.getMarkup(suchergebnis->TitleInArchive());
				}
				else
				{
					iPrint("Hole Markup von Disk...",&Statusbar,&StatusbarCS,&CharArea,-1,UTF8);
					markupstr = m.getMarkup(&t, suchergebnis->TitleInArchive());
					c.insert(suchergebnis->TitleInArchive(),markupstr);
				}

				string redirectMessage = "";
				u8 numberOfRedirections = 0;
				while ((numberOfRedirections<MAX_NUMBER_OF_REDIRECTIONS) && (redirection = t.isRedirect(markupstr)))
				{
					numberOfRedirections++;
					ArticleSearchResult* temp = suchergebnis;
					redirectMessage += "(\u2192 "+temp->TitleInArchive()+")\n";
					suchergebnis = redirection;
					FillVS(&Statusbar,PA_RGB(26,26,26));
					CharArea = (BLOCK) {{2,2},{0,0}};
					if (c.isInCache(suchergebnis->TitleInArchive()))
					{
						iPrint("Folge Umleitung aus dem Cache...",&Statusbar,&StatusbarCS,&CharArea,-1,UTF8);
						markupstr = c.getMarkup(suchergebnis->TitleInArchive());
					}
					else
					{
						iPrint("Folge Umleitung von Disk...",&Statusbar,&StatusbarCS,&CharArea,-1,UTF8);
						markupstr = m.getMarkup(&t, suchergebnis->TitleInArchive());
						c.insert(suchergebnis->TitleInArchive(),markupstr);
					}
				}
				currentTitle = suchergebnis->TitleInArchive();

				markupstr = redirectMessage + markupstr;
				markupstr = exchangeSGMLEntities(markupstr);
				delete markup;
				FillVS(&Statusbar,PA_RGB(26,26,26));
				CharArea = (BLOCK) {{2,2},{0,0}};
				iPrint("Formatiere Markup...",&Statusbar,&StatusbarCS,&CharArea,-1,UTF8);
				markup = new Markup(markupstr, &ContentWin1, &ContentWin2, &ContentCS, &t);
				FillVS(&Statusbar,PA_RGB(26,26,26));
				CharArea = (BLOCK) {{2,2},{0,0}};
				iPrint("Formatierung abgeschlossen",&Statusbar,&StatusbarCS,&CharArea,-1,UTF8);
				PA_Sleep(30);

				markupstr.clear();

				markup->setCurrentLine(forcedLine);

				if (setNewHistoryItem)
				{
					h.insert(suchergebnis->TitleInArchive(),0);
				}

				FillVS(&Statusbar,PA_RGB(26,26,26));
				updateTitle = 1;
				updateContent = 1;
			}
			else
			{
				FillVS(&Statusbar,PA_RGB(26,26,26));
				CharArea = (BLOCK) {{2,2},{0,0}};
				iPrint("\""+suchtitel+"\" nicht gefunden...",&Statusbar,&StatusbarErrorCS,&CharArea,-1,UTF8);
				PA_Sleep(60);
				FillVS(&Statusbar,PA_RGB(26,26,26));
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

		if (updateStatusbar)
		{
			FillVS(&Statusbar,PA_RGB(26,26,26));
			updatePercent = 1;
			updateStatusbar = 0;
		}

		if (updateContent)
		{
			markup->draw();
			updatePercent = 1;
			updateContent = 0;
		}

		if (updatePercent)
		{
			char out[5];
			sprintf(out,"%d",markup->currentPercent());
			string percentstr(out);
			while (percentstr.length()<3)
			{
				percentstr = " "+percentstr;
			}
			percentstr += "%";
			CharArea = (BLOCK) {{2,2},{0,0}};
			FillVS(&PercentArea,PA_RGB(26,26,26));
			iPrint(percentstr,&PercentArea,&StatusbarCS,&CharArea,-1,UTF8);
		}

		PA_CheckLid();
		PA_WaitForVBL();
	}
	return 0;
}
