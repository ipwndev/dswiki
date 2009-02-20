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
	_showing_index = false;
	index.clear();
	indexRoot = NULL;
	indexMarkup = NULL;
	indexMarkupStr.clear();

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


void Markup::parse(string & Str)
{
	// Transform the wikimarkup-string into proper xml-markup
	_globals->getStatusbar()->display("WikiMarkup->XML");
	WIKI2XML* w2x = new WIKI2XML();
	w2x->setGlobals(_globals);
	// Transform
	w2x->parse(Str);
	// Str was modified, so we can delete this helper class
	delete w2x;
	w2x = NULL;
	_globals->getStatusbar()->display("WikiMarkup->XML done");

	// Parse the xml-markup with tinyXML
	_globals->getStatusbar()->display("Parsing XML");
	_td = new TiXmlDocument();
	_td->Parse(Str.c_str(), NULL, TIXML_ENCODING_UTF8);
	_loadOK = !(_td->Error());
	_globals->getStatusbar()->display("Parsing XML done");

	if (_loadOK)
	{
		indexRoot = _td->LastChild();

		vector <TiXmlNode*>	index_tmp;
		build_index(_td, index_tmp);
		if (!index_tmp.empty())
		{
			int level_curr;
			int levels[9] = {0,0,0,0,0,0,0,0,0};
			for ( int a = 0; a < (int) index_tmp.size(); a++ )
			{
				// get current level of heading, the heading itself and compute the anchor (if some headings exist more than once)
				string heading = index_tmp[a]->ValueStr();
				level_curr = heading[1] - '0' - 1;
				string title   = ((TiXmlText*) index_tmp[a]->FirstChild())->ValueStr(); // TODO: get the pure text recursively from the XML, OK for now
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

				// indent as in wiki-markup
				for (int b=1;b<level_curr;b++)
					indexMarkupStr += ":";

				indexMarkupStr += "[[#" + anchor + "|";

				// generate the number
				for (int b=1;b<level_curr;b++)
					indexMarkupStr += val(levels[b])+".";
				indexMarkupStr += val( levels[level_curr] );

				// add the title
				indexMarkupStr += " " + title + "]]";
				if (a+1< (int) index_tmp.size())
					indexMarkupStr += '\n';
			}
			indexMarkup = new Markup();
			indexMarkup->setGlobals(_globals);
			indexMarkup->parse(indexMarkupStr);
		}
	}
}


void Markup::build_index(TiXmlNode* pParent, vector <TiXmlNode*> & index)
{
	if ( !pParent ) return;

	TiXmlNode* pChild;
	if (pParent->Type() == TiXmlNode::ELEMENT)
	{
		string value = pParent->ValueStr();
		if (value.length() == 2 && value[0] == 'h' && value[1] > '0' && value[1] <= '9')
		{
			index.push_back(pParent);
		}
	}
	for ( pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSibling())
	{
		build_index( pChild, index );
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
	else
	{
		return false;
	}
}

void Markup::draw()
{
	if (_showing_index)
	{
		indexMarkup->draw();
	}
	else
	{
		FillVS(&ContentWin1,_globals->backgroundColor());
		FillVS(&ContentWin2,_globals->backgroundColor());
		BLOCK CharArea = {{0,0},{0,0}};
		CharStat CS = ContentCS;
		CS.Color = _globals->textColor();
		Paint(indexRoot,&CS,&CharArea);
	}
}

void Markup::Paint(TiXmlNode* parent, CharStat* CS, BLOCK* CharArea)
{
	CharStat CopyCS = *CS;
	string text = parent->ValueStr();
	int linebreaksBefore, linebreaksAfter;
	switch (parent->Type())
	{
		case TiXmlNode::TEXT :
			iPrint(text,&ContentWin2,&CopyCS,CharArea,-1,UTF8);
			break;
		case TiXmlNode::ELEMENT :
			text = parent->ValueStr();

			number_Of_Linebreaks(text,linebreaksBefore,linebreaksAfter);
			for (int a=0;a<linebreaksBefore;a++)
				iPrint("\n",&ContentWin2,&CopyCS,CharArea,-1,UTF8);

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
			else if (text=="wt")
			{
				CopyCS.Color = _globals->templateColor();
				iPrint("<Template snipped>",&ContentWin2,&CopyCS,CharArea,-1,UTF8);
				return;
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
				iPrint("\n",&ContentWin2,&CopyCS,CharArea,-1,UTF8);

			break;
	}
}

string Markup::getFirstLink()
{
	return "";
}

void Markup::number_Of_Linebreaks(string & name, int & before, int & after)
{
/*	if (name=="p")
	{
		before = 0;
		after = 1;
	}
	else if (name=="h2")
	{
		before = 1;
		after = 1;
	}
	else if (name=="h3"||name=="h4"||name=="h5"||name=="h6"||name=="h7"||name=="h8"||name=="h9")
	{
		before = 1;
		after = 1;
	}
	else
	{*/
		before = 0;
		after = 0;
// 	}
}