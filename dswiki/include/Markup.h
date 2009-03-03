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
		void	draw();
		void	parse(string & Str);
		bool	toggleIndex();
		bool	LoadOK();
		void	setGlobals(Globals* globals);
		string	getFirstLink();
	private:
		void	postProcessDOM();
		void	build_index(TiXmlNode* pParent, vector <TiXmlNode*> & index);
		string	getFirstLink(TiXmlNode* pParent);
		void	Paint(TiXmlNode* parent, CharStat* CS, BLOCK* CharArea);
		void	number_Of_Linebreaks(string & name, int & before, int & after);
		string	pureText(TiXmlNode* pParent);
		TiXmlNode*	NextNode(TiXmlNode* current, bool skipChildren = false, bool skipSiblings = false);
		TiXmlNode*	PreviousNode(TiXmlNode* current);
		void	getElementStyle(CharStat & CStat, int & indent, bool & reallyPrint, string & alternativeText, TiXmlNode* current);

		TiXmlDocument*			_td;
		TiXmlNode*				_root;
		TiXmlNode*				_end;

		bool					_loadOK;
		bool					_showing_index;

		map <string,TiXmlNode*>	index;
		string					indexMarkupStr;
		Markup*					indexMarkup;

		Globals*				_globals;
};

#endif
