#include <PA9.h>

#include "main.h"
#include "chrlib.h"
#include "api.h"
#include "TitleIndex.h"
#include "WikiMarkupGetter.h"

// char blindtext[2000] = "Weit hinter den Wortbergen\n\nWeit hinten, hinter den Wortbergen, fern der Länder Vokalien und Konsonantien leben die Blindtexte. Abgeschieden wohnen Sie in Buchstabhausen an der Küste des Semantik, eines großen Sprachozeans. Ein kleines Bächlein namens Duden fließt durch ihren Ort und versorgt sie mit den nötigen Regelialien. Es ist ein paradiesmatisches Land, in dem einem gebratene Satzteile in den Mund fliegen. Nicht einmal von der allmächtigen Interpunktion werden die Blindtexte beherrscht - ein geradezu unorthographisches Leben. Eines Tages aber beschloss eine kleine Zeile Blindtext, ihr Name war Lorem Ipsum, hinaus zu gehen in die weite Grammatik. Der große Oxmox riet ihr davon ab, da es dort wimmele von bösen Kommata, wilden Fragezeichen und hinterhältigen Semikoli, doch das Blindtextchen ließ sich nicht beirren. Es packte seine sieben Versalien, schob sich sein Initial in den Gürtel und machte sich auf den Weg. kkhsdklhdksh dksjdhksjdhksjjhkshkshd\n\n z";
// char blindtext[2000] = "Weit hinter den Wortbergen\n\nWeit hinten, hinter den Wortbergen, fern der Länder Vokalien und Konsonantien leben die Blindtexte. Abgeschieden wohnen Sie in Buchstabhausen an der Küste des Semantik, eines großen Sprachozeans. Ein kleines öffentlöches Klosett namens Duden fließt durch ihren Ort und versorgt sie mit den nötigen Regelialien. Es ist ein paradiesmatisches Land, in dem einem gebratene Satzteile in den Mund fliegen. Nicht einmal von der allmächtigen Interpunktion werden die Blindtexte beherrscht - ein geradezu unorthographisches Leben. Eines Tages aber beschloss eine kleine Zeile Blindtext, ihr Name war Lorem Ipsum, hinaus zu gehen in die weite Grammatik. Der große Oxmox riet ihr davon ab, da es dort wimmele von bösen Kommata, wilden Fragezeichen und hinterhältigen Semikoli, doch das Blindtextchen ließ sich nicht beirren. Es packte seine sieben Versalien, schob sich sein Initial in den Gürtel und machte sich auf den Weg. kkhsdklhdksh dksjdhksjdhksjjhkshkshd\n\n z";
char* blindtexte[300] = {
  "", // 0
  "Test", // 1
  "Zwei Worte", // 2
  "Zeile 1\nZeile 2\nZeile 3", // 3
  "\n\nHier fangen wir in Zeile 3 an", // 4
  "\n\nÜUbler Anfang mit Umlaut in Zeile 3", // 5
  "\n\nÜÜbler Anfang mit Umlaut in Zeile 3", // 6
  "\n\nUÜbler Anfang mit Umlaut in Zeile 3", // 7
  "\nHier kommt eine einzige überlange Zeile die auf jeden Fall für einen normalen Umbruch ins nächste Feld sorgen soll (nur ein Leerzeichen)", // 8
  "\nHier kommt eine weitere überlange Zeile           die auf jeden Fall für einen normalen Umbruch ins nächste Feld sorgen soll (mehrere Leerzeichen)", // 9
  "\nHierkommteineweitereüberlangeZeiledieaufjedenFallfüreinenerzwungenenUmbruchinsnächsteFeldsorgensoll", // 10
  "\nHierkommteineweitereüberlangeZeiledieaufj€denFallfüreinenerzwungenenUmbruchinsnächsteFeldsorgensoll", // 11
  "\nHierkommteineweitereüberlangeZeiledieaufjedênFallfüreinenerzwungenenUmbruchinsnächsteFeldsorgensoll", // 12
  "\nHierkommteineweitereüberlangeZeiledieaufj€dênFallfüreinenerzwungenenUmbruchinsnächsteFeldsorgensoll", // 13
  "\nHierkommteineweitereüberlangeZeiledieaufjeđenFallfüreinenerzwungenenUmbruchinsnächsteFeldsorgensoll", // 14
  "\nHierkommteineweitereüberlangeZeiledieaufj€đenFallfüreinenerzwungenenUmbruchinsnächsteFeldsorgensoll", // 15
  "\nHierkommteineweitereüberlangeZeiledieaufjeđênFallfüreinenerzwungenenUmbruchinsnächsteFeldsorgensoll", // 16
  "\nHierkommteineweitereüberlangeZeiledieaufj€đênFallfüreinenerzwungenenUmbruchinsnächsteFeldsorgensoll", // 17
  "Test ", // 18
  "Test  ", // 19
  "Test                                                    ", // 20
  "Test \n", // 21
  "Test  \n", // 22
  "Test                                                    \n", // 23
  "Test \n Weiter", // 24
  "Test  \n  Weiter", // 25
  "Test                                                    \n              Weiter", // 26
  "Test                                                    \n                                                                 Weiter", // 27
};

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

	VirScreen   UpWin =  { 0,  0, 256, 192, {{0,0},{0,0}}, Deg0, &UpScreen}; InitVS(&UpWin);
	VirScreen   DnWin =  { 0,  0, 256, 192, {{0,0},{0,0}}, Deg0, &DnScreen}; InitVS(&DnWin);
	VirScreen   Z1Win =  { 0,  0, 256,  30, {{0,0},{0,0}}, Deg0, &DnScreen}; InitVS(&Z1Win);
	VirScreen   Z2Win =  { 0, 96, 256,  96, {{0,0},{0,0}}, Deg0, &DnScreen}; InitVS(&Z2Win);

	CharStat NormalCS = { PA_RGB(0,0,0), PA_RGB(31,31,31), 0, Deg0, NONE, 0, 0, 0, &terminus12regular};
	CharStat IncLHCS = { PA_RGB(0,0,0), PA_RGB(31,31,31), 0, Deg0, NONE, 0, 0, 3, &terminus12regular};

	u32 num;
	int i;
	SimPrint("Hallo@Maus",&UpScreen,0,0,PA_RGB(31,0,31),UTF8);
	for (i=0;i<28;i++) {
		PA_ClearTextBg(1);
		FillVS(&Z1Win,PA_RGB(20,31,20));
		FillVS(&Z2Win,PA_RGB(31,20,20));
		num = iPrint( blindtexte[i],      &Z1Win, &NormalCS, 0, UTF8);
		PA_OutputText(1, 0,  0, "%d", num);
		num = iPrint(&blindtexte[i][num], &Z2Win, &NormalCS, 0, UTF8);
		PA_OutputText(1, 0, 12, "%d", num);
		PA_WaitFor(Pad.Newpress.A);
	}

	while (1)
		PA_WaitForVBL();

// 	bool keyboardActive = false;
// 	PA_InitKeyboard(2);
// 	PA_KeyboardIn(24, 95);
// 	PA_KeyboardOut();
//
// 	ArticleSearchResult* suchergebnis = NULL;
// 	ArticleSearchResult* redirection = NULL;
// 	char* markup = NULL;
//
// 	PA_OutputText(1,0,0,"Lade \"%c3dewiki.dat%c0\"...");
// 	TitleIndex* titleIndex = new TitleIndex("dewiki.dat","dewiki.idx");
// 	PA_OutputText(1,20,0,"%c2OK");
// 	PA_OutputText(1,0,1,"%c3%d%c0 Artikel",titleIndex->NumberOfArticles());
// 	PA_OutputText(1,0,2,"Initialisiere MarkupGetter...");
// 	WikiMarkupGetter* mg = new WikiMarkupGetter("de");
// 	PA_OutputText(1,29,2,"%c2OK");
//
// 	int count = 0;
// 	int zeile = 0;
// 	int numOut = 0;
// 	char text[1000] = "";
// 	int nletter = 0;
// 	char letter = 0;
// 	int size = 3;
// 	int offset = 0;
//
//
// 	while(1) {
// 		zeile = 0;
// 		PA_ClearTextBg(1);
// // 		PA_OutputText(1,23,23,"%c1[Wait...]");
// 		titleIndex->DeleteSearchResult(suchergebnis);
// 		if (nletter==0)
// 		{
// // 			PA_OutputText(1,0,3,"Suche zuf. Artikel...");
// 			suchergebnis = titleIndex->GetRandomArticle();
// 		}
// 		else
// 		{
// // 			PA_OutputText(1,0,3,"     Suche Artikel...");
// 			suchergebnis = titleIndex->FindArticle(text);
// 		}
//
// 		if (suchergebnis!=NULL)
// 		{
// 			PA_ClearTextBg(1);
// // 			PA_OutputText(1,21,3,"%c2OK");
//
// 			count++;
// // 			PA_OutputText(1,0,4,"Hole Markup...");
// 			free(markup);
// 			markup = mg->GetMarkupForArticle(suchergebnis);
// // 			PA_OutputText(1,14,4,"%c2OK");
//
// 			PA_OutputText(1,0,zeile++,"%c1%s",suchergebnis->Title());
// 			while (redirection = titleIndex->isRedirect(markup))
// 			{
// 				free(markup);
// 				ArticleSearchResult* temp = suchergebnis;
// 				suchergebnis = redirection;
// 				PA_OutputText(1,0,zeile++,"%c1-> %s",suchergebnis->Title());
// 				titleIndex->DeleteSearchResult(temp);
// 				markup = mg->GetMarkupForArticle(suchergebnis);
// 			}
//
//
// // 			PA_OutputText(1,0,0,"0x%x %d", suchergebnis, count);
// // 			PA_OutputText(1,0,1,"Artikel %c1%s%c0 gefunden, Daten: Laenge %c3%d%c0 Bytes, Blockposition %c20x%x%c0, rel. Pos %c2%d%c0.", suchergebnis->TitleInArchive(), suchergebnis->ArticleLength(), suchergebnis->BlockPos(), suchergebnis->ArticlePos());
//
// 			PA_Clear16bitBg(0);
// 			offset = 0;
// 			numOut = SimPrint(markup, &DnScreen,0,0,PA_RGB(0,0,0),UTF8);
//
// 		}
// 		else
// 		{
// // 			PA_OutputText(1,21,3,"%c1Fehler");
// 		}
//
// 		while (!Pad.Newpress.A)
// 		{
// 			if (Pad.Newpress.X)
// 			{
// 				if (keyboardActive)
// 				{
// 					keyboardActive = false;
// 					PA_KeyboardOut();
// 				}
// 				else {
// 					keyboardActive = true;
// 					PA_KeyboardIn(24, 95);
//
// 					while(1) {
// 						letter = PA_CheckKeyboard();
//
// 						if (letter > 31) { // there is a new letter
// 							text[nletter] = letter;
// 							nletter++;
// 						}
// 						else if(letter == PA_TAB){// TAB Pressed...
// 							u8 i;
// 							for (i = 0; i < 4; i++){ // put 4 spaces...
// 								text[nletter] = ' ';
// 								nletter++;
// 							}
//
// 						}
// 						else if ((letter == PA_BACKSPACE)&&nletter) { // Backspace pressed
// 							nletter--;
// 							text[nletter] = ' '; // Erase the last letter
// 						}
// 						else if ((letter == '\n') || Pad.Newpress.X ) { // Enter pressed
// 							text[nletter] = '\0';
// 							keyboardActive = false;
// 							PA_KeyboardOut();
// 							break;
// 						}
//
// 						PA_OutputSimpleText(1, 8, 11, text); // Write the text
// 						PA_WaitForVBL();
// 					}
//
// 				}
// 			}
// 			else if (Pad.Newpress.Right||Pad.Held.Right)
// 			{
// 				PA_Clear16bitBg(0);
// 				offset += 600;
// 				if (offset>=suchergebnis->ArticleLength()) offset = 0;
// 				PA_OutputText(1,14,22,"%d%  ",offset*100/suchergebnis->ArticleLength());
// 				numOut = SimPrint(markup+offset, &DnScreen,0,0,PA_RGB(0,0,0),UTF8);
// 			}
// 			else if (Pad.Newpress.Left||Pad.Held.Left)
// 			{
// 				PA_Clear16bitBg(0);
// 				offset -= 600;
// 				if (offset<0) offset = 0;
// 				PA_OutputText(1,14,22,"%d%  ",offset*100/suchergebnis->ArticleLength());
// 				numOut = SimPrint(markup+offset, &DnScreen,0,0,PA_RGB(0,0,0),UTF8);
// 			}
// 			else if (Pad.Newpress.Up||Pad.Held.Up)
// 			{
// 				PA_Clear16bitBg(0);
// 				offset -= 42;
// 				if (offset<0) offset = 0;
// 				PA_OutputText(1,14,22,"%d%  ",offset*100/suchergebnis->ArticleLength());
// 				numOut = SimPrint(markup+offset, &DnScreen,0,0,PA_RGB(0,0,0),UTF8);
// 			}
// 			else if (Pad.Newpress.Down||Pad.Held.Down)
// 			{
// 				PA_Clear16bitBg(0);
// 				offset += 42;
// 				if (offset>=suchergebnis->ArticleLength()) offset = 0;
// 				PA_OutputText(1,14,22,"%d%  ",offset*100/suchergebnis->ArticleLength());
// 				numOut = SimPrint(markup+offset, &DnScreen,0,0,PA_RGB(0,0,0),UTF8);
// 			}
// 			PA_WaitForVBL();
// 		}
// 		PA_WaitForVBL();
// 	}

	return 0;
}
