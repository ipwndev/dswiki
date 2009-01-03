#ifndef _MARKUP_H
#define _MARKUP_H

#include <PA9.h>
#include <string>
#include <vector>
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
		TiXmlDocument*  _td;
		bool			_loadOK;
		Globals*		_globals;
};

void dump_to_stdout( TiXmlNode* pParent, unsigned int indent);
int dump_attribs_to_stdout(TiXmlElement* pElement, unsigned int indent);
void build_index(TiXmlNode* pParent, vector <TiXmlNode*> & index);
#endif
