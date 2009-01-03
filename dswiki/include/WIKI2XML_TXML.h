#ifndef _TXML_H_
#define _TXML_H_

#include "WIKI2XML_global.h"

class TXML
{
	public:
		TXML() { };
		TXML(int f, int t, string & s, bool fix_comments = true);
		void remove_at(int pos);
		void insert_at(int pos);

		void add_key_value(string k, string v = "");
		string get_string();

	    // Variables
		int from, to;
		bool closing, selfclosing;
		string name, text;
		vector < string > key, value;
};

#endif
