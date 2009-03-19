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
	_lastHighlightedLink = NULL;

	_showing_index = false;
	_colorChangeOnPage = false;
	_loadOK = false;

	_numberOfLines = 0;
	_currentLine = 0;
	_lastDisplayedLine = 0;
	_numNodes = 0;

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


void Markup::parse(string & Str, int type)
{
	// Transform the wikimarkup-string into proper XML-markup
	_globals->getStatusbar()->display("WikiMarkup->XML");
	WIKI2XML* w2x = new WIKI2XML();
	w2x->setGlobals(_globals);
	// Transform
	w2x->parse(Str, type);
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
	for (TiXmlNode* currentNode = _root; currentNode; currentNode = NextNode(currentNode) )
		_numNodes++;

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

	// create the layout of the article
	VirScreen LayoutSimulator = { 0, 0, ContentWin0.Width, 1048576 * _globals->getFont(FONT_R)->Height(), {{0,0},{0,0}}, &DnScreen};
	InitVS(&LayoutSimulator);

	BLOCK CharArea;
	CharArea.clear();

	// simulate the complete layout
	Paint(_root, &LayoutSimulator, &CharArea, true);
}

void Markup::Paint(TiXmlNode* firstNode, VirScreen* VS, BLOCK* CharArea, bool simulationPass)
{
	int VSLeft = VS->Left;
	CharStat CopyCS;
	int indent = 0;
	int numCurr = 0;

	for (TiXmlNode* currentNode = firstNode; currentNode && (CharArea->Start.y < (int) VS->Height); currentNode = NextNode(currentNode) )
	{
		getElementStyle(CopyCS, indent, currentNode);

		if (simulationPass)
		{
			CopyCS.Fx = SIMULATE;
			_globals->getPercentIndicator()->update( (numCurr++) * 100 / _numNodes );
		}

		if (currentNode->Type() == TiXmlNode::TEXT)
		{
			VS->Left = VSLeft + indent;
			VS->Width = ContentWin0.Width - indent;
			InitVS(VS);

			string text = currentNode->ValueStr();
			iPrint(text,VS,&CopyCS,CharArea,-1);
		}
		else if (currentNode->Type() == TiXmlNode::ELEMENT)
		{
			TiXmlElement* currentElement = (TiXmlElement*) currentNode;
			string name = currentElement->ValueStr();

			if (simulationPass)
			{
				// add offset information to an element only in the simulation pass
				int lineNumber = CharArea->Start.y / _globals->getFont(FONT_R)->Height();
				currentElement->SetAttribute("l",lineNumber);
				currentElement->SetAttribute("s",CharArea->Start.x);
			}

			// treatment of special elements
			if (name == "wl")
			{
				// a link, we have to keep track of all its clickable bounding boxes
				VS->Left = VSLeft + indent;
				VS->Width = ContentWin0.Width - indent;
				InitVS(VS);

				string content = pureText(currentNode);

				if (simulationPass)
				{
					// print the link text successively to calculate all bounding boxes
					// we only work with copies of the important variables VS and CharArea
					BLOCK CharAreaSimulate = *CharArea;

					int lineNumber = CharAreaSimulate.Start.y / _globals->getFont(FONT_R)->Height();
					int numOut = 0;
					int boxNr = 0;

					while (numOut < (int) content.length())
					{
						// Create a virtual screen that contains just one bottom line for the text to go into
						VirScreen oneLine = *VS;
						oneLine.Height = CharAreaSimulate.Start.y + _globals->getFont(FONT_R)->Height();
						InitVS(&oneLine);

						// output the rest of the text, and get the printed part
						BLOCK CharAreaSim_Backup = CharAreaSimulate;

						int outputCount = iPrint(content.substr(numOut),&oneLine,&CopyCS,&CharAreaSimulate,-1);

						string outputPart = content.substr(numOut,outputCount);
						trimRight(outputPart);

						int start = oneLine.Left + CharAreaSim_Backup.Start.x;
						iPrint(outputPart,&oneLine,&CopyCS,&CharAreaSim_Backup,-1);
						int   end = oneLine.Left + CharAreaSim_Backup.Start.x;

						if (end > start)
						{
							currentElement->SetAttribute(("l"+val(boxNr)).c_str(), lineNumber + boxNr);
							currentElement->SetAttribute(("s"+val(boxNr)).c_str(), start);
							currentElement->SetAttribute(("e"+val(boxNr)).c_str(), end);
						}

						numOut += outputCount;
						boxNr++;
					}

					// advance CharArea the normal way, just to make sure everything is layouted as in the display version

					iPrint(content,VS,&CopyCS,CharArea,-1);
				}
				else
				{
					iPrint(content,VS,&CopyCS,CharArea,-1);
				}
				while(currentNode->LastChild())
					currentNode = currentNode->LastChild();
			}
// 			else if (name == "wi")
// 			{
				// an image, we have to keep track of all its clickable bounding boxes (for future versions with image support) TODO
/*				VS->Left = VSLeft + indent;
				VS->Width = ContentWin0.Width - indent;
				InitVS(VS);

				string content = pureText(currentNode);
				iPrint(content,VS,&CopyCS,CharArea,-1);
				while(currentNode->LastChild())
					currentNode = currentNode->LastChild();
			}
			else if (name == "wt")
			{
				// a template, we have to keep track of all its clickable bounding boxes (for future versions) TODO
				VS->Left = VSLeft + indent;
				VS->Width = ContentWin0.Width - indent;
				InitVS(VS);

// 				string content = pureText(currentNode);
				iPrint("<template snipped>",VS,&CopyCS,CharArea,-1);
				while(currentNode->LastChild())
					currentNode = currentNode->LastChild();
			}*/
			else if (name == "li")
			{
				// element of a list, print either an asterisk or the correct enumeration number
				VS->Left = VSLeft + indent - 2 * _globals->getFont(FONT_R)->normalWidth();
				VS->Width = ContentWin0.Width - indent + 2 * _globals->getFont(FONT_R)->normalWidth();
				InitVS(VS);
				CharArea->Start.x = 0;
				iPrint(get_li_string(currentElement),VS,&CopyCS,CharArea);
				CharArea->Start.x -= 2 * _globals->getFont(FONT_R)->normalWidth();
				if (CharArea->Start.x < 0)
					CharArea->Start.x = 0;
			}

			// print all created attributes of the current element
/*			if (simulationPass)
			{
				PA_ClearTextBg(1);
				int i = 2;
				TiXmlAttribute* pAttrib = currentElement->FirstAttribute();
				while (pAttrib)
				{
					string name = pAttrib->Name();
					string val = pAttrib->ValueStr();
					PA_OutputText(1,0,i++,"%s %s",name.c_str(),val.c_str());
					pAttrib=pAttrib->Next();
				}
				PA_WaitFor(Pad.Newpress.Anykey);
			}*/
		}
	}

	if (simulationPass)
	{
		_numberOfLines = CharArea->Start.y / _globals->getFont(FONT_R)->Height() + 1;
	}
}

void Markup::getElementStyle(CharStat & CStat, int & indent, TiXmlNode* current)
{
	if (current == _root)
	{
		CStat = NormalCS;
		CStat.Color = _globals->textColor();
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
			else if (name=="math")
			{
				CStat.Color = PA_RGB(15,15,15);
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
				indent += 2 * _globals->getFont(FONT_R)->normalWidth();
			}
		}
	}
}

string Markup::get_li_string(TiXmlElement* current)
{
	TiXmlNode* pParent = current->Parent();
	if (pParent && pParent->Type() == TiXmlNode::ELEMENT)
	{
		string name = pParent->ValueStr();
		if (name == "ul")
			return "*\u00a0";
		else if (name == "ol")
		{
			int pos = 0;
			for (TiXmlNode* c = current; c; c = c->PreviousSibling() )
				pos++;
			string ret = val(pos)+".\u00a0";
			return ret;
		}
	}
	return "";
}

string Markup::pureText(TiXmlNode* pParent)
{
	switch (pParent->Type())
	{
		case TiXmlNode::TEXT :
		{
			return pParent->ValueStr();
			break;
		}
		case TiXmlNode::ELEMENT :
		{
			string name = pParent->ValueStr();

			if (name=="wl")
			{
				string ret; // the displayed text of a link

				TiXmlNode* child = pParent->FirstChild();

				if (child && child->ValueStr()=="wp") // syntax of internal links
				{
					ret += pureText(child); // the target

					TiXmlNode* altChild = child->NextSibling();

					if (altChild && altChild->ValueStr() == "wp")
					{
						// there is an alternative linktext given
						ret.clear();
						vector<string> alternativeTexts;
						for (TiXmlNode* aChild = altChild; aChild && aChild->ValueStr()=="wp"; aChild = aChild->NextSibling() )
						{
							alternativeTexts.push_back(pureText(aChild));
						}
						implode("|",alternativeTexts,ret);
					}

					TiXmlNode* trail = pParent->LastChild();
					if (trail && trail->ValueStr() == "trail")
					{
						ret += pureText(trail);
					}
				}
				else if (child && child->ValueStr()=="url") // syntax of external links
				{
					TiXmlNode* title = pParent->LastChild();
					if (title && title->ValueStr() == "title")
					{
						ret += pureText(title);
					}
				}

				return ret;
			}
/*			else if (name=="wi")
			{
			}*/
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
			first = _currentLine - _linesPerScreen;
			last = _currentLine + _linesPerScreen - 1;
		}
		else
		{
			// only scroll
			int textDelta = _currentLine - _lastDisplayedLine;

			if ( (textDelta <= -2 * _linesPerScreen) || (textDelta >= 2 * _linesPerScreen) )
			{
				// completely scrolled out
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
				// harder case for DMA_Copy
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
				// harder case for DMA_Copy
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
				// easier case for DMA_Copy
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
				// easier case for DMA_Copy
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

		if ( last < _currentLine )
		{
			// only repaint on top screen
			VirScreen repaintArea = { ContentWin1.Left, ContentWin1.Top + (first - _currentLine + _linesPerScreen) * h, ContentWin1.Width, (last - first + 1) * h, {{0,0},{0,0}}, &UpScreen};
			InitVS(&repaintArea);

			int line = 0;
			int offset = 0;
			BLOCK CharArea;

			TiXmlNode* before = findElementBeforeLine(first, line, offset);
			CharArea.clear();
			CharArea.Start.x = offset;
			CharArea.Start.y -= (first - line) * h;
			Paint(before, &repaintArea, &CharArea, false);
		}
		else if ( first >= _currentLine)
		{
			// only repaint on bottom screen
			VirScreen repaintArea = { ContentWin0.Left, ContentWin0.Top + (first-_currentLine) * h, ContentWin0.Width, (last - first + 1) * h, {{0,0},{0,0}}, &DnScreen};
			InitVS(&repaintArea);

			int line = 0;
			int offset = 0;
			BLOCK CharArea;

			TiXmlNode* before = findElementBeforeLine(first, line, offset);
			CharArea.clear();
			CharArea.Start.x = offset;
			CharArea.Start.y -= (first - line) * h;
			Paint(before, &repaintArea, &CharArea, false);
		}
		else
		{
			// repaint on both screens
			VirScreen repaintArea_top = { ContentWin1.Left, ContentWin1.Top + (first - _currentLine + _linesPerScreen) * h, ContentWin1.Width, (_currentLine - first) * h, {{0,0},{0,0}}, &UpScreen};
			InitVS(&repaintArea_top);
			VirScreen repaintArea_bottom = { ContentWin0.Left, ContentWin0.Top, ContentWin0.Width, (last - _currentLine + 1) * h, {{0,0},{0,0}}, &DnScreen};
			InitVS(&repaintArea_bottom);

			int line = 0;
			int offset = 0;
			BLOCK CharArea;

			TiXmlNode* before = findElementBeforeLine(first, line, offset);
			CharArea.clear();
			CharArea.Start.x = offset;
			CharArea.Start.y -= (first - line) * h;
			Paint(before, &repaintArea_top, &CharArea, false);

			before = findElementBeforeLine(_currentLine, line, offset);
			CharArea.clear();
			CharArea.Start.x = offset;
			CharArea.Start.y -= (_currentLine - line) * h;
			Paint(before, &repaintArea_bottom, &CharArea, false);
		}

		_lastDisplayedLine = _currentLine;
		_colorChangeOnPage = false;
	}
}

TiXmlNode* Markup::findElementBeforeLine(int lineNo, int & line, int & offset) // TODO make this fast by traversing the tree in a clever way
{
	line = 0;
	offset = 0;

	int line_best = 0;
	int offset_best = 0;

	if (lineNo == 0)
		return _root;

	TiXmlElement* currentElement;
	TiXmlNode* beforeLineElement = _root;

	for (TiXmlNode* currentNode = _root; currentNode; currentNode = NextNode(currentNode) )
	{
		if (currentNode->Type() == TiXmlNode::ELEMENT)
		{
			currentElement = (TiXmlElement*) currentNode;

			int ival;
			bool lineRead = false;
			bool offsetRead = false;

			TiXmlAttribute* pAttrib = currentElement->FirstAttribute();
			while (pAttrib)
			{
				string name = pAttrib->Name();
				if ( (name == "l") && (pAttrib->QueryIntValue(&ival) == TIXML_SUCCESS) )
				{
					lineRead = true;
					line = ival;
				}
				else if ( (name == "s") && (pAttrib->QueryIntValue(&ival) == TIXML_SUCCESS) )
				{
					offsetRead = true;
					offset = ival;
				}

				if (lineRead && offsetRead)
					break;
				else
					pAttrib=pAttrib->Next();
			}

			if (line < lineNo)
			{
				line_best = line;
				offset_best = offset;
				beforeLineElement = currentNode;
			}
			else
				break;
		}
	}

	offset = offset_best;
	line = line_best;
	return beforeLineElement;
}

bool Markup::evaluateClick(int absolute_x, int absolute_y)
{
	if (_showing_index)
	{
		return indexMarkup->evaluateClick(absolute_x, absolute_y);
	}
	else
	{
		int x = absolute_x - ContentWin0.AbsoluteBound.Start.x;
		int y = absolute_y - ContentWin0.AbsoluteBound.Start.y;
		int line = y / _globals->getFont(FONT_R)->Height() + _currentLine;
		int start = x;

		for (TiXmlElement* currentLink = NextLink(_root); currentLink; currentLink = NextLink(currentLink) )
		{
			TiXmlAttribute* pAttrib = currentLink->FirstAttribute();

			int ival;

			while (pAttrib)
			{
				string name = pAttrib->Name();
				if (name[0] == 'l' && name.length() > 1 && name.find_last_not_of("0123456789") == 0)
				{
					// we found a link part line attribute
					if ( (pAttrib->QueryIntValue(&ival) == TIXML_SUCCESS) && (line == ival) )
					{
						// we found a link part on the correct line
						string line_ident = name.substr(1);
						int s = -1;
						int e = -1;

						// get the bounding box
						TiXmlAttribute* offsetsAttrib = currentLink->FirstAttribute();
						while (offsetsAttrib)
						{
							string offsetname = offsetsAttrib->Name();
							if ( (offsetname == ("s" + line_ident)) && (offsetsAttrib->QueryIntValue(&ival) == TIXML_SUCCESS))
							{
								s = ival;
							}
							else if ( (offsetname == ("e" + line_ident)) && (offsetsAttrib->QueryIntValue(&ival) == TIXML_SUCCESS))
							{
								e = ival;
							}
							offsetsAttrib = offsetsAttrib->Next();
						}
						if ((s <= start) && (start <= e))
						{
							_currentHighlightedLink = currentLink;
							return true;
						}
					}
				}
				pAttrib=pAttrib->Next();
			}
		}
		return false;
	}
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
	if (_showing_index)
	{
		indexMarkup->getCurrentLink(title, anchor);
	}
	else
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
							anchor = val.substr(val.find("#")+1);
					}
				}
			}
		}
	}
}

void Markup::getFirstLink(string & title, string & anchor)
{
	_currentHighlightedLink = NextLink(_root);
	getCurrentLink(title, anchor);
	unselect();
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
	if (_showing_index)
	{
		indexMarkup->scrollToLine(lineNo);
	}
	else
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
}

void Markup::bringElementToTop(TiXmlElement* current)
{
	int line = getLine(current);
	if (line != -1)
		scrollToLine(line);
}

void Markup::jumpToAnchor(string anchor)
{
	if ( index.find(anchor) != index.end() )
		bringElementToTop((TiXmlElement*) index[anchor]);
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
			_lastHighlightedLink = _currentHighlightedLink;
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
		_lastHighlightedLink = _currentHighlightedLink;

		if (_currentHighlightedLink)
			_currentHighlightedLink = PreviousLink(_currentHighlightedLink);
		else
		{
			for (_currentHighlightedLink = PreviousLink(_end); _currentHighlightedLink && getLine(_currentHighlightedLink) >= _currentLine; _currentHighlightedLink = PreviousLink(_currentHighlightedLink));

			if (!_currentHighlightedLink)
				_currentHighlightedLink = PreviousLink(_end);
		}

		if (_currentHighlightedLink != _lastHighlightedLink)
		{
			_colorChangeOnPage = true;
		}

		if (_currentHighlightedLink)
		{
			bringElementToTop(_currentHighlightedLink);
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
		_lastHighlightedLink = _currentHighlightedLink;

		if (_currentHighlightedLink)
			_currentHighlightedLink = NextLink(_currentHighlightedLink);
		else
		{
			for (_currentHighlightedLink = NextLink(_root); _currentHighlightedLink && getLine(_currentHighlightedLink) < _currentLine; _currentHighlightedLink = NextLink(_currentHighlightedLink));

			if (_currentLine == _numberOfLines - 1)
				_currentHighlightedLink = NextLink(_root);

			if (!_currentHighlightedLink)
				_currentHighlightedLink = NextLink(_root);
		}

		if (_currentHighlightedLink != _lastHighlightedLink)
		{
			_colorChangeOnPage = true;
		}

		if (_currentHighlightedLink)
		{
			bringElementToTop(_currentHighlightedLink);
		}
	}
}


int Markup::getLine(TiXmlElement* current)
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
				return ival;
			}
			pAttrib=pAttrib->Next();
		}
	}
	return -1;
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

bool Markup::showingArticle()
{
	return !_showing_index;
}

void Markup::showArticle()
{
	_showing_index = false;
}
