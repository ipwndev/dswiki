#include "WIKI2XML_global.h"
#include <PA9.h>
#include "main.h"

// *****************************************************************************
// *****************************************************************************
//
// global string functions
//
// *****************************************************************************
// *****************************************************************************


// The following functions should be language specific
bool is_text_char(chart ch)
{
	if (ch >= 'a' && ch <= 'z')
		return true;
	if (ch >= 'A' && ch <= 'Z')
		return true;
	return false;
}


// These are not :

string left(string & s, int num)
{
	if (num <= 0)
		return "";
	if (num >= s.length())
		return s;
	return s.substr(0, num);
}

string right(string & s, int num)
{
	if (num <= 0)
		return "";
	int from = s.length() - num;
	if (from <= 0)
		return s;
	else
		return s.substr(from, s.length() - from);
}

string upper(string s)		// For internal purposes, will do...
{
	int a;
	for (a = 0; a < s.length(); a++)
	{
		if (s[a] >= 'a' && s[a] <= 'z')
			s[a] = s[a] - 'a' + 'A';
	}
	return s;
}

void explode(string pattern, string & s, vector < string > & parts)
{
	parts.clear();
	int a, b;

	a = 0;
	b = s.find(pattern,a);
	while (b != string::npos)
	{
		parts.push_back(s.substr(a,b-a));
		a = b + pattern.length();
		b = s.find(pattern,a);
	}
	parts.push_back(s.substr(a));
}

void implode(string pattern, vector < string > & parts, string & s)
{
	s.clear();

	if (parts.size() == 0)
		return;
	if (parts.size() == 1)
	{
		s += parts[0];
		return;
	}

	s += parts[0];
	for (int a = 1; a < parts.size(); a++)
	{
		s += pattern + parts[a];
	}
}

string unquote(chart quote, string & s)
{
	int a;
	for (a = 0; a < s.length(); a++)
	{
		if (s[a] == quote && (a == 0 || (a > 0 && s[a - 1] != '\\')))
		{
			s.insert(a, "\\");
			a++;
		}
	}
	return s;
}

bool submatch(string & main, string & sub, int from)
{
	if (from + sub.length() > main.length())
		return false;
	int a;
	for (a = 0; a < sub.length(); a++)
	{
		if (sub[a] != main[a + from])
			return false;
	}
	return true;
}

int find_first(chart c, string & s)
{
	int a;
	for (a = 0; a < s.length() && s[a] != c; a++);
	if (a == s.length())
		return -1;
	return a;
}

int find_last(chart c, string & s)
{
	int a, b = -1;
	for (a = 0; a < s.length(); a++)
	{
		if (s[a] == c)
			b = a;
	}
	return b;
}

string before_first(chart c, string s)
{
	int pos = find_first(c, s);
	if (pos == -1)
		return s;
	return s.substr(0, pos);
}

string before_last(chart c, string s)
{
	int pos = find_last(c, s);
	if (pos == -1)
		return "";
	return s.substr(0, pos);
}

string after_first(chart c, string s)
{
	int pos = find_first(c, s);
	if (pos == -1)
		return "";
	return s.substr(pos + 1, s.length());
}

string after_last(chart c, string s)
{
	int pos = find_last(c, s);
	if (pos == -1)
		return s;
	return s.substr(pos + 1, s.length());
}

string trim(string & s)
{
	if (s.length() == 0)
		return s;
	if (s[0] != ' ' && s[s.length() - 1] != ' ')
		return s;
	int a, b;
	for (a = 0; a < s.length() && s[a] == ' '; a++);
	for (b = s.length() - 1; b >= 0 && s[b] == ' '; b--);
	return s.substr(a, b - a + 1);
}

int find_next_unquoted(chart c, string & s, int start)
{
	int a;
	chart lastquote = ' ';
	for (a = start; a < s.length(); a++)
	{
		if (s[a] == c && lastquote == ' ')
			return a;		// Success!
		if (s[a] != SINGLE_QUOTE && s[a] != DOUBLE_QUOTE)
			continue;		// No quotes, next
		if (a > 0 && s[a - 1] == '\\')
			continue;		// Ignore \' and \"
		if (lastquote == ' ')
			lastquote = s[a];	// Remember opening quote, text now quoted
		else if (lastquote == s[a])
			lastquote = ' ';	// Close quote, not quoted anymore
	}
	return -1;
}

string val(int a)
{
	char t[20];
	sprintf(t, "%d", a);
	return string(t);
}

/*
 * ( inside, tag    ""        ) => <tag>inside</tag>
 * ( inside, tag, param       ) => <tag param>inside</tag>
 * (   ""    tag    ""        ) => <tag />
 * (   ""    tag, param       ) => <tag param />
 * (   ??    tag, param, true ) => <tag param>
 */
string xml_embed(string inside, string tag, string param, bool openonly)
{
	string ret;

	tag = trim(tag);
	param = trim(param);
	inside = trim(inside);

	ret = "<" + tag;
	if (!param.empty())
		ret += " " + param;
	if (openonly)
		return ret + ">";
	else if (inside.empty())
		return ret + " />";
	else
		return ret + ">" + inside + "</" + tag + ">";
}


// string xml_params(string l)
// {
// 	string ret;
// 	vector < string > params;
// 	l = trim(l);
// 	while (!l.empty())
// 	{
// 		int p = find_next_unquoted(' ', l);
// 		string first;
// 		if (p == -1)
// 		{
// 			first = l;
// 			l = "";
// 		}
// 		else
// 		{
// 			first = left(l, p+1);
// 			l = l.substr(p+1, l.length() - p - 1);
// 		}
// 		first = trim(first);
// 		l = trim(l);
//
// 		if (first.empty())
// 			continue;
//
// 		p = find_next_unquoted('=', first);
// 		if (p == -1)
// 			first = xml_embed(first, "key");
// 		else
// 		{
// 			first = xml_embed(left(first, p), "key") + xml_embed(first.substr(p + 1, first.length() - p), "val");
// 		}
// 		first = xml_embed(first, "wp");
// 		ret += first;
// 	}
// 	return ret;
// }
