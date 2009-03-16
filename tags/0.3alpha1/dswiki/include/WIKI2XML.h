#ifndef _WIKI2XML_H_
#define _WIKI2XML_H_

#include "WIKI2XML_global.h"
#include "WIKI2XML_TXML.h"

class Globals;

class TTableInfo
{
	public:
		TTableInfo();
		string new_row       (string attributes = "");
		string new_cell      (string attributes = "");
		string new_headercell(string attributes = "");
		string new_caption   (string caption, string attributes = "");
		string close();
		bool tr_open, td_open, th_open;
};

class WIKI2XML
{
	public:
		enum {FULL_PARSE,MEDIUM_PARSE,TEXT_PARSE};
		WIKI2XML();
		void parse(string & s, int type);
		void setGlobals(Globals* globals);
	private:
		void doQuotes(string & l);
		void make_tag_list(string & s, vector < TXML > &list);
		void parse_link(string & l, int &from, char mode = 'L');
		void parse_line_sub(string & l);
		void parse_line(string & l);
		string fix_list(string & l);
		string get_list_tag(chart c, bool open);
		bool is_list_char(chart c);
		void sanitize_html(string & s, vector < TXML > &taglist);
		void replace_part(string & s, int from, int to, string with);
		void replace_part_sync(string & s, int from, int to, string with, vector < TXML > &list);
		void parse_external_freelink(string & l, int &from);
		void parse_external_link(string & l, int &from);
		bool is_external_link_protocol(string protocol);
		int scan_url(string & l, int from);
		string table_markup(string & l);

		// Variables
		vector < string > allowed_html, nowiki_contents, math_contents, pre_contents, source_contents;
		vector < TTableInfo > tables;
		string list;
		Globals* _globals;
};

#endif
