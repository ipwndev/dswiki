#include <PA9.h>

#include "main.h"
#include "chrlib.h"
#include "api.h"
#include "TitleIndex.h"
#include "WikiMarkupGetter.h"

#include "ter12rp.h"
#include "unifont.h"

int main(int argc, char ** argv)
{
	// PAlib Initialization

	PA_Init();
	PA_InitVBL();
	PA_InitFat();

	PA_Init16bitBg(1, 3);
	PA_Init16bitBg(0, 3);
	PA_InitText   (1, 2);
	PA_InitText   (0, 2);
	PA_SetBgPalCol(1, 0, PA_RGB(31,31,31));
	PA_SetBgPalCol(0, 0, PA_RGB(31,31,31));

	PA_SetTextCol (1, 0, 0, 0);
	PA_SetTextCol (0, 0, 0, 0);

	PA_InitKeyboard(2);
	PA_KeyboardOut();

	// important variables

	Font terminus12p; InitFont(&terminus12p,ter12rp);
// 	Font unifont16;   InitFont(&unifont16,unifont);

	Device	UpScreen = {"U", 1, (u16*)PA_DrawBg[1], 256, 192};
	Device	DnScreen = {"D", 0, (u16*)PA_DrawBg[0], 256, 192};

	VirScreen  Titlebar     = {  0,   0, 256,  16, {{0,0},{0,0}}, &UpScreen}; InitVS(&Titlebar);
	VirScreen  ContentWin1  = {  0,  16, 256, 176, {{0,0},{0,0}}, &UpScreen}; InitVS(&ContentWin1);
	VirScreen  ContentWin2  = {  0,   0, 256, 176, {{0,0},{0,0}}, &DnScreen}; InitVS(&ContentWin2);
	VirScreen  Statusbar    = {  0, 176, 256,  16, {{0,0},{0,0}}, &DnScreen}; InitVS(&Statusbar);
	VirScreen  Searchbar    = { 47,  37, 162,  22, {{0,0},{0,0}}, &DnScreen}; InitVS(&Searchbar);

	CharStat   TitlebarCS   = { PA_RGB(31,31,31), PA_RGB( 0, 0, 0),   HARDWRAP, DEG0, NONE, 1, 1, 0, &terminus12p};
	CharStat   ContentCS    = { PA_RGB( 0, 0, 0), PA_RGB(21,21,21), NORMALWRAP, DEG0, NONE, 0, 0, 0, &terminus12p};
	CharStat   StatusbarCS  = { PA_RGB( 5, 5, 5), PA_RGB( 0, 0, 0),   HARDWRAP, DEG0, NONE, 1, 1, 0, &terminus12p};

	FillVS(&Titlebar, PA_RGB( 9,16,28));
	FillVS(&Statusbar,PA_RGB(26,26,26));

	int nletter = 0;
	int numOut  = 0;
	int offset  = 0;
	ArticleSearchResult* suchergebnis = NULL;
	ArticleSearchResult* redirection  = NULL;

	char*  markup          = NULL;
	char*  temptext        = NULL;
	char   suchtitel[1001] = "";

	// start of main program

	FillVS(&Statusbar,PA_RGB(26,26,26));
	iPrint("Lade dewiki.dat...",&Statusbar,&StatusbarCS,2,2,-1,UTF8);
	TitleIndex* titleIndex = new TitleIndex("dewiki.dat","dewiki.idx");
	PA_Sleep(30);

	FillVS(&Statusbar,PA_RGB(26,26,26));
	sprintf(temptext,"%d Artikel",titleIndex->NumberOfArticles());
	iPrint(temptext,&Statusbar,&StatusbarCS,2,2,-1,UTF8);
	PA_Sleep(60);

	FillVS(&Statusbar,PA_RGB(26,26,26));
	iPrint("Initialisiere MarkupGetter...",&Statusbar,&StatusbarCS,2,2,-1,UTF8);
	WikiMarkupGetter* mg = new WikiMarkupGetter("de");
	PA_Sleep(60);


	u8 updateTitle     = 0;
	u8 updateContent   = 0;
	u8 updateStatusbar = 0;
	u8 loadArticle     = 1;

	while(1)
	{
		if (loadArticle)
		{
			titleIndex->DeleteSearchResult(suchergebnis);

			if (strlen(suchtitel)==0)
			{
				FillVS(&Statusbar,PA_RGB(26,26,26));
				iPrint("Suche zufälligen Artikel...",&Statusbar,&StatusbarCS,2,2,-1,UTF8);
				suchergebnis = titleIndex->GetRandomArticle();
			}
			else
			{
				FillVS(&Statusbar,PA_RGB(26,26,26));
				iPrint("Suche Artikel...",&Statusbar,&StatusbarCS,2,2,-1,UTF8);
				suchergebnis = titleIndex->FindArticle(suchtitel);
			}
			PA_Sleep(30);

			if (suchergebnis!=NULL)
			{
				FillVS(&Statusbar,PA_RGB(26,26,26));
				iPrint("Hole Markup...",&Statusbar,&StatusbarCS,2,2,-1,UTF8);
				free(markup);
				markup = mg->GetMarkupForArticle(suchergebnis);

				if (redirection = titleIndex->isRedirect(markup))
				{
					free(markup);
					ArticleSearchResult* temp = suchergebnis;
					suchergebnis = redirection;
					FillVS(&Statusbar,PA_RGB(26,26,26));
					iPrint("Folge Umleitung...",&Statusbar,&StatusbarCS,2,2,-1,UTF8);
					titleIndex->DeleteSearchResult(temp);
					markup = mg->GetMarkupForArticle(suchergebnis);
				}
				FillVS(&Statusbar,PA_RGB(26,26,26));
				offset = 0;
				updateTitle = 1;
				updateContent = 1;
			}
			else
			{
				FillVS(&Statusbar,PA_RGB(26,26,26));
				iPrint("Artikel nicht gefunden...",&Statusbar,&StatusbarCS,2,2,-1,UTF8);
				PA_Sleep(60);
				FillVS(&Statusbar,PA_RGB(26,26,26));
			}

			loadArticle = 0;
		}

		if (Pad.Newpress.Right||Pad.Held.Right)
		{
			offset += numOut;
			if (offset>=suchergebnis->ArticleLength())
				offset = 0;
			updateContent = 1;
			PA_Sleep(10);
		}

		if ((Pad.Newpress.Left||Pad.Held.Left) && (offset>0))
		{
			offset -= numOut; // TODO
			if (offset<0)
				offset = 0;
			updateContent = 1;
			PA_Sleep(10);
		}

		if (Pad.Newpress.X)
		{
			PA_KeyboardIn(24, 72);
			PA_Clear16bitBg(1);
			PA_Clear16bitBg(0);

			ArticleSearchResult* suggestions = NULL;
			char letter = 0;
			u8 updateSearchbar   = 1;
			u8 updateSuggestions = 0;
			u8 searchSuggestions = 0;
			u8 countdown         = 0;

			while(1)
			{
				letter = PA_CheckKeyboard();

				if (letter > 31) { // there is a new letter
					suchtitel[nletter++] = letter;
					suchtitel[nletter  ] = '\0';
					updateSearchbar = 1;
				}

				if ((letter == PA_BACKSPACE) && nletter)
				{
					suchtitel[--nletter] = '\0'; // Erase the last letter
					updateSearchbar = 1;
				}

				if ( (letter == '\n') )
				{
					loadArticle = 1;
					break;
				}

				if (updateSearchbar)
				{
					FillVS(&Searchbar,PA_RGB(28,28,28));
					iPrint(suchtitel,&Searchbar,&ContentCS,5,5,-1,UTF8);
					countdown = 180;
					updateSearchbar = 0;
				}

				if (Pad.Newpress.X)
				{
					break;
				}

				if (searchSuggestions)
				{
					suggestions = titleIndex->GetSuggestions(suchtitel,21);
					updateSuggestions = 1;
					searchSuggestions = 0;
				}

				if (updateSuggestions)
				{
					FillVS(&ContentWin1,PA_RGB(31,31,31));
					u8 z = 0;
					ArticleSearchResult* temp = suggestions;
					while (temp!=NULL)
					{
						iPrint(temp->TitleInArchive(),&ContentWin1,&ContentCS,2,2+10*z,-1,UTF8);
						temp = temp->Next;
						z++;
					}
					updateSuggestions = 0;
				}

				if (countdown>0)
				{
					countdown--;
					if ((countdown==0) && nletter)
					{
						searchSuggestions = 1;
					}
				}

				PA_WaitForVBL();
			}
			PA_KeyboardOut();
			updateTitle = 1;
			updateContent = 1;
			updateStatusbar = 1;
		}

		if (Pad.Newpress.A)
		{
			loadArticle = 1;
		}

		if (updateTitle)
		{
			FillVS(&Titlebar, PA_RGB( 9,16,28));
			iPrint(suchergebnis->TitleInArchive(), &Titlebar,&TitlebarCS,2,2,-1,UTF8);
			updateTitle = 0;
		}

		if (updateContent)
		{
			FillVS(&ContentWin1,PA_RGB(31,31,31));
			numOut = iPrint(markup+offset, &ContentWin1,&ContentCS,2,2,-1,UTF8);
			FillVS(&ContentWin2,PA_RGB(31,31,31));
			iPrint(markup+offset+numOut, &ContentWin2,&ContentCS,2,2,-1,UTF8);
			updateContent = 0;
		}

		if (updateStatusbar)
		{
			FillVS(&Statusbar,PA_RGB(26,26,26));
			updateStatusbar = 0;
		}

		PA_WaitForVBL();
	}
	return 0;
}
