#include "WIKI2XML_TXML.h"
#include <PA9.h>
#include "main.h"

// *****************************************************************************
// *****************************************************************************
//
// TXML
//
// *****************************************************************************
// *****************************************************************************

TXML::TXML(int f, int t, string & s, bool fix_comments)
{
	from = f;
	to = t;
	closing = selfclosing = false;

	name = s.substr(from + 1, to - (from + 1));
	trim(name);
	if (left(name, 1) == "/")
	{
		closing = true;
		name = name.substr(1, name.length() - 1);
	}
	if (right(name, 1) == "/")
	{
		selfclosing = true;
		name = name.substr(0, name.length() - 1);
	}
	name = before_first(' ', name);

    // This will replace < and > within a comment with the appropriate HTML entities
	if (fix_comments && left(name, 1) == "!")
	{
		int a;
		for (a = from + 1; a < to; a++)
		{
			if (s[a] != '>' && s[a] != '<')
				continue;
			to += 3;
			if (s[a] == '>')
				s.insert(a, "&gt");
			if (s[a] == '<')
				s.insert(a, "&lt");
			s[a + 3] = ';';
		}
	}
}

void TXML::remove_at(int pos)
{
	if (pos < from)
		from--;
	if (pos < to)
		to--;
}

void TXML::insert_at(int pos)
{
	if (pos < from)
		from++;
	if (pos < to)
		to++;
}

void TXML::add_key_value(string k, string v)
{
	trim(k);
	trim(v);
	key.push_back(k);
	value.push_back(v);
}

string TXML::get_string()
{
	string ret;
	ret = "<" + name;
	for (int a = 0; a < (int) key.size(); a++)
	{
		for (int b = 0; b < (int) key[a].length(); b++)
		{
			if (key[a][b] == ' ')
				key[a][b] = '_';
		}
		ret += " " + key[a];
		if (!(value[a].empty()))
			ret += "=\"" + unquote(SINGLE_QUOTE, value[a]) + "\"";
	}
	if (text.empty())
		ret += " />";
	else
		ret += ">" + text + "</" + name + ">";
	return ret;
}
