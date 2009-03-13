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
	_lastDisplayedLine = 0;

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

// 	int numOut = 0;
// 	PA_Clear16bitBg(0);
// 	numOut = SimPrint(Str,&DnScreen,PA_RGB(0,0,0));
// 	PA_WaitFor(Pad.Newpress.Anykey);
// 	while (numOut<Str.length())
// 	{
// 		PA_Clear16bitBg(0);
// 		numOut += SimPrint(Str.substr(numOut),&DnScreen,PA_RGB(0,0,0));
// 		PA_WaitFor(Pad.Newpress.Anykey);
// 	}

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
	_globals->getPercentIndicator()->update(0);

	// set the root node and the last node
	_root = _td->LastChild();
	for (_end = _root; _end->LastChild(); _end = _end->LastChild() );

	// count the nodes
	int numNodes = 0;
	for (TiXmlNode* currentNode = _root; currentNode; currentNode = NextNode(currentNode) )
		numNodes++;

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
	VirScreen LayoutSimulator = { 0, 0, ContentWin0.Width, 1048576 * _globals->getFont(FONT_R)->Height(), {{0,0},{0,0}}, &DnScreen};
	InitVS(&LayoutSimulator);
	PA_Clear16bitBg(0);

	BLOCK CharArea;
	CharArea.clear();

	CharStat CopyCS = NormalCS;
	int indent = 0;

	for (TiXmlNode* currentNode = _root; currentNode; currentNode = NextNode(currentNode) )
	{
		getElementStyle(CopyCS, indent, currentNode);

		if (currentNode->Type() == TiXmlNode::TEXT)
		{
			LayoutSimulator.Left = indent;
			LayoutSimulator.Width = ContentWin0.Width - indent;
			InitVS(&LayoutSimulator);

			string text = currentNode->ValueStr();
			iPrint(text,&LayoutSimulator,&CopyCS,&CharArea);

			PA_WaitFor(Pad.Newpress.Anykey);
		}
		else if (currentNode->Type() == TiXmlNode::ELEMENT)
		{
			TiXmlElement* currentElement = (TiXmlElement*) currentNode;
			string name = currentElement->ValueStr();

			int lineNumber = CharArea.Start.y / _globals->getFont(FONT_R)->Height();
			currentElement->SetAttribute("l",lineNumber);

			if (name=="li")
			{
				LayoutSimulator.Left = indent - 12;
				LayoutSimulator.Width = ContentWin0.Width - indent + 12;
				InitVS(&LayoutSimulator);
				CharArea.Start.x = 0;
				iPrint("*",&LayoutSimulator,&CopyCS,&CharArea);
				CharArea.Start.x = 0;
				PA_WaitFor(Pad.Newpress.Anykey);
			}
		}
	}

	_numberOfLines = CharArea.Start.y / _globals->getFont(FONT_R)->Height() + 1;
}


void Markup::getElementStyle(CharStat & CStat, int & indent, TiXmlNode* current)
{
	if (current == _root)
	{
		CStat = NormalCS;
		indent = 0;
	}
	else
	{
		// inherit style
		TiXmlNode* parent = current->Parent();
		getElementStyle(CStat, indent, parent);

		// and modify
		if (current->Type() == TiXmlNode::ELEMENT)
		{
			string name = current->ValueStr();
			if (name=="i")
			{
				if (CStat.FONT == _globals->getFont(FONT_R))
				{
					CStat.FONT = _globals->getFont(FONT_O);
				}
				else if (CStat.FONT == _globals->getFont(FONT_B))
				{
					CStat.FONT = _globals->getFont(FONT_BO);
				}
			}
			else if (name=="b")
			{
				if (CStat.FONT == _globals->getFont(FONT_R))
				{
					CStat.FONT = _globals->getFont(FONT_B);
				}
				else if (CStat.FONT == _globals->getFont(FONT_O))
				{
					CStat.FONT = _globals->getFont(FONT_BO);
				}
			}
			else if (name=="wl")
			{
				if ( current == _currentHighlightedLink)
				{
					CStat.Color = _globals->activeLinkColor();
				}
				else
				{
					CStat.Color = _globals->linkColor();
				}
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
			}
			else if (name=="h1"||name=="h2"||name=="h3"||name=="h4"||name=="h5"||name=="h6"||name=="h7"||name=="h8"||name=="h9")
			{
				CStat.FONT = _globals->getFont(FONT_B);
			}
			else if (name=="dl"||name=="ul"||name=="ol")
			{
				indent += 12;
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


void Markup::draw(bool force)
{
	if (_showing_index)
	{
		indexMarkup->draw(force);
	}
	else
	{
		if ((!force) && (!_colorChangeOnPage) && (_currentLine == _lastDisplayedLine))
			return;

		int h = _globals->getFont(FONT_R)->Height();
		int _linesPerScreen = ContentWin1.Height / h;

		int first = 0;
		int last = 0;

		if (force || (_colorChangeOnPage && (_currentLine != _lastDisplayedLine))) // TODO treat the latter case more intelligent
		{
// 			PA_OutputText(1,25,5,"force");
			DMA_Copy(Blank,
					 (void*) (UpScreen.Ptr + ((ContentWin1.AbsoluteBound.Start.y ) << 8)),
					 (_linesPerScreen * h) << 8,
					 DMA_16NOW
					);
			DMA_Copy(Blank,
					 (void*) (DnScreen.Ptr + ((ContentWin0.AbsoluteBound.Start.y ) << 8)),
					 (_linesPerScreen * h) << 8,
					 DMA_16NOW
					);
			first = _currentLine - _linesPerScreen;
			last = _currentLine + _linesPerScreen - 1;
		}
		else if (_colorChangeOnPage && (_currentLine == _lastDisplayedLine))
		{
// 			PA_OutputText(1,25,5,"soft ");
			first = _currentLine - _linesPerScreen;
			last = _currentLine + _linesPerScreen - 1;
		}
		else
		{
			// only scroll
			int textDelta = _currentLine - _lastDisplayedLine;

// 			PA_OutputText(1,25,5,"%d     ",textDelta);

			if ( (textDelta <= -2 * _linesPerScreen) || (textDelta >= 2 * _linesPerScreen) )
			{
				DMA_Copy((void*) Blank,
						 (void*) (UpScreen.Ptr + ((ContentWin1.AbsoluteBound.Start.y ) << 8)),
						  (_linesPerScreen * h) << 8,
						   DMA_16NOW
						);
				DMA_Copy((void*) Blank,
						 (void*) (DnScreen.Ptr + ((ContentWin0.AbsoluteBound.Start.y ) << 8)),
						  (_linesPerScreen * h) << 8,
						   DMA_16NOW
						);
				first = _currentLine - _linesPerScreen;
				last = _currentLine + _linesPerScreen - 1;
			}
			else if ((textDelta > -2 * _linesPerScreen) && (textDelta <= -_linesPerScreen))
			{
				// big scroll, copy text from top to bottom screen
				// scrolled up, the text moves down
				// hard case for DMA_Copy
				DMA_Copy((void*) (UpScreen.Ptr + ((ContentWin1.AbsoluteBound.Start.y ) << 8)),
						  (void*) (DnScreen.Ptr + ((ContentWin0.AbsoluteBound.Start.y + (-_linesPerScreen - textDelta) * h) << 8)),
						   ((2*_linesPerScreen + textDelta) * h) << 8,
							 DMA_16NOW
						);
				DMA_Copy((void*) Blank,
						 (void*) (UpScreen.Ptr + ((ContentWin1.AbsoluteBound.Start.y ) << 8)),
						  (_linesPerScreen * h) << 8,
						   DMA_16NOW
						);
				if (textDelta < -_linesPerScreen)
				{
					DMA_Copy((void*) Blank,
						 (void*) (DnScreen.Ptr + ((ContentWin0.AbsoluteBound.Start.y ) << 8)),
						  ((-_linesPerScreen - textDelta) * h) << 8,
							 DMA_16NOW
						);
				}
				first = _currentLine - _linesPerScreen;
				last = _currentLine - _linesPerScreen - textDelta - 1;
			}
			else if ((textDelta > -_linesPerScreen) && (textDelta < 0))
			{
				// small scroll, moving text on every screen, and from top to bottom
				// scrolled up, the text moves down
				// hard case for DMA_Copy
				for (int i = _linesPerScreen + textDelta - 1; i >= 0; i--)
				{
					DMA_Copy((void*) (DnScreen.Ptr + ((ContentWin0.AbsoluteBound.Start.y + i * h) << 8)),
							  (void*) (DnScreen.Ptr + ((ContentWin0.AbsoluteBound.Start.y + ( i - textDelta ) * h) << 8)),
							   (h) << 8,
								DMA_16NOW
							);
				}
				DMA_Copy((void*) (UpScreen.Ptr + ((ContentWin1.AbsoluteBound.Start.y + (_linesPerScreen + textDelta) * h) << 8)),
						  (void*) (DnScreen.Ptr + ((ContentWin0.AbsoluteBound.Start.y ) << 8)),
						   (-textDelta * h) << 8,
							 DMA_16NOW
						);
				for (int i = _linesPerScreen + textDelta - 1; i >= 0; i--)
				{
					DMA_Copy((void*) (UpScreen.Ptr + ((ContentWin1.AbsoluteBound.Start.y + i * h) << 8)),
							  (void*) (UpScreen.Ptr + ((ContentWin1.AbsoluteBound.Start.y + ( i - textDelta ) * h) << 8)),
							   (h) << 8,
								DMA_16NOW
							);
				}
				DMA_Copy((void*) Blank,
						 (void*) (UpScreen.Ptr + ((ContentWin1.AbsoluteBound.Start.y ) << 8)),
						  (-textDelta * h) << 8,
							DMA_16NOW
						);
				first = _currentLine - _linesPerScreen;
				last = _currentLine - _linesPerScreen - textDelta - 1;
			}
			else if ((textDelta > 0) && (textDelta < _linesPerScreen))
			{
				// small scroll, moving text on every screen, and from bottom to top
				// scrolled down, the text moves up
				// easy case for DMA_Copy
				DMA_Copy((void*) (UpScreen.Ptr + ((ContentWin1.AbsoluteBound.Start.y + textDelta * h) << 8)),
						  (void*) (UpScreen.Ptr + ((ContentWin1.AbsoluteBound.Start.y ) << 8)),
						   ((_linesPerScreen - textDelta) * h) << 8,
							 DMA_16NOW
						);
				DMA_Copy((void*) (DnScreen.Ptr + ((ContentWin0.AbsoluteBound.Start.y) << 8)),
						  (void*) (UpScreen.Ptr + ((ContentWin1.AbsoluteBound.Start.y + (_linesPerScreen - textDelta) * h) << 8)),
						   (textDelta * h) << 8,
							DMA_16NOW
						);
				DMA_Copy((void*) (DnScreen.Ptr + ((ContentWin0.AbsoluteBound.Start.y + textDelta * h) << 8)),
						  (void*) (DnScreen.Ptr + ((ContentWin0.AbsoluteBound.Start.y ) << 8)),
						   ((_linesPerScreen - textDelta) * h) << 8,
							 DMA_16NOW
						);
				DMA_Copy((void*) Blank,
						 (void*) (DnScreen.Ptr + ((ContentWin0.AbsoluteBound.Start.y + (_linesPerScreen - textDelta) * h) << 8)),
						  (textDelta * h) << 8,
						   DMA_16NOW
						);
				first = _currentLine + _linesPerScreen - textDelta;
				last = _currentLine + _linesPerScreen - 1;
			}
			else if ((textDelta >= _linesPerScreen) && (textDelta < 2 * _linesPerScreen))
			{
				// big scroll, copy text from bottom to top screen
				// scrolled down, the text moves up
				// easy case for DMA_Copy
				DMA_Copy((void*) (DnScreen.Ptr + ((ContentWin0.AbsoluteBound.Start.y + (textDelta - _linesPerScreen) * h) << 8)),
						  (void*) (UpScreen.Ptr + ((ContentWin1.AbsoluteBound.Start.y ) << 8)),
						   ((2 * _linesPerScreen - textDelta) * h) << 8,
							 DMA_16NOW
						);
				if (textDelta > _linesPerScreen)
				{
					DMA_Copy((void*) Blank,
							  (void*) (UpScreen.Ptr + ((ContentWin1.AbsoluteBound.Start.y + (2 * _linesPerScreen - textDelta) * h) << 8)),
							   ((textDelta - _linesPerScreen) * h) << 8,
								 DMA_16NOW
							);
				}
				DMA_Copy((void*) Blank,
						 (void*) (DnScreen.Ptr + (ContentWin0.AbsoluteBound.Start.y << 8)),
						  (_linesPerScreen * h) << 8,
						   DMA_16NOW
						);
				first = _currentLine + _linesPerScreen - textDelta;
				last = _currentLine + _linesPerScreen - 1;
			}
		}


		if (first < 0)
			first = 0;
		if (last > _numberOfLines - 1 )
			last = _numberOfLines - 1;

		CharStat CopyCS = NormalCS;
		CopyCS.Color = PA_RGB(PA_RandMinMax(0,31),PA_RandMinMax(0,5),PA_RandMinMax(0,31));
		BLOCK CharArea;
		CharArea.clear();

		if (first < _currentLine)
		{
			CharArea.Start.y = (first - _currentLine + _linesPerScreen) * h; // Important part
			for (int i=first; (i<=last) && (i<_currentLine); i++)
			{
				iPrint("█ Line "+val(i)+"\n",&ContentWin1,&CopyCS,&CharArea);
			}
			CharArea.clear(); // Important part
			for (int i=_currentLine; (i<=last) && (i < _currentLine + _numberOfLines); i++)
			{
				iPrint("█ Line "+val(i)+"\n",&ContentWin0,&CopyCS,&CharArea);
			}
		}
		else
		{
			CharArea.Start.y = (first - _currentLine) * h; // Important part
			for (int i=first; (i<=last) && (i < _currentLine + _numberOfLines); i++)
			{
				iPrint("█ Line "+val(i)+"\n",&ContentWin0,&CopyCS,&CharArea);
			}
		}

		_lastDisplayedLine = _currentLine;
		_colorChangeOnPage = false;
	}
}


bool Markup::evaluateClick(int x, int y)
{
	x -= ContentWin0.AbsoluteBound.Start.x;
	y -= ContentWin0.AbsoluteBound.Start.y;
	int line = y / _globals->getFont(FONT_R)->Height() + _currentLine;
	int start = x;
	// TODO: Find a link on this coordinates, if there is one, make it active and return true
	PA_OutputText(1,0,0,"%d/%d     ",line,start);
	return false;
}


/******************************************************************************/

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

void Markup::getCurrentLink(string & title, string & anchor)
{
	title = "";
	anchor = "";

	if (_currentHighlightedLink)
	{
		TiXmlNode* child = _currentHighlightedLink->FirstChild();
		if (child)
		{
			if (child->ValueStr() == "wp")
			{
				child = child->FirstChild();
				if (child)
				{
					string val = child->ValueStr();
					title = val.substr(0,val.find("#"));
					if (val.find("#") != string::npos )
						anchor = val.substr(val.find("#"));
				}
			}
		}
	}
}

void Markup::getFirstLink(string & title, string & anchor)
{
	_currentHighlightedLink = NextLink(_root);
	getCurrentLink(title, anchor);
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
		if(_currentHighlightedLink)
		{
			_currentHighlightedLink = NULL;
			_colorChangeOnPage = true;
		}
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
		scrollToLine(_currentLine - ContentWin1.Height / _globals->getFont(FONT_R)->Height() );
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
		scrollToLine(_currentLine + ContentWin1.Height / _globals->getFont(FONT_R)->Height() );
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
		if (_currentHighlightedLink)
		{
			_currentHighlightedLink = PreviousLink(_currentHighlightedLink);
			_colorChangeOnPage = true;
		}
		else
		{
			_currentHighlightedLink = PreviousLink(_end);
			if (_currentHighlightedLink)
			{
				_colorChangeOnPage = true;
			}
		}

		if (_currentHighlightedLink)
		{
			bringElementToTop(_currentHighlightedLink);

			int halfLinesPerScreen = ContentWin1.Height / (2*_globals->getFont(FONT_R)->Height());
			for (int i=0;i<halfLinesPerScreen;i++)
			{
				scrollLineUp();
			}
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
		if (_currentHighlightedLink)
		{
			_currentHighlightedLink = NextLink(_currentHighlightedLink);
			_colorChangeOnPage = true;
		}
		else
		{
			_currentHighlightedLink = NextLink(_root);
			if (_currentHighlightedLink)
			{
				_colorChangeOnPage = true;
			}
		}

		if (_currentHighlightedLink)
		{
			bringElementToTop(_currentHighlightedLink);

			int halfLinesPerScreen = ContentWin1.Height / (2*_globals->getFont(FONT_R)->Height());
			for (int i=0;i<halfLinesPerScreen;i++)
			{
				scrollLineUp();
			}
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

bool Markup::LoadOK()
{
	return _loadOK;
}

void Markup::setGlobals(Globals* globals)
{
	_globals = globals;
}

bool Markup::toggleIndex()
{
	if (indexMarkup)
	{
		_showing_index = !_showing_index;
		return true;
	}
	return false;
}
