#include <PA9.h>
#include <string>

#include "main.h"

#include "api.h"
#include "Big52Uni16.h"
#include "Cache.h"
#include "chrlib.h"
#include "History.h"
#include "Markup.h"
#include "SearchResults.h"
#include "struct.h"
#include "ter12rp.h"
#include "unifont.h"
#include "TitleIndex.h"
#include "WikiMarkupGetter.h"



int main(int argc, char ** argv)
{
	// PAlib Initialization

	PA_Init();
	PA_InitVBL();
	PA_InitFat();

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

// 	string olli = "ABC@ł€¶ŧ←↓→øþ¨æßðđŋħjĸł˝^«»¢“”nµ\n ΩŁ€®Ŧ¥↑ı§ÐªŊĦJ&Ł<>©‘’Nº\u00d7\u00f7\n ÄÖÜßÁÀÂÑØÞÆ";
	Font terminus12p;
	InitFont(&terminus12p,ter12rp);
	Font unifont16p;
	InitFont(&unifont16p,unifont);

	Device	UpScreen = {"U", 1, (u16*)PA_DrawBg[1], 256, 192};
	Device	DnScreen = {"D", 0, (u16*)PA_DrawBg[0], 256, 192};

	VirScreen  Titlebar    = {   0,   0, 256,  16, {{0,0},{0,0}}, &UpScreen}; InitVS(&Titlebar);
	VirScreen  ContentWin1 = {   2,  18, 252, 172, {{0,0},{0,0}}, &UpScreen}; InitVS(&ContentWin1);
	VirScreen  ContentWin2 = {   2,   2, 252, 172, {{0,0},{0,0}}, &DnScreen}; InitVS(&ContentWin2);
	VirScreen  Statusbar   = {   0, 176, 256,  16, {{0,0},{0,0}}, &DnScreen}; InitVS(&Statusbar);
	VirScreen  PercentArea = { 226, 176,  30,  16, {{0,0},{0,0}}, &DnScreen}; InitVS(&Statusbar);
	VirScreen  Searchbar   = {  47,  37, 162,  22, {{0,0},{0,0}}, &DnScreen}; InitVS(&Searchbar);

	CharStat       TitlebarCS = { PA_RGB(31,31,31), PA_RGB( 0, 0, 0),   HARDWRAP, DEG0, NONE, 1, 1, 0, &unifont16p};
	CharStat        ContentCS = { PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), NORMALWRAP, DEG0, NONE, 0, 0, 0, &unifont16p};
	CharStat      StatusbarCS = { PA_RGB( 5, 5, 5), PA_RGB( 0, 0, 0),   HARDWRAP, DEG0, NONE, 1, 1, 0, &unifont16p};
	CharStat StatusbarErrorCS = { PA_RGB(27, 4, 4), PA_RGB( 0, 0, 0),   HARDWRAP, DEG0, NONE, 1, 1, 0, &unifont16p};
	CharStat SearchResultsCS1 = { PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0),     NOWRAP, DEG0, NONE, 0, 0, 0, &unifont16p};
	CharStat SearchResultsCS2 = { PA_RGB(31, 0, 0), PA_RGB( 0, 0, 0),     NOWRAP, DEG0, NONE, 0, 0, 0, &unifont16p};

	BLOCK ClearBtn = {{216,37},{237,58}};
	BLOCK OKBtn    = {{ 18,37},{ 39,58}};
	BLOCK CharArea = {{0,0},{0,0}};

	FillVS(&Titlebar, PA_RGB( 9,16,28));
	FillVS(&Statusbar,PA_RGB(26,26,26));

	ArticleSearchResult* suchergebnis = NULL;
	ArticleSearchResult* redirection  = NULL;

	string markupstr;
	string suchtitel;
	string currentTitle;

	History          h;
	Cache            c;

	Markup* markup = NULL;

	// start of main program

	FillVS(&Statusbar,PA_RGB(26,26,26));
	CharArea = (BLOCK) {{2,2},{0,0}};
	iPrint("Lade dewiki.dat...",&Statusbar,&StatusbarCS,&CharArea,-1,UTF8);
	TitleIndex       t("dewiki");
	FillVS(&Statusbar,PA_RGB(26,26,26));

// 	t.test();

	FillVS(&Statusbar,PA_RGB(26,26,26));
	CharArea = (BLOCK) {{2,2},{0,0}};
	iPrint("Initialisiere MarkupGetter...",&Statusbar,&StatusbarCS,&CharArea,-1,UTF8);
	WikiMarkupGetter m("dewiki");
	FillVS(&Statusbar,PA_RGB(26,26,26));

	SearchResults s(&t,&ContentWin1,&SearchResultsCS1,&SearchResultsCS2);

	u8  updateTitle       = 0;
	u8  updateContent     = 0;
	u8  updateStatusbar   = 0;
	u8  updatePercent     = 0;
	u8  loadArticle       = 1;
	u8  setNewHistoryItem = 1;
	s32 forcedLine        = 0;

	while(1)
	{
		PA_CheckLid();

		if (Stylus.Newpress)
		{
			suchtitel = markup->evaluateClick(Stylus.X,Stylus.Y);
			if (!suchtitel.empty())
			{
				setNewHistoryItem = 1;
				forcedLine = 0;
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
			loadArticle = 1;
		}

		if (Pad.Newpress.B)
		{
		}

		if (Pad.Newpress.X)
		{
			PA_KeyboardIn(24, 72);
			PA_Clear16bitBg(1);
			PA_Clear16bitBg(0);

			char letter = 0;
			u8 updateSearchbar   = 1;
			u8 updateSuggestions = 0;
			u8 searchSuggestions = 0;

			while(1)
			{
				PA_CheckLid();

				letter = PA_CheckKeyboard();

				if (letter > 31) { // there is a new letter
					suchtitel += letter;
					updateSearchbar = 1;
				}

				if ((letter == PA_BACKSPACE) && (!suchtitel.empty()))
				{
					suchtitel.resize(suchtitel.length()-1); // Erase the last letter
					updateSearchbar = 1;
				}

				if ( (letter == '\n') )
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
					if (IsInArea(ClearBtn,S))
					{
						suchtitel.clear();
						updateSearchbar = 1;
					}
					if (IsInArea(OKBtn,S))
					{
						suchtitel = s.currentHighlightedItem();
						forcedLine = 0;
						setNewHistoryItem = 1;
						loadArticle = 1;
						break;
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

				if (Pad.Newpress.A)
				{
					suchtitel = s.currentHighlightedItem();
					forcedLine = 0;
					setNewHistoryItem = 1;
					loadArticle = 1;
					break;
				}

				if (Pad.Newpress.X)
				{
					break;
				}

				if (updateSearchbar)
				{
					DrawBlock(&DnScreen,ClearBtn,PA_RGB(31,0,0),1);
					DrawBlock(&DnScreen,OKBtn,PA_RGB(0,31,0),1);
					FillVS(&Searchbar,PA_RGB(28,28,28));
					CharArea = (BLOCK) {{2,5},{0,0}};
					iPrint(suchtitel,&Searchbar,&SearchResultsCS1,&CharArea,-1,UTF8);
					searchSuggestions = 1;
					updateSearchbar = 0;
				}

				if (searchSuggestions)
				{
					s.load(suchtitel);
					updateSuggestions = 1;
					searchSuggestions = 0;
				}

				if (updateSuggestions)
				{
					s.display();
					updateSuggestions = 0;
				}

				PA_WaitForVBL();
			}

			PA_KeyboardOut();
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
				delete markup;
				markup = new Markup(markupstr, &ContentWin1, &ContentWin2, &ContentCS);
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
			CharArea = (BLOCK) {{5,0},{0,0}};
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


		PA_WaitForVBL();
	}
	return 0;
}
