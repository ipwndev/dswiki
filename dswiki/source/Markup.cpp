#include "Markup.h"

#define SLEEPTIME 0

Element::~Element()
{
}

Element::Element(string text)
{
	Type = TEXT_EL;
	displayText = text;
}


Element::Element(string str_namesp, string str_targ, string str_anch, string str_disp, u32 start, u32 length, u32 link_id)
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


Element* createLink(string Str, u32 startPos, u32 link_id)
{
	Device	UpScreen = {"U", 1, (u16*)PA_DrawBg[1], 256, 192};
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

// 		PA_Clear16bitBg(1);
// 		SimPrint(linkinnertext,&UpScreen,PA_RGB(0,0,0),UTF8);

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
// 	PA_OutputText(1,5,15,"Destructing Markup-Line");
	PA_Sleep(SLEEPTIME);

	children.clear();

// 	PA_OutputText(1,5,15,"                       ");
	PA_Sleep(SLEEPTIME);
}

void Markupline::drawToVScreen(VirScreen* VScreen, CharStat* CStat, s32 line)
{
	BLOCK CharArea = {{0,line*(CStat->FONT->Regular.Height+CStat->H_Space)},{0,0}}; // TODO: switch(Rotate)
	int i;
	for (i=0;i<children.size();i++)
	{
		switch (children[i].Type)
		{
			case TEXT_EL:
				CStat->Color = PA_RGB(0,0,0);
				break;
			case LINK_EL:
				CStat->Color = PA_RGB(0,5,23);
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
u8 Markupline::containsCertainLink(u32 id)
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
	u16 zeile = y / ( _markupCStat->FONT->Regular.Height + _markupCStat->H_Space );
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

string splitNormal(string Str)
{
	return Str;
}

string splitPre(string Str)
{
	string markup = "";
	u32 pos = 0;
	while ((pos=Str.find("<pre>"))!=string::npos)
	{
		string temp = Str.substr(0,pos);
		Str.erase(0,pos);
		markup += splitNormal(temp);
		if ((pos=Str.find("</pre>"))!=string::npos)
		{
			markup += treatPreText(Str.substr(5,pos-5));
			Str.erase(0,pos+6);
		}
		else
		{
			markup += treatPreText(Str.substr(5));
			Str.clear();
		}
	}
	markup += splitNormal(Str);
	Str.clear();

	return markup;
}

string splitNowiki(string Str)
{
	string markup = "";
	u32 pos = 0;
	while ((pos=Str.find("<nowiki>"))!=string::npos)
	{
		string temp = Str.substr(0,pos);
		Str.erase(0,pos);
		markup += splitPre(temp);
		if ((pos=Str.find("</nowiki>"))!=string::npos)
		{
			markup += treatNowikiText(Str.substr(8,pos-8));
			Str.erase(0,pos+9);
		}
		else
		{
			markup += treatNowikiText(Str.substr(8));
			Str.clear();
		}
	}
	markup += splitPre(Str);
	Str.clear();

	return markup;
}

Markup::Markup(string Str, VirScreen* VScreen1, VirScreen* VScreen2, CharStat* CStat, TitleIndex* titleindex)
{
// 	PA_OutputText(1,5,3,"Creating Markup");
	PA_Sleep(SLEEPTIME);


	_markupVScreen1 = VScreen1;
	_markupVScreen2 = VScreen2;
	_markupCStat    = CStat;
	_titleindex     = titleindex;
	_linesOnVScreen1 = 1 + ( ( _markupVScreen1->Height - _markupCStat->FONT->Regular.Height ) / ( _markupCStat->FONT->Regular.Height + _markupCStat->H_Space ) );
	_linesOnVScreen2 = 1 + ( ( _markupVScreen2->Height - _markupCStat->FONT->Regular.Height ) / ( _markupCStat->FONT->Regular.Height + _markupCStat->H_Space ) );
	_currentLine = 0;

	u32 pos = 0;
	u32 posPre = 0;
	u32 link_id = 0;
	Element* t2;
	Element* l;

	Str = splitNowiki(Str);

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
// 	PA_OutputText(1,5,19,"while (l) OK");

	Element t(Str.substr(pos));
	visibleChildren.push_back(t);

// 	PA_OutputText(1,5,18,"add (t) OK");

	createLines(VScreen1, CStat);
// 	PA_OutputText(1,5,17,"createLines OK");
// 	setCurrentLine(PA_RandMax(numberOfLines()-1));

// 	PA_OutputText(1,5,3,"               ");
	PA_Sleep(SLEEPTIME);
}

Markup::~Markup()
{
// 	PA_OutputText(1,5,9,"Destructing Markup");
	PA_Sleep(SLEEPTIME);

	visibleChildren.clear();
	lines.clear();

// 	PA_OutputText(1,5,9,"                  ");
	PA_Sleep(SLEEPTIME);
}

void Markup::createLines(VirScreen* VScreen, CharStat* CStat)
{
// 	PA_OutputText(1,5,13,"Creating Lines");
	PA_Sleep(SLEEPTIME);

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

	u32 elementNumber = 0;
	u32 numOut;
	u8 update = 1;
	Element CurrentElement("");

	while (elementNumber<visibleChildren.size()) // Loop until every token is put on some line
	{
		BLOCK FakeCA = {{0,0},{0,0}};
		Markupline CurrentLine;

		while(1) // Loop until the current line is filled
		{
			if (update)
			{
				CurrentElement = visibleChildren[elementNumber];
				update = 0;
			}
			numOut = iPrint(CurrentElement.displayText,&FakeVS,&FakeCS,&FakeCA,-1,UTF8);

			if (numOut == CurrentElement.displayText.length())
			{
				CurrentLine.children.push_back(CurrentElement);
				elementNumber++;
				update = 1;
				if (elementNumber==visibleChildren.size())
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

// 	PA_OutputText(1,5,13,"              ");
	PA_Sleep(SLEEPTIME);
}

u32 Markup::numberOfLines()
{
	return lines.size();
}

s32 Markup::currentLine()
{
	return _currentLine;
}

u8 Markup::currentPercent()
{
	if (lines.size()==0)
		return 0;
	if (lines.size()==1)
		return 100;
	return (_currentLine*100)/(lines.size()-1);
}

u8 Markup::setCurrentLine(s32 line)
{
	s32 before = _currentLine;
	_currentLine = line;
	if (_currentLine < 0)
		_currentLine = 0;
	if (_currentLine >= lines.size())
		_currentLine = lines.size()-1;
	return (before!=_currentLine)?1:0;
}

u8 Markup::scrollLineDown()
{
	return setCurrentLine(_currentLine+1);
}

u8 Markup::scrollLineUp()
{
	return setCurrentLine(_currentLine-1);
}

u8 Markup::scrollPageDown()
{
	return setCurrentLine(_currentLine+_linesOnVScreen2);
}

u8 Markup::scrollPageUp()
{
	return setCurrentLine(_currentLine-_linesOnVScreen2);
}

void Markup::draw()
{
	s32 i;
	FillVS(_markupVScreen1,PA_RGB(31,31,31));
	FillVS(_markupVScreen2,PA_RGB(31,31,31));
	for (i=0;i<_linesOnVScreen1;i++)
	{
		if (((i+_currentLine-_linesOnVScreen1)>=0) && ((i+_currentLine-_linesOnVScreen1)<numberOfLines()))
			lines[i+_currentLine-_linesOnVScreen1].drawToVScreen(_markupVScreen1,_markupCStat,i);
	}
	for (i=0;i<_linesOnVScreen2;i++)
	{
		if (((i+_currentLine)>=0) && ((i+_currentLine)<numberOfLines()))
			lines[i+_currentLine].drawToVScreen(_markupVScreen2,_markupCStat,i);
	}
}
