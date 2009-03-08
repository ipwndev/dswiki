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

		void	parse(string & Str, bool interpreteWikiMarkup = true);
		bool	LoadOK();

		void	draw(bool force = false);

		void	toggleIndex();

		void	scrollToLine(int lineNo);
		void	scrollPageUp();
		void	scrollPageDown();
		void	scrollLineUp();
		void	scrollLineDown();
		int		currentLine() ;
		int		currentPercent();

		string	getFirstLinkTarget();
		void	jumpToAnchor(string anchor);
		void	selectPreviousLink();
		void	selectNextLink();
		void	unselect();

		void	setGlobals(Globals* globals);

	private:
		void	postProcessDOM();
		void	build_index(vector <TiXmlNode*> & index);

		void	Paint(TiXmlNode* parent, CharStat* CS, BLOCK* CharArea);
		void	number_Of_Linebreaks(string & name, int & before, int & after);
		string	pureText(TiXmlNode* pParent);
		void	getElementStyle(CharStat & CStat, int & indent, bool & reallyPrint, string & alternativeText, TiXmlNode* current);

		void	bringElementToTop(TiXmlElement* current);

		TiXmlNode*	PreviousNode(TiXmlNode* current);
		TiXmlNode*	NextNode(TiXmlNode* current, bool skipChildren = false, bool skipSiblings = false);
		TiXmlElement* PreviousLink(TiXmlNode* current);
		TiXmlElement* NextLink(TiXmlNode* current);

		TiXmlDocument*			_td;
		TiXmlNode*				_root;
		TiXmlNode*				_end;
		TiXmlElement*			_currentHighlightedLink;

		bool					_loadOK;
		bool					_showing_index;
		bool					_colorChangeOnPage;
		int						_numberOfLines;
		int						_currentLine;
		int						_lastDisplayedLine;

		map <string,TiXmlNode*>	index;
		string					indexMarkupStr;
		Markup*					indexMarkup;

		Globals*				_globals;
};

#endif
