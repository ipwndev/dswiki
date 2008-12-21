#include "Markup.h"

#include <PA9.h>
#include <string>
#include <vector>
#include "char_convert.h"
#include "main.h"
#include "Globals.h"
#include "PercentIndicator.h"
#include "WIKI2XML.h"
// #include "tinyxml.h"

using namespace std;

Element::~Element()
{
}

Element::Element(string text)
{
	Type = TEXT_EL;
	displayText = text;
}


Element::Element(string str_namesp, string str_targ, string str_anch, string str_disp, unsigned int start, unsigned int length, unsigned int link_id)
{
	Type = LINK_EL;

	if ((str_namesp == "Image") || (str_namesp == "Bild")) // TODO
	{
		Type = IMG_EL;
	}

	wikiNamespace = str_namesp;
	target        = str_targ;
	anchor        = str_anch;
	displayText   = str_disp;

	sourcePositionStart = start;
	sourceLength        = length;
	id                  = link_id;
}


Element* createLink(string Str, unsigned int startPos, unsigned int link_id)
{
	while (1)
	{
		string nameStr = "";
		string targStr = "";
		string anchStr = "";
		string dispStr = "";

		if (startPos>=Str.length())
			break;

		int linkstart = Str.find("[[",startPos);
		if (linkstart==string::npos)
			break;

		startPos = linkstart+2;

		int linkend = Str.find("]]",startPos);
		if (linkend==string::npos)
			break;

		string linkinnertext = Str.substr(linkstart+2,linkend-linkstart-2);
		if (linkinnertext.empty())
			continue;

		vector<string> pipeSeperatedContent;

		int stelleStart = 0;
		int stelleEnd;

		// split at each pipe symbol
		while ( (stelleEnd=linkinnertext.find("|",stelleStart)) != string::npos)
		{
			if (stelleEnd>stelleStart)
			{
				pipeSeperatedContent.push_back(linkinnertext.substr(stelleStart,stelleEnd-stelleStart));
			}
			stelleStart = stelleEnd+1;
		}
		if (stelleStart<linkinnertext.length())
		{
			pipeSeperatedContent.push_back(linkinnertext.substr(stelleStart));
		}

		if ((stelleStart=pipeSeperatedContent.front().find(":")) != string::npos) // potential namespace
		{
			nameStr = pipeSeperatedContent.front().substr(0,stelleStart);
			pipeSeperatedContent.front().erase(0,stelleStart+1);
		}

		if ((stelleStart=pipeSeperatedContent.front().find("#")) != string::npos) // anchor
		{
			anchStr = pipeSeperatedContent.front().substr(stelleStart+1);
			pipeSeperatedContent.front().resize(stelleStart);
		}

		targStr = pipeSeperatedContent.front();

		if ((nameStr == "Image") || (nameStr == "Bild")) // TODO
		{   // handle things differently
			dispStr = pipeSeperatedContent.back();
		}
		else
		{
			if (pipeSeperatedContent.size() > 1)
			{
				dispStr = pipeSeperatedContent[1];
				int i;
				for (i=2;i<pipeSeperatedContent.size();i++)
					dispStr += "|"+pipeSeperatedContent[i];
			}
			else
			{
				if (nameStr.empty())
					dispStr = targStr;
				else
					dispStr = nameStr+":"+targStr;
			}
		}

/*		int i;
		for (i=0;i<pipeSeperatedContent.size();i++)
		PA_OutputText(1,5,i,"->%s<-",pipeSeperatedContent[i].c_str());*/

// 		PA_ClearTextBg(1);
// 		PA_OutputText(1,5,20,"->%s<-",nameStr.c_str());
// 		PA_OutputText(1,5,21,"->%s<-",targStr.c_str());
// 		PA_OutputText(1,5,22,"->%s<-",anchStr.c_str());
// 		PA_OutputText(1,5,23,"->%s<-",dispStr.c_str());


		if (targStr.find_first_of("[]<>{}\n")!=string::npos)
			continue;

		nameStr = trimPhrase(nameStr);
		dispStr = trimPhrase(dispStr);
		targStr = trimPhrase(targStr);
		anchStr = trimPhrase(anchStr);

		while ((stelleStart=targStr.find("_"))!=string::npos)
		{
			targStr.replace(stelleStart,1," ");
		}

		// collect and append any suffix and spaces
		stelleStart = Str.find_first_of(" \n.,;?:|<>(){}[]!'\"=",linkend+2);
		stelleEnd = Str.find_first_not_of(" ",stelleStart);
		dispStr.append(Str.substr(linkend+2,stelleEnd-linkend-2));

		return new Element(nameStr,targStr,anchStr,dispStr,linkstart,(linkend+2)+Str.substr(linkend+2,stelleEnd-(linkend+2)).length()-linkstart, link_id);
	}

	return NULL;
}

Markupline::~Markupline()
{
	children.clear();
}

void Markupline::drawToVScreen(VirScreen* VScreen, CharStat* CStat, int line)
{
	BLOCK CharArea = {{0,line*(CStat->FONT->Regular.Height+CStat->H_Space)},{0,0}}; // TODO: switch(Rotate)
	int i;
	for (i=0;i<children.size();i++)
	{
		switch (children[i].Type)
		{
			case TEXT_EL:
				CStat->Color = _globals->textColor();
				break;
			case LINK_EL:
				CStat->Color = _globals->linkColor();
				break;
			case IMG_EL:
				CStat->Color = PA_RGB(24,0,24);
				break;
		}
		children[i].BoundingBox.Start.x = CharArea.Start.x;                   // TODO: Rotate & Screen-absolut machen
		children[i].BoundingBox.Start.y = CharArea.Start.y;                   // TODO: Rotate
		iPrint(children[i].displayText,VScreen,CStat,&CharArea,-1,UTF8);
		children[i].BoundingBox.End.x = CharArea.Start.x;                     // TODO: Rotate
		children[i].BoundingBox.End.y = CharArea.Start.y+CStat->FONT->Regular.Height; // TODO: Rotate
	}
}
unsigned char Markupline::containsCertainLink(unsigned int id)
{
	int i;
	for (i=0;i<children.size();i++)
	{
		if (children[i].Type == LINK_EL)
		{
			Element l = children[i];
			if (l.id == id)
				return 1;
		}
	}
	return 0;
}

string Markup::evaluateClick(s16 x,s16 y)
{
	POINT p = {x,y};
	unsigned short int zeile = y / ( _markupCStat->FONT->Regular.Height + _markupCStat->H_Space );
	Markupline ClickedLine;
	Element* l;
	int i;
	if (((zeile+_currentLine) >= 0) && ((zeile+_currentLine) < numberOfLines()))
	{
		ClickedLine = lines[zeile+_currentLine];
		for (i=0;i<ClickedLine.children.size();i++)
		{
			if (IsInArea(ClickedLine.children[i].BoundingBox, p))
			{
				if (ClickedLine.children[i].Type == LINK_EL)
				{
					l = &ClickedLine.children[i];
// 					PA_OutputText(0,0,23,"%s",l->target.c_str());
					return trimPhrase(l->target);
				}
			}
		}
	}
	return "";
}


Markup::Markup()
{
	_td = NULL;
	TiXmlBase::SetCondenseWhiteSpace( false );
}


void Markup::parse(string & Str)
{
	PA_ClearTextBg(1);
	_loadOK = false;

	_markupCStat    = &ContentCS;
	_linesOnVScreen1 = 1 + ( ( ContentWin1.Height - _markupCStat->FONT->Regular.Height ) / ( _markupCStat->FONT->Regular.Height + _markupCStat->H_Space ) );
	_linesOnVScreen2 = 1 + ( ( ContentWin2.Height - _markupCStat->FONT->Regular.Height ) / ( _markupCStat->FONT->Regular.Height + _markupCStat->H_Space ) );
	_currentLine = 0;

	unsigned int pos = 0;
	unsigned int link_id = 0;
	Element* l;

// 	PA_OutputText(1,0,4,"wiki2xml");
	WIKI2XML* w2x = new WIKI2XML(Str);
// 	PA_OutputText(1,0,4,"wiki2xml [Init OK]   ");
	w2x->parse();
// 	PA_OutputText(1,0,4,"wiki2xml [Parsing OK]");
	w2x->get_xml();
// 	PA_OutputText(1,0,4,"wiki2xml [getXML OK] ");

	if (w2x != NULL)
	{
		delete w2x;
		w2x = NULL;
	}

	if (_td != NULL)
	{
		delete _td;
		_td = NULL;
	}

// 	PA_OutputText(1,0,5,"TinyXML-FAT-Parsing: new");
	_td = new TiXmlDocument("fat:/dswiki/article.xml");
// 	PA_OutputText(1,0,5,"TinyXML-FAT-Parsing: LoadFile");
	_loadOK = _td->LoadFile();

	if ( _loadOK && (!_td->Error()) )
	{
// 		PA_OutputText(1,0,5,"%c2TinyXML-FAT-Parsing OK");
		_td->SaveFile("fat:/dswiki/article.tiny.xml");
		FILE* f = fopen("fat:/dswiki/article.tiny.xml","rb");
		if (f != NULL)
		{
			fseek(f, 0, SEEK_END);
			int size = ftell(f);
			fseek(f, 0, SEEK_SET);
			char* buffer = (char*)malloc(size+1);
			fread(buffer, 1, size, f);
			buffer[size] = '\0';
			fclose(f);
			Str = buffer;
		}

	}
	else
	{
// 		PA_OutputText(1,0,5,"%c1TinyXML-FAT-Error %d at (%d/%d)",_td->ErrorId(),_td->ErrorRow(),_td->ErrorCol());
// 		PA_OutputText(1,0,6,"%s",_td->ErrorDesc());
		FILE* tinyerror = fopen("fat:/dswiki/article.tiny.xml","w");
		if (tinyerror!=NULL)
		{
			fprintf(tinyerror,"TinyXML-FAT-Error %d at (%d/%d)\n",_td->ErrorId(),_td->ErrorRow(),_td->ErrorCol());
			fprintf(tinyerror,"%s\n",_td->ErrorDesc());
			fclose(tinyerror);
		}
	}

// 	PA_Sleep(300);
	PA_ClearTextBg(1);

// 	Str.clear();
	pos = 0;
	l = createLink(Str,pos,link_id++);

	while (l)
	{
		Element t(Str.substr(pos,l->sourcePositionStart-pos));
		visibleChildren.push_back(t);

		visibleChildren.push_back(*l);

		pos = l->sourcePositionStart + l->sourceLength;
		delete l;

		l = createLink(Str,pos,link_id++);
	}

	Element t(Str.substr(pos));
	visibleChildren.push_back(t);

}

Markup::~Markup()
{
	visibleChildren.clear();
	lines.clear();
// 	delete _td;
// 	_td = NULL;
}

void Markup::createLines(VirScreen* VScreen, CharStat* CStat)
{
	VirScreen FakeVS = { VScreen->Left, VScreen->Top, VScreen->Width, VScreen->Height, {{0,0},{0,0}}, VScreen->Screen};
	CharStat  FakeCS = { CStat->FONT, REGULAR, CStat->W_Space, CStat->H_Space, CStat->Color, CStat->FxColor, CStat->BgColor, CStat->Rotate, CStat->Wrap, SIMULATE, 0};
	switch (CStat->Rotate)
	{
		case DEG0:
		case DEG180:
			FakeVS.Height = FakeCS.FONT->Regular.Height;
			break;
		case DEG90:
		case DEG270:
			FakeVS.Width = FakeCS.FONT->Regular.Height;
			break;
	}
	InitVS(&FakeVS);

	unsigned int numOut;
	unsigned char update = 1;
	Element CurrentElement("");
	int percent = 0;
	int insg = visibleChildren.size();

	while (!visibleChildren.empty()) // Loop until every token is put on some line
	{
		percent = 100-visibleChildren.size()*100/insg;
		_globals->getPercentIndicator()->update(percent);

		BLOCK FakeCA = {{0,0},{0,0}};
		Markupline CurrentLine;
		CurrentLine.setGlobals(_globals);

		while(1) // Loop until the current line is filled
		{
			if (update)
			{
				CurrentElement = visibleChildren.front();
				update = 0;
			}
			numOut = iPrint(CurrentElement.displayText,&FakeVS,&FakeCS,&FakeCA,-1,UTF8);

			if (numOut == CurrentElement.displayText.length())
			{
				CurrentLine.children.push_back(CurrentElement);
				visibleChildren.erase(visibleChildren.begin());
				update = 1;
				if (visibleChildren.empty())
					break;
			}
			else
			{
				Element EPart1(CurrentElement.displayText.substr(0,numOut));
				Element L_Old = CurrentElement;
				Element LPart1(L_Old.wikiNamespace, L_Old.target, L_Old.anchor, L_Old.displayText.substr(0,numOut), L_Old.sourcePositionStart, L_Old.sourceLength, L_Old.id);
				switch(CurrentElement.Type)
				{
					case TEXT_EL:
						CurrentLine.children.push_back(EPart1);
						CurrentElement.displayText.erase(0,numOut);
						break;
					case LINK_EL:
					case IMG_EL:
						CurrentLine.children.push_back(LPart1);
						CurrentElement.displayText.erase(0,numOut);
						break;
				}
				break;
			}
		}

		lines.push_back(CurrentLine);
	}

}

unsigned int Markup::numberOfLines()
{
	return lines.size();
}

int Markup::currentLine()
{
	return _currentLine;
}

unsigned char Markup::currentPercent()
{
	if (lines.size()==0)
		return 0;
	if (lines.size()==1)
		return 100;
	return (_currentLine*100)/(lines.size()-1);
}

unsigned char Markup::setCurrentLine(int line)
{
	int before = _currentLine;
	_currentLine = line;
	if (_currentLine < 0)
		_currentLine = 0;
	if (_currentLine >= lines.size())
		_currentLine = lines.size()-1;
	return (before!=_currentLine)?1:0;
}

unsigned char Markup::scrollLineDown()
{
	return setCurrentLine(_currentLine+1);
}

unsigned char Markup::scrollLineUp()
{
	return setCurrentLine(_currentLine-1);
}

unsigned char Markup::scrollPageDown()
{
	return setCurrentLine(_currentLine+_linesOnVScreen2);
}

unsigned char Markup::scrollPageUp()
{
	return setCurrentLine(_currentLine-_linesOnVScreen2);
}

void Markup::draw()
{
	int i;
	FillVS(&ContentWin1,_globals->backgroundColor());
	FillVS(&ContentWin2,_globals->backgroundColor());
	for (i=0;i<_linesOnVScreen1;i++)
	{
		if (((i+_currentLine-_linesOnVScreen1)>=0) && ((i+_currentLine-_linesOnVScreen1)<numberOfLines()))
			lines[i+_currentLine-_linesOnVScreen1].drawToVScreen(&ContentWin1,_markupCStat,i);
	}
	for (i=0;i<_linesOnVScreen2;i++)
	{
		if (((i+_currentLine)>=0) && ((i+_currentLine)<numberOfLines()))
			lines[i+_currentLine].drawToVScreen(&ContentWin2,_markupCStat,i);
	}
}

bool Markup::LoadOK()
{
	return _loadOK;
}

void Markup::setGlobals(Globals* globals)
{
	_globals = globals;
}

void Markupline::setGlobals(Globals* globals)
{
	_globals = globals;
}
