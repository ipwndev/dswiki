#include "Markup.h"

#include <PA9.h>
#include <string>
#include <vector>
#include "char_convert.h"
#include "main.h"
#include "Globals.h"
#include "PercentIndicator.h"
#include "Statusbar.h"
#include "WIKI2XML.h"
#include "WIKI2XML_global.h"
#include "tinyxml.h"

using namespace std;


Markup::Markup()
{
	_td = NULL;
	_root = NULL;
	_end = NULL;
	_currentHighlightedLink = NULL;

	_showing_index = false;
	_colorChangeOnPage = false;
	_loadOK = false;

	_numberOfLines = 0;
	_currentLine = 0;
	_lastDisplayedLine = -1;

	index.clear();
	indexMarkup = NULL;
	indexMarkupStr.clear();

	_globals = NULL;

	TiXmlBase::SetCondenseWhiteSpace( false );
}


Markup::~Markup()
{
	index.clear();
	if (_td)
	{
		delete _td;
		_td = NULL;
	}
	if (indexMarkup)
	{
		delete indexMarkup;
		indexMarkup = NULL;
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


void Markup::toggleIndex()
{
	if (indexMarkup)
	{
		_showing_index = !_showing_index;
	}
}

void Markup::build_index(vector <TiXmlNode*> & index)
{
	for ( TiXmlNode* pParent = _root; pParent; pParent = NextNode(pParent) )
	{
		if (pParent->Type() == TiXmlNode::ELEMENT)
		{
			string value = pParent->ValueStr();
			if (value.length() == 2 && value[0] == 'h' && value[1] > '0' && value[1] <= '9')
			{
				index.push_back(pParent);
			}
		}
	}
}

void Markup::parse(string & Str, bool interpreteWikiMarkup)
{
	// Transform the wikimarkup-string into proper XML-markup
	_globals->getStatusbar()->display("WikiMarkup->XML");
	WIKI2XML* w2x = new WIKI2XML();
	w2x->setGlobals(_globals);
	// Transform
	w2x->parse(Str, interpreteWikiMarkup);
	// Str was modified, so we can delete this helper class
	delete w2x;
	w2x = NULL;
	_globals->getStatusbar()->display("WikiMarkup->XML done");

// 	PA_Clear16bitBg(0);
// 	SimPrint(Str,&DnScreen,PA_RGB(0,0,0));
// 	PA_WaitFor(Pad.Newpress.Anykey);

	// Parse the xml-markup with tinyXML
	_globals->getStatusbar()->display("Parsing XML");
	_td = new TiXmlDocument();
	_td->setGlobals(_globals);
	_td->Parse( Str, Str.c_str(), NULL, TIXML_ENCODING_UTF8);
	_loadOK = !(_td->Error());
	_globals->getStatusbar()->display("Parsing XML done");

	if (_loadOK)
	{
		_globals->getStatusbar()->display("Post-Processing DOM");
		postProcessDOM();
	}
}


void Markup::postProcessDOM()
{
	// set the root node and the last node
	_root = _td->LastChild();
	for (_end = _root; _end->LastChild(); _end = _end->LastChild() );

	// create the index from the document structure (headings)
	vector <TiXmlNode*>	index_tmp;
	build_index(index_tmp);
	if (!index_tmp.empty())
	{
		int level_curr;
		int levels[9] = {0,0,0,0,0,0,0,0,0};
		for ( int a = 0; a < (int) index_tmp.size(); a++ )
		{
			// get current level of heading, the heading itself and compute the anchor (if some headings exist more than once)
			string heading = index_tmp[a]->ValueStr();
			level_curr = heading[1] - '0' - 1;
			string title   = pureText(index_tmp[a]);
			string anchor  = title;
			bool anchor_different = false;
			if (index.find(anchor) != index.end())
			{
				anchor_different = true;
				int b;
				for (b=2;index.find(title+"_"+val(b))!=index.end();b++);
				anchor += ("_"+val(b));
			}

			// link the anchor with its node
			index[anchor] = index_tmp[a];

			// count up
			levels[level_curr]++;
			for (int b=level_curr+1;b<9;b++)
				levels[b] = 0;

			// Generate the index string

			//   1. indent as in wiki-markup
			for (int b=1;b<level_curr;b++)
				indexMarkupStr += ":";

			indexMarkupStr += "[[#" + anchor + "|";

			//   2. generate the number
			for (int b=1;b<level_curr;b++)
				indexMarkupStr += val(levels[b])+".";
			indexMarkupStr += val( levels[level_curr] );

			//   3. add the title
			indexMarkupStr += " " + title + "]]";
			if (a+1< (int) index_tmp.size())
				indexMarkupStr += '\n';
		}
		indexMarkup = new Markup();
		indexMarkup->setGlobals(_globals);
		indexMarkup->parse(indexMarkupStr);
	}

	// create the layout of the article (TODO)
	VirScreen oneLayoutLine = ContentWin2;
	oneLayoutLine.Height = _globals->getFont(FONT_R)->Height();
	InitVS(&oneLayoutLine);

	CharStat CopyCS = NormalCS;
	BLOCK CharArea;
	CharArea.clear();
	int indent = 0;
	int lineNumber = 0;
	bool reallyPrint;
	string alternativeText;

	int numNodes = 0;
	for (TiXmlNode* currentNode = _root; currentNode; currentNode = NextNode(currentNode) )
		numNodes++;

	int numCurrentNode = 0;
	for (TiXmlNode* currentNode = _root; currentNode; currentNode = NextNode(currentNode) )
	{
		numCurrentNode++;
		_globals->getPercentIndicator()->update(numCurrentNode*100/numNodes);

		if (currentNode->Type() == TiXmlNode::TEXT)
		{
			getElementStyle(CopyCS, indent, reallyPrint, alternativeText, currentNode);
			CopyCS.Fx = SIMULATE;

			string text = currentNode->ValueStr();
			int length = text.length();

			int numOut = iPrint(text,&oneLayoutLine,&CopyCS,&CharArea);
			while (numOut < length)
			{
				lineNumber++;
				CharArea.clear();
				CharArea.Start.x = indent;
				numOut += iPrint(text.substr(numOut),&oneLayoutLine,&CopyCS,&CharArea,-1,true);
			}
		}
		else if (currentNode->Type() == TiXmlNode::ELEMENT)
		{
			TiXmlElement* currentElement = (TiXmlElement*) currentNode;
			currentElement->SetAttribute("l",lineNumber);
			currentElement->SetAttribute("s",CharArea.Start.x);

			getElementStyle(CopyCS, indent, reallyPrint, alternativeText, currentNode);
			CopyCS.Fx = SIMULATE;

			int length = alternativeText.length();

			int numOut = iPrint(alternativeText,&oneLayoutLine,&CopyCS,&CharArea);
			while (numOut < length)
			{
				lineNumber++;
				CharArea.clear();
				CharArea.Start.x = indent;
				numOut += iPrint(alternativeText.substr(numOut),&oneLayoutLine,&CopyCS,&CharArea,-1,true);
			}

		}
	}
	_numberOfLines = lineNumber;
}

// TODO add: info if the element begins on a new line
void Markup::getElementStyle(CharStat & CStat, int & indent, bool & reallyPrint, string & alternativeText, TiXmlNode* current)
{
	if (current == _root)
	{
		CStat = NormalCS;
		indent = 0;
		reallyPrint = true;
		alternativeText = "";
	}
	else
	{
		TiXmlNode* parent = current->Parent();
		getElementStyle(CStat, indent, reallyPrint, alternativeText, parent);
		if (current->Type() == TiXmlNode::ELEMENT)
		{
			string name = current->ValueStr();
			if (name=="i")
			{
				if (CStat.FONT == _globals->getFont(FONT_R))
					CStat.FONT = _globals->getFont(FONT_O);
				else if (CStat.FONT == _globals->getFont(FONT_B))
					CStat.FONT = _globals->getFont(FONT_BO);
			}
			else if (name=="b")
			{
				if (CStat.FONT == _globals->getFont(FONT_R))
					CStat.FONT = _globals->getFont(FONT_B);
				else if (CStat.FONT == _globals->getFont(FONT_O))
					CStat.FONT = _globals->getFont(FONT_BO);
			}
			else if (name=="wl")
			{
				CStat.Color = _globals->linkColor();
			}
			else if (name=="wi")
			{
				CStat.Color = _globals->imageColor();
			}
			else if (name=="pre")
			{
				CStat.BgColor = PA_RGB(29,29,30);
				CStat.Fx = BACKGR;
			}
			else if (name=="wt")
			{
				CStat.Color = _globals->templateColor();
				alternativeText = "<Template snipped>";
			}
			else if (name=="li")
			{
				alternativeText = "*\u00a0";
			}
			else if (name=="h2"||name=="h3"||name=="h4"||name=="h5"||name=="h6"||name=="h7"||name=="h8"||name=="h9")
			{
				CStat.FONT = _globals->getFont(FONT_B);
			}
		}
	}
}



string Markup::pureText(TiXmlNode* pParent)
{
	switch (pParent->Type())
	{
		case TiXmlNode::TEXT :
			return pParent->ValueStr();
			break;
		case TiXmlNode::ELEMENT :
			string name = pParent->ValueStr();
			if (name=="wl")
			{
				string ret; // the displayed text of a link

				TiXmlNode* child = pParent->FirstChild();
				if (child)
				{
					if (child->ValueStr()=="wp")
					{
						ret += pureText(child); // the target
					}

					TiXmlNode* altChild = child->NextSibling();
					if (altChild && altChild->ValueStr() == "wp")
					{
						// there is an alternative linktext given
						ret.clear();
					}
					for (altChild = child->NextSibling(); altChild && altChild->ValueStr()=="wp"; altChild = altChild->NextSibling())
					{
						ret += pureText(altChild);
						ret += "|";
					}
					ret.erase(ret.length()-1);
				}

				return ret;
			}
			else if (name=="wi")
			{
			}
			else
			{
				string ret;
				for (TiXmlNode* child=pParent->FirstChild();child;child=child->NextSibling())
				{
					ret += pureText(child);
				}
				return ret;
			}
			break;
	}
	return "";
}


void Markup::draw()
{
	if (_showing_index)
	{
		indexMarkup->draw();
	}
	else
	{
		if (_currentLine != _lastDisplayedLine)
		{
			// full re-draw
			_lastDisplayedLine = _currentLine;
		}
		else if (_colorChangeOnPage)
		{
			// minor re-draw
		}
		else
		{
			// nothing changed
		}

		// (TODO)
		FillVS(&ContentWin1,_globals->backgroundColor());
		FillVS(&ContentWin2,_globals->backgroundColor());
		BLOCK CharArea = {{0,0},{251,171}};
		CharStat CS = NormalCS;
		CS.Color = _globals->textColor();
		CS.Rotate = DEG0;
		Paint(_root,&CS,&CharArea);
	}
}


void Markup::Paint(TiXmlNode* parent, CharStat* CS, BLOCK* CharArea)
{
	CharStat CopyCS = *CS;
	string text = parent->ValueStr();
	int linebreaksBefore = 0;
	int linebreaksAfter = 0;

	switch (parent->Type())
	{
		case TiXmlNode::TEXT :
			iPrint(text,&ContentWin2,&CopyCS,CharArea);
			break;
		case TiXmlNode::ELEMENT :
			for (int a=0;a<linebreaksBefore;a++)
				iPrint("\n",&ContentWin2,&CopyCS,CharArea);

			if (text=="i")
			{
				if (CopyCS.FONT == _globals->getFont(FONT_R))
					CopyCS.FONT = _globals->getFont(FONT_O);
				else if (CopyCS.FONT == _globals->getFont(FONT_B))
					CopyCS.FONT = _globals->getFont(FONT_BO);
			}
			else if (text=="b")
			{
				if (CopyCS.FONT == _globals->getFont(FONT_R))
					CopyCS.FONT = _globals->getFont(FONT_B);
				else if (CopyCS.FONT == _globals->getFont(FONT_O))
					CopyCS.FONT = _globals->getFont(FONT_BO);
			}
			else if (text=="wl")
			{
				CopyCS.Color = _globals->linkColor();
			}
			else if (text=="wi")
			{
				CopyCS.Color = _globals->imageColor();
			}
			else if (text=="pre")
			{
				CopyCS.BgColor = PA_RGB(29,29,30);
				CopyCS.Fx = BACKGR;
			}
			else if (text=="wt")
			{
				CopyCS.Color = _globals->templateColor();
				iPrint("<Template snipped>",&ContentWin2,&CopyCS,CharArea);
				return;
			}
			else if (text=="li")
			{
				iPrint("* ",&ContentWin2,&CopyCS,CharArea);
			}
			else if (text=="h2"||text=="h3"||text=="h4"||text=="h5"||text=="h6"||text=="h7"||text=="h8"||text=="h9")
			{
				CopyCS.FONT = _globals->getFont(FONT_B);
			}

			TiXmlNode* child;
			for (child=parent->FirstChild(); child; child=child->NextSibling())
			{
				Paint(child,&CopyCS,CharArea);
			}

			for (int a=0;a<linebreaksAfter;a++)
				iPrint("\n",&ContentWin2,&CopyCS,CharArea);

			break;
	}
}

string Markup::getFirstLinkTarget()
{
	TiXmlElement* firstLink = NextLink(_root);

	if (firstLink)
	{
		TiXmlNode* child = firstLink->FirstChild();
		if (child)
		{
			if (child->ValueStr() == "wp")
			{
				child = child->FirstChild();
				if (child)
				{
					string ret = child->ValueStr();
					ret = ret.substr(0,ret.find("#"));
					return ret;
				}
			}
		}
	}

	return "";
}

TiXmlElement* Markup::NextLink(TiXmlNode* current)
{
	TiXmlNode* next;
	for (next = NextNode(current); next; next = NextNode(next))
	{
		if (next->Type() == TiXmlNode::ELEMENT && next->ValueStr() == "wl")
			break;
	}
	return (TiXmlElement*) next;
}

TiXmlElement* Markup::PreviousLink(TiXmlNode* current)
{
	TiXmlNode* prev;
	for (prev = PreviousNode(current); prev; prev = PreviousNode(prev))
	{
		if (prev->Type() == TiXmlNode::ELEMENT && prev->ValueStr() == "wl")
			break;
	}
	return (TiXmlElement*) prev;
}

TiXmlNode* Markup::NextNode(TiXmlNode* current, bool skipChildren, bool skipSiblings)
{
	TiXmlNode* child = current->FirstChild();
	if (child && (!skipChildren) )
	{
		return child;
	}

	TiXmlNode* sibling = current->NextSibling();
	if (sibling)
	{
		return sibling;
	}

	TiXmlNode* ancestor;
	for (ancestor = current->Parent(); ancestor && (ancestor != _root) && (!ancestor->NextSibling()); ancestor = ancestor->Parent());
	if ( ancestor && (ancestor != _root) )
	{
		return ancestor->NextSibling();
	}
	return NULL;
}

TiXmlNode* Markup::PreviousNode(TiXmlNode* current)
{
	TiXmlNode* sibling = current->PreviousSibling();
	if (sibling)
	{
		while (sibling->LastChild())
		{
			sibling = sibling->LastChild();
		}
		return sibling;
	}

	TiXmlNode* parent = current->Parent();
	if (parent)
	{
		return parent;
	}
	return NULL;
}

void Markup::scrollToLine(int lineNo)
{
	if (lineNo < 0)
	{
		_currentLine = 0;
	}
	else if (lineNo > _numberOfLines - 1)
	{
		_currentLine = _numberOfLines - 1;
	}
	else
	{
		_currentLine = lineNo;
	}
// 	PA_OutputText(1,0,3,"Scrolling to line %d    ",_currentLine);
// 	PA_WaitFor(Pad.Newpress.Anykey);
// 	PA_OutputText(1,0,3,"                          ");
}

void Markup::bringElementToTop(TiXmlElement* current)
{
	if ( current )
	{
		int ival;
		TiXmlAttribute* pAttrib = current->FirstAttribute();
		while (pAttrib)
		{
			string name = pAttrib->Name();
			if ( (name == "l") && (pAttrib->QueryIntValue(&ival) == TIXML_SUCCESS) )
			{
				scrollToLine(ival);
				return;
			}
			pAttrib=pAttrib->Next();
		}
	}
}

void Markup::jumpToAnchor(string anchor)
{
	if ( index.find(anchor) != index.end() )
	{
		bringElementToTop((TiXmlElement*) index[anchor]);
	}
}

void Markup::unselect()
{
	if (_showing_index)
	{
		indexMarkup->unselect();
	}
	else
	{
		_currentHighlightedLink = NULL;
		_colorChangeOnPage = true;
	}
}

void Markup::scrollLineUp()
{
	if (_showing_index)
	{
		indexMarkup->scrollLineUp();
	}
	else
	{
		scrollToLine(_currentLine - 1);
	}
}

void Markup::scrollLineDown()
{
	if (_showing_index)
	{
		indexMarkup->scrollLineDown();
	}
	else
	{
		scrollToLine(_currentLine + 1);
	}
}

void Markup::scrollPageUp()
{
	if (_showing_index)
	{
		indexMarkup->scrollPageUp();
	}
	else
	{
		scrollToLine(_currentLine - 10); // TODO
	}
}

void Markup::scrollPageDown()
{
	if (_showing_index)
	{
		indexMarkup->scrollPageDown();
	}
	else
	{
		scrollToLine(_currentLine + 10); // TODO
	}
}

void Markup::selectPreviousLink()
{
	if (_showing_index)
	{
		indexMarkup->selectPreviousLink();
	}
	else
	{
		if (_currentHighlightedLink == NULL)
		{
			// select the first link visible on both screens // TODO: (or last???)
			// otherwise scroll up
		}
		else
		{
			// select the previous link
			_currentHighlightedLink = PreviousLink(_currentHighlightedLink);
			_colorChangeOnPage = true;
		}
	}
}

void Markup::selectNextLink()
{
	if (_showing_index)
	{
		indexMarkup->selectNextLink();
	}
	else
	{
		if (_currentHighlightedLink == NULL)
		{
			// select the first link visible on both screens
			// otherwise scroll down
		}
		else
		{
			// select the next link
			_currentHighlightedLink = NextLink(_currentHighlightedLink);
			_colorChangeOnPage = true;
		}
	}
}

int Markup::currentPercent()
{
	if (_showing_index)
	{
		return indexMarkup->currentPercent();
	}
	else
	{
		if (_numberOfLines == 0)
		{
			return 0;
		}
		else if (_numberOfLines == 1)
		{
			return 100;
		}
		else
		{
			return (_currentLine*100/(_numberOfLines-1));
		}
	}
}

int Markup::currentLine()
{
	if (_showing_index)
	{
		return indexMarkup->currentLine();
	}
	else
	{
		return _currentLine;
	}
}

