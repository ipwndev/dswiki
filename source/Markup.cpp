#include "Markup.h"

#define SLEEPTIME 0

Element::~Element()
{
	switch (Type)
	{
		case TEXT_EL:
// 			PA_OutputText(1,5,11,"Destructing Text-Element");
			break;
		case LINK_EL:
// 			PA_OutputText(1,5,11,"Destructing Link-Element");
			break;
	}
	PA_Sleep(SLEEPTIME);

// 	PA_OutputText(1,5,11,"                        ");
	PA_Sleep(SLEEPTIME);
}

Element::Element(string text)
{
// 	PA_OutputText(1,5,5,"Creating Text-Element");
	PA_Sleep(SLEEPTIME);

	Type = TEXT_EL;

	displayText = text;

// 	PA_OutputText(1,5,5,"                     ");
	PA_Sleep(SLEEPTIME);
}


Element::Element(string str_namesp, string str_targ, string str_anch, string str_disp, u32 start, u32 length, u32 link_id)
{
// 	PA_OutputText(1,5,5,"Creating Link-Element");
	PA_Sleep(SLEEPTIME);

	Type = LINK_EL;

	wikiNamespace = str_namesp;
	target        = str_targ;
	anchor        = str_anch;
	displayText   = str_disp;

	sourcePositionStart = start;
	sourceLength        = length;
	id                  = link_id;

// 	PA_OutputText(1,5,5,"                     ");
	PA_Sleep(SLEEPTIME);
}


Element* createLink(string Str, u32 startPos, u32 link_id)
{
	while (1)
	{
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

		int stelle;

		stelle=linkinnertext.find("|");

		string dispStr = "";
		string targStr = "";
		string anchStr = "";

		if (stelle==string::npos)
		{
			targStr = linkinnertext;
			dispStr = linkinnertext;
		}
		else
		{
			targStr = linkinnertext.substr(0,stelle);
			dispStr = linkinnertext.substr(stelle+1,stelle+1-linkend);
		}

		if (targStr.find_first_of("[]<>{}\n")!=string::npos)
			continue;

		dispStr = trim(dispStr);

		stelle = targStr.find("#");

		if (stelle!=string::npos)
		{
			anchStr = targStr.substr(stelle+1);
			targStr.resize(stelle);
		}

		targStr = trim(targStr);
		anchStr = trim(anchStr);

		while ((stelle=targStr.find("_"))!=string::npos)
		{
			targStr.replace(stelle,1," ");
		}

		// collect and append any suffix and spaces
		stelle = Str.find_first_of(" \n.,;?:|<>(){}[]!'\"=",linkend+2);
		stelle = Str.find_first_not_of(" ",stelle);
		dispStr.append(Str.substr(linkend+2,stelle-linkend-2));

		return new Element("",targStr,anchStr,dispStr,linkstart,(linkend+2)+Str.substr(linkend+2,stelle-(linkend+2)).length()-linkstart, link_id);
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
	BLOCK CharArea = {{0,line*(CStat->FONT->Height+CStat->H_Space)},{0,0}}; // TODO: switch(Rotate)
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
		}
		children[i].BoundingBox.Start.x = CharArea.Start.x;                   // TODO: Rotate & Screen-absolut machen
		children[i].BoundingBox.Start.y = CharArea.Start.y;                   // TODO: Rotate
		iPrint(children[i].displayText,VScreen,CStat,&CharArea,-1,UTF8);
		children[i].BoundingBox.End.x = CharArea.Start.x;                     // TODO: Rotate
		children[i].BoundingBox.End.y = CharArea.Start.y+CStat->FONT->Height; // TODO: Rotate
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
	u16 zeile = y / ( _markupCStat->FONT->Height + _markupCStat->H_Space );
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
					return trim(l->target);
				}
			}
		}
	}
	return "";
}


Markup::Markup(string Str, VirScreen* VScreen1, VirScreen* VScreen2, CharStat* CStat)
{
// 	PA_OutputText(1,5,3,"Creating Markup");
	PA_Sleep(SLEEPTIME);


	_markupVScreen1 = VScreen1;
	_markupVScreen2 = VScreen2;
	_markupCStat    = CStat;
	_linesOnVScreen1 = 1 + ( ( _markupVScreen1->Height - _markupCStat->FONT->Height ) / ( _markupCStat->FONT->Height + _markupCStat->H_Space ) );
	_linesOnVScreen2 = 1 + ( ( _markupVScreen2->Height - _markupCStat->FONT->Height ) / ( _markupCStat->FONT->Height + _markupCStat->H_Space ) );
	_currentLine = 0;

	u32 pos = 0;
	u32 link_id = 0;
	Element* t2;
	Element* l;

	while((pos=Str.find("&nbsp;"))!=string::npos)
	{
		Str.replace(pos,6,"\u00a0");
	}

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

	createLines(VScreen1, CStat);
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
	CharStat  FakeCS = { CStat->Color, CStat->BgColor, CStat->Wrap, CStat->Rotate, SIMULATE, 0, CStat->W_Space, 0, CStat->FONT};
	switch (CStat->Rotate)
	{
		case DEG0:
		case DEG180:
			FakeVS.Height = FakeCS.FONT->Height;
			break;
		case DEG90:
		case DEG270:
			FakeVS.Width = FakeCS.FONT->Height;
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
