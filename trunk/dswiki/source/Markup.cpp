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
	TiXmlBase::SetCondenseWhiteSpace( false );
}


Markup::~Markup()
{
	if (_td)
	{
		delete _td;
		_td = NULL;
	}
}


void Markup::parse(string & Str)
{
	if (_td)
	{
		delete _td;
		_td = NULL;
	}

	// Transform the wikimarkup-string into proper xml-markup
	WIKI2XML* w2x = new WIKI2XML();
	if (w2x)
	{
		w2x->setGlobals(_globals);
		// Transform
		w2x->parse(Str);
		// Str was modified, so we can delete this helper class
		delete w2x;
		w2x = NULL;
	}

	// Parse the xml-markup with tinyXML
	_globals->getStatusbar()->display("Parsing XML");
	_loadOK = false;
	_td = new TiXmlDocument();
	_td->Parse(Str.c_str(), NULL, TIXML_ENCODING_UTF8);
	_loadOK = !(_td->Error());

	if (_loadOK)
	{
		index.clear();
		indexMarkupStr.clear();

		root = _td->LastChild();

		vector <TiXmlNode*>	index_tmp;
		build_index(_td, index_tmp);
		if (!index_tmp.empty())
		{
			int level_curr;
			int levels[9] = {0,0,0,0,0,0,0,0,0};
			for (int a=0;a<index_tmp.size();a++)
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
				if (a+1<index_tmp.size())
					indexMarkupStr += '\n';
			}
			Markup* markup = new Markup();
			markup->setGlobals(_globals);
			markup->parse(indexMarkupStr);
		}
	}
}

void Markup::build_index(TiXmlNode* pParent, vector <TiXmlNode*> & index)
{
	if ( !pParent ) return;

	TiXmlNode* pChild;
	if (pParent->Type() == TiXmlNode::ELEMENT)
	{
		string value = pParent->Value();
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
