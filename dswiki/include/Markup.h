#ifndef _MARKUP_H
#define _MARKUP_H

#include <PA9.h>
#include <string>
#include <vector>
#include <map>
#include "api.h"
#include "chrlib.h"
#include "tinyxml.h"

class Globals;

using namespace std;

class Markup
{
	public:
		Markup();
		~Markup();
		void 			parse(string & Str);
		bool			LoadOK();
		void 			setGlobals(Globals* globals);
	private:
		void build_index(TiXmlNode* pParent, vector <TiXmlNode*> & index);
		TiXmlDocument*			_td;
		bool					_loadOK;
		Globals*				_globals;
		string					indexMarkupStr;
		TiXmlNode*				root;
		map <string,TiXmlNode*>	index;
};

#endif
