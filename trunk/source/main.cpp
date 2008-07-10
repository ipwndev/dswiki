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

	FontInit(&terminus12regular,ter12r);

	Device	UpScreen = {"U", 1, (u16*)PA_DrawBg[1], 256, 192};
	Device	DnScreen = {"D", 0, (u16*)PA_DrawBg[0], 256, 192};
	VirScreen   UpWin =  { 0, 0, 256, 192, {{ 0, 0},{255,191}}, Deg0, &UpScreen};
	VirScreen   DnWin =  { 0, 0, 256, 192, {{ 0, 0},{255,191}}, Deg0, &DnScreen};
	VirScreen SmDnWin =  { 32, 24, 192, 144, {{ 32, 24},{223,167}}, Deg0, &DnScreen};
	BLOCK area = {{0, 0},{255,191}};

	DrawBlock(&SmDnWin,area,PA_RGB(27,27,31),1);
	CharStat C=CS;
	C.Color=PA_RGB(0,0,0);
	iPrint("variusqua minnullal uctustristique.Nameteros.Sed ¼ von (12¾)³€ ut@ma öäüÖÄÜß                   €€€€€€€€€€€€ aliquid zyxwvutsr «¢“”nµ─·»            tractatos. Lorem     ipsum       dolor sit    amet, consectetuer       adipiscing      elit.      Nunc ac       ante sed ante imperdiet auctor. Fusce dignissim, ░ ▒ ▓ █ magna eu feugiat tincidunt, nibh metus tincidunt augue, quis ullamcorper lorem pede a ante. Proin congue nisl a arcu. Donec et elit. Etiam ac eros nec metus molestie aliquam. Nullam vestibulum molestie magna. In varius quam in nulla luctus tristique. Nam et eros. Sed vitae sem a velit mattis dapibus. Sed blandit, sapien auctor adipiscing viverra, purus urna fermentum wisi, id luctus tortor augue et ligula. In quis ¼g¼g¼g¼g¼g. Sed urna arcu, ¼€€€g¼g¼g¼g¼g in.\nΌταν ανεπιθύμητη.\nहोभरआशाआपस\nक.福建西部北方方言的明显特\nSome\nMore\nText\nTo\nGet\nTo\nThe\nBottom\nAnd\nFar\nBelowy", &SmDnWin, &C, 0, UTF8);

	while (1)
		;

	bool keyboardActive = false;
	PA_InitKeyboard(2);
	PA_KeyboardIn(24, 95);
	PA_KeyboardOut();

	ArticleSearchResult* suchergebnis = NULL;
	ArticleSearchResult* redirection  = NULL;
	char* markup = NULL;

	PA_OutputText(1,0,0,"Lade \"%c3dewiki.dat%c0\"...");
	TitleIndex* titleIndex = new TitleIndex("dewiki.dat","dewiki.idx");
	PA_OutputText(1,20,0,"%c2OK");
	PA_OutputText(1,0,1,"%c3%d%c0 Artikel",titleIndex->NumberOfArticles());
	PA_OutputText(1,0,2,"Initialisiere MarkupGetter...");
	WikiMarkupGetter* mg = new WikiMarkupGetter("de");
	PA_OutputText(1,29,2,"%c2OK");

	int count = 0;
	int zeile = 0;
	int numOut = 0;
	char text[1000] = "";
	int nletter = 0;
	char letter = 0;
	int size = 3;
	int offset = 0;


	while(1) {
		zeile = 0;
		PA_ClearTextBg(1);
// 		PA_OutputText(1,23,23,"%c1[Wait...]");
		titleIndex->DeleteSearchResult(suchergebnis);
		if (nletter==0)
		{
// 			PA_OutputText(1,0,3,"Suche zuf. Artikel...");
			suchergebnis = titleIndex->GetRandomArticle();
		}
		else
		{
// 			PA_OutputText(1,0,3,"     Suche Artikel...");
			suchergebnis = titleIndex->FindArticle(text);
		}

		if (suchergebnis!=NULL)
		{
			PA_ClearTextBg(1);
// 			PA_OutputText(1,21,3,"%c2OK");

			count++;
// 			PA_OutputText(1,0,4,"Hole Markup...");
			free(markup);
			markup = mg->GetMarkupForArticle(suchergebnis);
// 			PA_OutputText(1,14,4,"%c2OK");

			PA_OutputText(1,0,zeile++,"%c1%s",suchergebnis->Title());
			while (redirection = titleIndex->isRedirect(markup))
			{
				free(markup);
				ArticleSearchResult* temp = suchergebnis;
				suchergebnis = redirection;
				PA_OutputText(1,0,zeile++,"%c1-> %s",suchergebnis->Title());
				titleIndex->DeleteSearchResult(temp);
				markup = mg->GetMarkupForArticle(suchergebnis);
			}


// 			PA_OutputText(1,0,0,"0x%x %d", suchergebnis, count);
// 			PA_OutputText(1,0,1,"Artikel %c1%s%c0 gefunden, Daten: Laenge %c3%d%c0 Bytes, Blockposition %c20x%x%c0, rel. Pos %c2%d%c0.", suchergebnis->TitleInArchive(), suchergebnis->ArticleLength(), suchergebnis->BlockPos(), suchergebnis->ArticlePos());

			PA_Clear16bitBg(0);
			offset = 0;
			numOut = SimPrint(markup, &DnScreen,0,0,PA_RGB(0,0,0),UTF8);

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
							break;
						}

						PA_OutputSimpleText(1, 8, 11, text); // Write the text
						PA_WaitForVBL();
					}

				}
			}
			else if (Pad.Newpress.Right||Pad.Held.Right)
			{
				PA_Clear16bitBg(0);
				offset += 600;
				if (offset>=suchergebnis->ArticleLength()) offset = 0;
				PA_OutputText(1,14,22,"%d%  ",offset*100/suchergebnis->ArticleLength());
				numOut = SimPrint(markup+offset, &DnScreen,0,0,PA_RGB(0,0,0),UTF8);
			}
			else if (Pad.Newpress.Left||Pad.Held.Left)
			{
				PA_Clear16bitBg(0);
				offset -= 600;
				if (offset<0) offset = 0;
				PA_OutputText(1,14,22,"%d%  ",offset*100/suchergebnis->ArticleLength());
				numOut = SimPrint(markup+offset, &DnScreen,0,0,PA_RGB(0,0,0),UTF8);
			}
			else if (Pad.Newpress.Up||Pad.Held.Up)
			{
				PA_Clear16bitBg(0);
				offset -= 42;
				if (offset<0) offset = 0;
				PA_OutputText(1,14,22,"%d%  ",offset*100/suchergebnis->ArticleLength());
				numOut = SimPrint(markup+offset, &DnScreen,0,0,PA_RGB(0,0,0),UTF8);
			}
			else if (Pad.Newpress.Down||Pad.Held.Down)
			{
				PA_Clear16bitBg(0);
				offset += 42;
				if (offset>=suchergebnis->ArticleLength()) offset = 0;
				PA_OutputText(1,14,22,"%d%  ",offset*100/suchergebnis->ArticleLength());
				numOut = SimPrint(markup+offset, &DnScreen,0,0,PA_RGB(0,0,0),UTF8);
			}
			PA_WaitForVBL();
		}
		PA_WaitForVBL();
	}

	return 0;
}
