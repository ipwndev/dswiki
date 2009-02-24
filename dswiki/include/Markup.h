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

class MarkupLineElement
{
	public:
		TiXmlNode*			linkedWith;
		string				text;
		unsigned short int	color;
		unsigned short int	bgcolor;
		int					left;
		int					width;
};

class MarkupLine
{
	public:
		vector<MarkupLineElement> elements;
};

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
		void	build_index(TiXmlNode* pParent, vector <TiXmlNode*> & index);
		string	getFirstLink(TiXmlNode* pParent);
		TiXmlDocument*			_td;
		TiXmlNode*				_root;
		bool					_loadOK;
		bool					_showing_index;
		Globals*				_globals;
		map <string,TiXmlNode*>	index;
		string					indexMarkupStr;
		Markup*					indexMarkup;
		void	Paint(TiXmlNode* parent, CharStat* CS, BLOCK* CharArea);
		void	number_Of_Linebreaks(string & name, int & before, int & after);
};

#endif
