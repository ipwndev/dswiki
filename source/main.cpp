#include <PA9.h>

#include "main.h"
#include "chrlib.h"
#include "api.h"
#include "TitleIndex.h"
#include "WikiMarkupGetter.h"

int main(int argc, char ** argv)
{
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

	FontInit(&terminus12regular,ter12rp);

	Device	UpScreen = {"U", 1, (u16*)PA_DrawBg[1], 256, 192};
	Device	DnScreen = {"D", 0, (u16*)PA_DrawBg[0], 256, 192};

	VirScreen  Titlebar     = { 2,   2, 252,  12, {{0,0},{0,0}}, &UpScreen}; InitVS(&Titlebar);
	VirScreen  TitlebarD    = { 0,   0, 256,  16, {{0,0},{0,0}}, &UpScreen}; InitVS(&TitlebarD);
	VirScreen  ContentWin1  = { 2,  20, 252, 168, {{0,0},{0,0}}, &UpScreen}; InitVS(&ContentWin1);
	VirScreen  ContentWin2  = { 2,   4, 252, 168, {{0,0},{0,0}}, &DnScreen}; InitVS(&ContentWin2);
	VirScreen  Statusbar    = { 2, 178, 252,  12, {{0,0},{0,0}}, &DnScreen}; InitVS(&Statusbar);
	VirScreen  StatusbarD   = { 0, 176, 256,  16, {{0,0},{0,0}}, &DnScreen}; InitVS(&StatusbarD);

	CharStat   TitlebarCS   = { PA_RGB(31,31,31), PA_RGB( 0, 0, 0),   HARDWRAP, DEG0, NONE, 0, 1, 0, &terminus12regular};
	CharStat   ContentCS    = { PA_RGB( 0, 0, 0), PA_RGB( 0, 0, 0), NORMALWRAP, DEG0, NONE, 0, 0, 0, &terminus12regular};
	CharStat   StatusbarCS  = { PA_RGB( 5, 5, 5), PA_RGB( 0, 0, 0),   HARDWRAP, DEG0, NONE, 0, 1, 0, &terminus12regular};

	FillVS(&TitlebarD, PA_RGB( 9,16,28));
	FillVS(&StatusbarD,PA_RGB(26,26,26));

	bool keyboardActive = false;
	PA_InitKeyboard(2);
	PA_KeyboardIn(24, 95);
	PA_KeyboardOut();

	ArticleSearchResult* suchergebnis = NULL;
	ArticleSearchResult* redirection = NULL;
	char* markup = NULL;

	FillVS(&Statusbar,PA_RGB(26,26,26));
	iPrint("Lade \"dewiki.dat\"...",&Statusbar,&StatusbarCS,PA_RGB(5,5,5),UTF8);
	TitleIndex* titleIndex = new TitleIndex("dewiki.dat","dewiki.idx");
	iPrint("Lade \"dewiki.dat\"...OK!",&Statusbar,&StatusbarCS,PA_RGB(5,5,5),UTF8);

	FillVS(&Statusbar,PA_RGB(26,26,26));
	iPrint("Initialisiere MarkupGetter...",&Statusbar,&StatusbarCS,PA_RGB(5,5,5),UTF8);
	WikiMarkupGetter* mg = new WikiMarkupGetter("de");
	iPrint("Initialisiere MarkupGetter...OK!",&Statusbar,&StatusbarCS,PA_RGB(5,5,5),UTF8);

	int count = 0;
	int zeile = 0;
	int numOut = 0;
	char text[1000] = "";
	int nletter = 0;
	char letter = 0;
	int size = 3;
	int offset = 0;


	while(1) {
		FillVS(&Statusbar,PA_RGB(26,26,26));
		iPrint("Bitte warten...",&Statusbar,&StatusbarCS,PA_RGB(5,5,5),UTF8);
		titleIndex->DeleteSearchResult(suchergebnis);
		if (nletter==0)
		{
			FillVS(&Statusbar,PA_RGB(26,26,26));
			iPrint("Suche zufälligen Artikel...",&Statusbar,&StatusbarCS,PA_RGB(5,5,5),UTF8);
			suchergebnis = titleIndex->GetRandomArticle();
		}
		else
		{
			FillVS(&Statusbar,PA_RGB(26,26,26));
			iPrint("Suche Artikel...",&Statusbar,&StatusbarCS,PA_RGB(5,5,5),UTF8);
			suchergebnis = titleIndex->FindArticle(text);
		}

		if (suchergebnis!=NULL)
		{
			count++;
			FillVS(&Statusbar,PA_RGB(26,26,26));
			iPrint("Hole Markup...",&Statusbar,&StatusbarCS,PA_RGB(5,5,5),UTF8);
			free(markup);
			markup = mg->GetMarkupForArticle(suchergebnis);

			while (redirection = titleIndex->isRedirect(markup))
			{
				free(markup);
				ArticleSearchResult* temp = suchergebnis;
				suchergebnis = redirection;
				FillVS(&Statusbar,PA_RGB(26,26,26));
				iPrint("Folge Umleitung...",&Statusbar,&StatusbarCS,PA_RGB(5,5,5),UTF8);
				titleIndex->DeleteSearchResult(temp);
				markup = mg->GetMarkupForArticle(suchergebnis);
			}

			FillVS(&Titlebar,PA_RGB(9,16,28));
			iPrint(suchergebnis->Title(),&Titlebar,&TitlebarCS,-1,UTF8);
			FillVS(&Statusbar,PA_RGB(26,26,26));
			offset = 0;
			FillVS(&ContentWin1,PA_RGB(31,31,31));
			numOut = iPrint(markup, &ContentWin1,&ContentCS,PA_RGB(0,0,0),UTF8);
			FillVS(&ContentWin2,PA_RGB(31,31,31));
			numOut += iPrint(markup+numOut, &ContentWin2,&ContentCS,PA_RGB(0,0,0),UTF8);

		}
		else
		{
// 			PA_OutputText(1,21,3,"%c1Fehler");
		}

		while (!Pad.Newpress.A)
		{
			if (Pad.Newpress.X)
			{
				if (keyboardActive)
				{
					keyboardActive = false;
					PA_KeyboardOut();
				}
				else {
					keyboardActive = true;
					PA_KeyboardIn(24, 95);

					while(1) {
						letter = PA_CheckKeyboard();

						if (letter > 31) { // there is a new letter
							text[nletter] = letter;
							nletter++;
						}
						else if(letter == PA_TAB){// TAB Pressed...
							u8 i;
							for (i = 0; i < 4; i++){ // put 4 spaces...
								text[nletter] = ' ';
								nletter++;
							}

						}
						else if ((letter == PA_BACKSPACE)&&nletter) { // Backspace pressed
							nletter--;
							text[nletter] = ' '; // Erase the last letter
						}
						else if ((letter == '\n') || Pad.Newpress.X ) { // Enter pressed
							text[nletter] = '\0';
							keyboardActive = false;
							PA_KeyboardOut();
							PA_ClearTextBg(1);
							break;
						}

						PA_OutputSimpleText(1, 8, 11, text); // Write the text
						PA_WaitForVBL();
					}

				}
			}
			else if (Pad.Newpress.Right||Pad.Held.Right)
			{
				offset += numOut;
				if (offset>=suchergebnis->ArticleLength())
					offset = 0;
				FillVS(&ContentWin1,PA_RGB(31,31,31));
				numOut = iPrint(markup+offset, &ContentWin1,&ContentCS,PA_RGB(0,0,0),UTF8);
				FillVS(&ContentWin2,PA_RGB(31,31,31));
				numOut += iPrint(markup+offset+numOut, &ContentWin2,&ContentCS,PA_RGB(0,0,0),UTF8);
				PA_WaitForVBL();PA_WaitForVBL();PA_WaitForVBL();PA_WaitForVBL();PA_WaitForVBL();PA_WaitForVBL();
				PA_WaitForVBL();PA_WaitForVBL();PA_WaitForVBL();PA_WaitForVBL();PA_WaitForVBL();PA_WaitForVBL();
			}
		}
		PA_WaitForVBL();
	}

	return 0;
}
