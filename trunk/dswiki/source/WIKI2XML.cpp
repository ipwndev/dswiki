#include "WIKI2XML.h"
#include "main.h"
#include "char_convert.h"
#include "TextBox.h"
#include "fat.h"
#include "efs_lib.h"

TTableInfo::TTableInfo()
{
	tr_open = false;
	td_open = false;
}

string TTableInfo::close()
{
	string ret;
	if (td_open)
		ret += "</td>";
	if (tr_open)
		ret += "</tr>";
	ret += "</table>";
	return ret;
}

string TTableInfo::new_row()
{
	string ret;
	if (td_open)
		ret += "</td>";
	if (tr_open)
		ret += "</tr>";
	ret += "<tr>";
	td_open = false;
	tr_open = true;
	return ret;
}

string TTableInfo::new_cell(string type)
{
	string ret;
	if (!tr_open)
		ret += new_row();
	if (td_open)
		ret += "</td>";
	ret += "<td type=\"" + type + "\">";
	td_type = type;
	td_open = true;
	return ret;
}

// *****************************************************************************
// *****************************************************************************
//
// WIKI2XML
//
// *****************************************************************************
// *****************************************************************************
const string dswiki_magic_phrase = "DЅωïƙɨ";
const string dswiki_magic_nowiki_open  = "DЅωïƙɨИowiki";
const string dswiki_magic_nowiki_close = "DЅωïƙɨ/Иowiki";
const unsigned int magic_offset = 0xE000;


void WIKI2XML::doQuotes(string & l)
{
	int a,b;
	vector < string > parts;

	b = l.find("\'\'");
	if (b==string::npos)
		return;
	parts.push_back(l.substr(0,b));
	a = b;
	while (a < l.length())
	{
		for (b=a;b<l.length() && l[b] == '\''; b++);
		parts.push_back(l.substr(a,b-a));
		a = b;
		b = l.find("\'\'",a);
		if (b!=string::npos)
		{
			parts.push_back(l.substr(a,b-a));
			a = b;
		}
		else
		{
			parts.push_back(l.substr(a));
			break;
		}
	}

	int numbold = 0;
	int numitalics = 0;

	for (a=1;a<parts.size();a+=2)
	{
		if (parts[a].length() == 4)
		{
			parts[a-1] += "\'";
			parts[a] = "\'\'\'";
		}
		else if (parts[a].length() > 5)
		{
			for (b=0;b<parts[a].length() - 5;b++)
				parts[a-1] += "\'";
			parts[a] = "\'\'\'\'\'";
		}
		if (parts[a].length() == 2)
			numitalics++;
		else if (parts[a].length() == 3)
			numbold++;
		else if (parts[a].length() == 5)
		{
			numitalics++;
			numbold++;
		}
	}

	if ((numitalics % 2 == 1) && (numbold % 2 == 1))
	{
		int firstsingleletterword = -1;
		int firstmultiletterword = -1;
		int firstspace = -1;
		for ( a = 1; a < parts.size(); a += 2 )
		{
			if ( parts[a].length() == 3 )
			{
				string x1 = parts[a-1].substr(parts[a-1].length()-1);
				string x2 = parts[a-1].substr(parts[a-1].length()-2,1);
				if ((x1 == " ") && (firstspace == -1))
				{
					firstspace = a;
				}
				else if ((x2 == " ") && (firstsingleletterword == -1))
				{
					firstsingleletterword = a;
				}
				else if (firstmultiletterword == -1)
				{
					firstmultiletterword = a;
				}
			}
		}

		// If there is a single-letter word, use it!
		if (firstsingleletterword > -1)
		{
			parts[firstsingleletterword - 1] += "\'";
			parts[firstsingleletterword] = "\'\'";
		}
		// If not, but there's a multi-letter word, use that one.
		else if (firstmultiletterword > -1)
		{
			parts[firstmultiletterword - 1] += "\'";
			parts[firstmultiletterword] = "\'\'";
		}
		// ... otherwise use the first one that has neither.
		// (notice that it is possible for all three to be -1 if, for example,
		// there is only one pentuple-apostrophe in the line)
		else if (firstspace > -1)
		{
			parts[ firstspace-1 ] += "\'";
			parts[ firstspace ] = "\'\'";
		}

	}

// 	for (a=0;a<parts.size();a++)
// 	{
// 		PA_ClearTextBg(1);
// 		PA_OutputText(1,0,2,"%d italics, %d bold",numitalics,numbold);
// 		PA_OutputText(1,0,3,"%d/%d:->%s<-",a+1,parts.size(),parts[a].c_str());
// 		PA_WaitFor(Pad.Newpress.Anykey);
// 	}

	l = "";
	string buffer = "";
	string state  = "";

	for (a = 0; a < parts.size(); a++)
	{
		if ((a % 2) == 0)
		{
			if (state == "both")
				buffer += parts[a];
			else
				l += parts[a];
		}
		else
		{
			if (parts[a].length() == 2)
			{
				if (state == "i")
				{
					l += "</i>";
					state = "";
				}
				else if (state == "bi")
				{
					l += "</i>";
					state = "b";
				}
				else if (state == "ib")
				{
					l += "</b></i><b>";
					state = "b";
				}
				else if (state == "both")
				{
					l += "<b><i>" + buffer + "</i>";
					state = "b";
				}
				else // state can be "b" or ""
				{
					l += "<i>";
					state += "i";
				}
			}
			else if (parts[a].length() == 3)
			{
				if (state == "b")
				{
					l += "</b>";
					state = "";
				}
				else if (state == "bi")
				{
					l += "</i></b><i>";
					state = "i";
				}
				else if (state == "ib")
				{
					l += "</b>";
					state = "i";
				}
				else if (state == "both")
				{
					l += "<i><b>" + buffer + "</b>";
					state = "i";
				}
				else // state can be "i" or ""
				{
					l += "<b>";
					state += "b";
				}
			}
			else if (parts[a].length() == 5)
			{
				if (state == "b")
				{
					l += "</b><i>";
					state = "i";
				}
				else if (state == "i")
				{
					l += "</i><b>";
					state = "b";
				}
				else if (state == "bi")
				{
					l += "</i></b>";
					state = "";
				}
				else if (state == "ib")
				{
					l += "</b></i>";
					state = "";
				}
				else if (state == "both")
				{
					l += "<i><b>" + buffer + "</b></i>";
					state = "";
				}
				else // (state == "")
				{
					buffer = "";
					state = "both";
				}
			}
		}
	}

	// Now close all remaining tags.  Notice that the order is important.
	if (state == "b" || state == "ib")
		l += "</b>";
	if (state == "i" || state == "bi" || state == "ib")
		l += "</i>";
	if (state == "bi")
		l += "</b>";
	// There might be lonely ''''', so make sure we have a buffer
	if ((state == "both") && (!buffer.empty()))
		l += "<b><i>" + buffer + "</i></b>";
}


bool WIKI2XML::is_list_char(chart c)	// For now...
{
	if (c == '*')
		return true;
	if (c == '#')
		return true;
	if (c == ':')
		return true;
	return false;
}


string WIKI2XML::get_list_tag(chart c, bool open)
{
	string ret;
	if (debug)
		cout << "get_list_tag : " << c << endl;
	if (c == '*')
		ret = "ul";
	if (c == '#')
		ret = "ol";
	if (c == ':')
		ret = "dl";
	if (ret != "") {
		string itemname = "li";
		if (c == ':')
			itemname = "dd";
		if (open)
			ret = "<" + ret + "><" + itemname + ">";
		else
			ret = "</" + itemname + "></" + ret + ">";
	}
	return ret;
}


string WIKI2XML::fix_list(string & l)
{
	int a, b;
	for (a = 0; a < l.length() && is_list_char(l[a]); a++);
	string newlist, pre;
	if (a > 0) {
		newlist = left(l, a);
		while (a < l.length() && l[a] == ' ')
			a++;		// Removing leading blanks
		l = l.substr(a, l.length());
	}
	if (debug)
		cout << "fix_list : " << l << endl;
	if (list == "" && newlist == "")
		return "";
	for (a = 0; a < list.length() && a < newlist.length() && list[a] == newlist[a]; a++);	// The common part, if any

	for (b = a; b < list.length(); b++)
		pre = get_list_tag(list[b], false) + pre;	// Close old list tags
	for (b = a; b < newlist.length(); b++)
		pre += get_list_tag(newlist[b], true);	// Open new ones

	if (debug)
		cout << "pre : " << pre << endl;
	if (debug)
		cout << "newlist : " << newlist << endl;
	list = newlist;
	return pre;
}


bool WIKI2XML::is_external_link_protocol(string protocol)
{
	if (protocol == "HTTP")
		return true;
	if (protocol == "HTTPS")
		return true;
	if (protocol == "FTP")
		return true;
	if (protocol == "MAILTO")
		return true;
	return false;
}

int WIKI2XML::scan_url(string & l, int from)
{
	int a;
	for (a = from; a < l.length(); a++) {
		if (l[a] == '$' || l[a] == '-' || l[a] == '_' || l[a] == '.' || l[a] == '+' || l[a] == '\''
				  || l[a] == '*' || l[a] == '!' || l[a] == '(' || l[a] == ')' || l[a] == ','
				  || l[a] == '&' || l[a] == '/' || l[a] == ':' || l[a] == ';' || l[a] == '='
				  || l[a] == '?' || l[a] == '@' || l[a] == '~' || l[a] == '%')
			continue;
		if (l[a] >= '0' && l[a] <= '9')
			continue;
		if (is_text_char(l[a]))
			continue;
		break;			// End of URL
	}
	return a;
}


void WIKI2XML::parse_external_freelink(string & l, int &from)
{
	int a;
	for (a = from - 1; a >= 0 && is_text_char(l[a]); a--);
	if (a == -1)
		return;
	a++;
	string protocol = upper(l.substr(a, from - a));
	if (debug)
		cout << "protocol : " << protocol << endl;
	if (!is_external_link_protocol(protocol))
		return;
	int to = scan_url(l, a);
	string url = l.substr(a, to - a);
	string replacement;
	replacement += xml_embed(url, "url");
	replacement += xml_embed(url, "title");
	l = left(l, a) + replacement + l.substr(to, l.length() - to);
	from = a + replacement.length() - 1;
}


void WIKI2XML::parse_external_link(string & l, int &from)
{
	string protocol =
			upper(before_first(':', l.substr(from + 1, l.length() - from)));
	if (!is_external_link_protocol(protocol))
		return;
	int to;
	for (to = from + 1; to < l.length() && l[to] != ']'; to++);
	if (to == l.length())
		return;
	string url = l.substr(from + 1, to - from - 1);
	string title = after_first(' ', url);
	url = before_first(' ', url);
	string replacement;
	replacement += xml_embed(url, "url");
	if (title == "")
		replacement += xml_embed("<wuc action=\"add\" />", "title"); //wikiurlcounter
	else
		replacement += xml_embed(title, "title");

	replacement = xml_embed(replacement, "wl", "type='ext' protocol='" + protocol + "'");
	l = left(l, from) + replacement + l.substr(to + 1, l.length() - to);
	from = from + replacement.length() - 1;
}


void WIKI2XML::parse_link(string & l, int &from, char mode)
{
	int a, cnt = 1;
	chart par_open = '[';	// mode 'L'
	chart par_close = ']';	// mode 'L'
	if (mode == 'T')
	{
		par_open = '{';
		par_close = '}';
	}

	for (a = from + 2; a + 1 < l.length(); a++)
	{
		if (l[a] == par_open)
		{
			from = a - 2;
			return;
		}
		if (l[a] == '|' || (l[a] == par_close && l[a + 1] == par_close))
		{
			break;
		}
	}

	for (a = from + 1; cnt > 0 && a + 1 < l.length(); a++)
	{
		if (l[a] == par_open && l[a + 1] == par_open)
		{
			parse_link(l, a);
		}
		else if (l[a] == par_close && l[a + 1] == par_close)
		{
			cnt--;
		}
	}

	if (cnt > 0)
	{
		return;			// Not a valid link
	}

	int to = a - 1;		// Without "]]"
	string link = l.substr(from + 2, to - from - 2);

	TXML x;
	vector < string > parts;
	explode('|', link, parts);
	if (mode == 'L')
	{
		x.name = "wl";
		x.add_key_value("type", "int");
	}
	else if (mode == 'T')
		x.name = "wtemplate";

	for (a = 0; a < parts.size(); a++)
	{
		bool last = (a == parts.size()-1);
		string p = parts[a];

		if ((a > 0) && (mode == 'T'))
		{
			string key, value;
			vector<string> subparts;
			explode('=', p, subparts);
			if (subparts.size() == 1)
			{
				value = xml_embed(p, "val");
			}
			else
			{
				key = xml_embed(subparts[0], "key");
				subparts.erase(subparts.begin());
				value = xml_embed(implode("=", subparts), "val");
			}
			p = key + value;
		}
		else
			p = xml_embed(p, "val");

		string param = "number=\"" + val(a) + "\"";
		if (last)
			param += " last=\"1\"";
		x.text += xml_embed(p, "wp", param);
	}

	if (mode == 'L')		// Try link trail
	{
		string trail;
		for (a = to + 2; a < l.length() && is_text_char(l[a]); a++) // TODO: is_text_char() verbessern
			trail += l[a];
		to = a - 2;
		if (trail != "")
			x.text += xml_embed(trail, "trail");
	}

	x.add_key_value("params", val(parts.size()));
	string replacement = x.get_string();
	parse_line_sub(replacement);

	l.erase(from, to - from + 2);
	l.insert(from, replacement);

	from = from + replacement.length() - 1;

	if (debug)
		cout << "Link : " << link << endl << "Replacement : " <<
				replacement << endl;
	if (debug)
		cout << "Result : " << l << endl << endl;
}


void WIKI2XML::parse_line_sub(string & l)
{
	int a;
	for (a = 0; a < l.length(); a++)
	{
		if (l[a] == '[' && a + 1 < l.length() && l[a + 1] == '[')	// [[Link]]
		{
			parse_link(l, a, 'L');
		}
		else if (l[a] == '{' && a + 1 < l.length() && l[a + 1] == '{')	// {{Template}}
		{
			parse_link(l, a, 'T');
		}
		else if (l[a] == '[')	// [External link]
		{
			parse_external_link(l, a);
		}
		else if (a + 2 < l.length() && l[a] == ':' && l[a + 1] == '/' && l[a + 2] == '/')	// External freelink
		{
			parse_external_freelink(l, a);
		}
	}
}


void WIKI2XML::parse_line(string & l)
{
	PA_ClearTextBg(1);
	PA_OutputText(1,0,2,"->%s<-",l.c_str());
// 	PA_WaitFor(Pad.Newpress.Anykey);

	if (debug)
		cout << l << endl;

	doQuotes(l);

	int a, b;
	string pre;
	string oldlist = list;
	pre += fix_list(l);
	if (list != "" && list == oldlist)
	{
		string itemname = "li";
		if (right(list, 1) == ":")
			itemname = "dd";
		pre = "</" + itemname + "><" + itemname + ">" + pre;
	}

	if (l == "")	// Paragraph
	{
		l = "<p />";
	}
	else if (left(l, 4) == "----")	// <hr>
	{
		for (a = 0; a < l.length() && l[a] == l[0]; a++);
		pre += "<wuc action=\"reset\"/><hr />"; //wikiurlcounter
		l = l.substr(a, l.length() - a);
	}
	else if (l[0] == '=')	// Heading
	{
		for (a = 0; a < l.length() && l[a] == '=' && l[l.length() - a - 1] == '='; a++);
		string h = "h0";
		if (a >= l.length())
			h = "";		// No heading
		else if (a < 1 || a > 9)
			h = "";
		if (h != "")
		{
			l = l.substr(a, l.length() - a * 2);
			h[1] += a;
			l = xml_embed(l, h);
		}
	}
	else if (l[0] == ' ')	// Pre-formatted text
	{
		for (a = 0; a < l.length() && l[a] == ' '; a++);
		if (a==l.length())
		{
			l = "<pre></pre>";
		}
		else
		{
			string spaces = l.substr(1,a-1);
			l = l.substr(a,l.length()-a);
			parse_line_sub(l);
			pre += "<pre>" + spaces + l + "</pre>";
			l = "";
		}
	}
	else if ( (left(l, 2) == "{|")
				  || left(l, 2) == "|}"
				  || (tables.size() > 0 && l != "" && (l[0] == '|' || l[0] == '!'))
			)
	{
		pre += table_markup(l);
		l = "";
	}

	if (l != "")
		parse_line_sub(l);

	if (pre != "")
		l = pre + l;
}


void WIKI2XML::parse(string & s)
{
	PA_Clear16bitBg(0);
	SimPrint(s,&DnScreen,PA_RGB(0,0,0),UTF8);
	PA_WaitFor(Pad.Newpress.Anykey);

	int a,b,c,d;

	// In order to get a valid XML-document, we have to treat all environments
	// which may legally contain unmasked XML-entities ('&','<','>','"',''').
	// The biggest problem for make_tags are '<' and '>'.
	// These environments are separated out. Later they get recombined with
	// the rest of the text. These environments are: nowiki, math, pre, source
	string substring;
	nowiki_contents.clear();
	a = s.find("<nowiki>");
	while (a != string::npos)
	{
		b = s.find("</nowiki>",a);
		if (b == string::npos)
			b = s.length()+1;
		substring = s.substr(a+8,b-a-8);
		substring = exchangeSGMLEntities(substring); // &amp; -> &
		// mask all XML-entities
		replace_all(substring,"&","&amp;");
		replace_all(substring,"<","&lt;");
		replace_all(substring,">","&gt;");
		replace_all(substring,"\"","&quot;");
		replace_all(substring,"\'","&apos;");
		replace_all(substring,"\n"," ");
		trim(substring);
		nowiki_contents.push_back(substring);
		substring.clear();
		replace_part(s, a, b+8, dswiki_magic_phrase+FromUTF(magic_offset+nowiki_contents.size()-1));
		a = s.find("<nowiki>",a+1);
	}

	math_contents.clear();
	a = s.find("<math>");
	while (a != string::npos)
	{
		b = s.find("</math>",a);
		if (b == string::npos)
			b = s.length()+1;
		substring = s.substr(a+6,b-a-6);
		replace_all(substring,"&","&amp;");
		for (c=0;c<nowiki_contents.size();c++)
		{
			replace_all(substring,dswiki_magic_phrase+FromUTF(magic_offset+c),dswiki_magic_nowiki_open+nowiki_contents[c]+dswiki_magic_nowiki_close);
		}
		replace_all(substring,"<","&lt;");
		replace_all(substring,">","&gt;");
		replace_all(substring,"\"","&quot;");
		replace_all(substring,"\'","&apos;");
		replace_all(substring,"\n"," ");
		trim(substring);
		math_contents.push_back(substring);
		substring.clear();
		replace_part(s, a, b+6, dswiki_magic_phrase+FromUTF(magic_offset+nowiki_contents.size()+math_contents.size()-1));
		a = s.find("<math>",a+1);
	}

	pre_contents.clear();
	a = s.find("<pre>");
	while (a != string::npos)
	{
		b = s.find("</pre>",a);
		if (b == string::npos)
			b = s.length()+1;
		substring = s.substr(a+5,b-a-5);
		substring = exchangeSGMLEntities(substring); // &amp; -> &
		replace_all(substring,"&","&amp;");
		for (c=0;c<math_contents.size();c++)
		{
			replace_all(substring,dswiki_magic_phrase+FromUTF(magic_offset+nowiki_contents.size()+c),"<math>"+math_contents[c]+"</math>");
		}
		// special treatment for nowiki in math (when both in pre)
		replace_all(substring,dswiki_magic_nowiki_open,"");
		replace_all(substring,dswiki_magic_nowiki_close,"");
		for (c=0;c<nowiki_contents.size();c++)
		{
			replace_all(substring,dswiki_magic_phrase+FromUTF(magic_offset+c),nowiki_contents[c]);
		}
		replace_all(substring,"<","&lt;");
		replace_all(substring,">","&gt;");
		replace_all(substring,"\"","&quot;");
		replace_all(substring,"\'","&apos;");
		pre_contents.push_back(substring);
		replace_part(s, a, b+5, dswiki_magic_phrase+FromUTF(magic_offset+nowiki_contents.size()+math_contents.size()+pre_contents.size()-1));
		a = s.find("<pre>",a+1);
	}


	// every '<source ...>' tag we find here, is NOT contained in a nowiki, math or pre environment, that's sure
	source_contents.clear();
	a = s.find("<source");
	while (a != string::npos)
	{
		c = s.find(">",a);
		if (c==string::npos)
			break;
		b = s.find("</source>",c);
		if (b == string::npos)
			b = s.length()+1;
		substring = s.substr(c+1,b-c-1);
		replace_all(substring,"&","&amp;");
		for (d=0;d<pre_contents.size();d++)
		{
			replace_all(substring,dswiki_magic_phrase+FromUTF(magic_offset+nowiki_contents.size()+math_contents.size()+d),"&lt;pre&gt;"+pre_contents[d]+"&lt;/pre&gt;");
		}
		for (d=0;d<math_contents.size();d++)
		{
			replace_all(substring,dswiki_magic_phrase+FromUTF(magic_offset+nowiki_contents.size()+d),"&lt;math&gt;"+math_contents[d]+"&lt;/math&gt;");
		}
		for (d=0;d<nowiki_contents.size();d++)
		{
			replace_all(substring,dswiki_magic_phrase+FromUTF(magic_offset+d),"&lt;nowiki&gt;"+nowiki_contents[d]+"&lt;/nowiki&gt;");
		}
		replace_all(substring,"<","&lt;");
		replace_all(substring,">","&gt;");
		replace_all(substring,"\"","&quot;");
		replace_all(substring,"\'","&apos;");
		source_contents.push_back(s.substr(a+7,c-a-6)+substring);
		replace_part(s, a, b+8, dswiki_magic_phrase+FromUTF(magic_offset+nowiki_contents.size()+math_contents.size()+pre_contents.size()+source_contents.size()-1));
		a = s.find("<source",a+1);
	}

	// Remove HTML-Comments
	a = s.find("<!--");
	while (a != string::npos)
	{
		b = s.find("-->",a);
		if (b == string::npos)
			b = s.length() - 1;
		else
			b += 2;

		replace_part(s, a, b, "");
		a = s.find("<!--",a+1);
	}
	// everything was separated out

	PA_Clear16bitBg(0);
	SimPrint(s,&DnScreen,PA_RGB(0,0,0),UTF8);
	PA_WaitFor(Pad.Newpress.Anykey);

	// In the remaining text, we exchange all uncritical named SGML-entities
	s = exchangeSGMLEntities(s,true);

	PA_Clear16bitBg(0);
	SimPrint(s,&DnScreen,PA_RGB(0,0,0),UTF8);
	PA_WaitFor(Pad.Newpress.Anykey);

	// Help 'make_tags' and mask literal '&' and '<'
	a = s.find("&");
	while (a != string::npos)
	{
		for (b=0;b<MAX_NAMED_ENTITIES;b++)
		{
			if (s.substr(a,entities[b].entity.length()) == entities[b].entity)
				break;
		}
		if (b==MAX_NAMED_ENTITIES)
		{
			s.replace(a,1,"&amp;");
		}
		a = s.find("&",a+1);
	}
	a = s.find("<");
	while (a != string::npos)
	{
		for (b=0;b<allowed_html.size();b++)
		{
			if ((s.substr(a+1,allowed_html[b].length()) == allowed_html[b])
						  ||(s.substr(a+1,allowed_html[b].length()+1) == "/"+allowed_html[b]))
				break;
		}
		if (b==allowed_html.size())
		{
			s.replace(a,1,"&lt;");
		}
		a = s.find("<",a+1);
	}

	PA_Clear16bitBg(0);
	SimPrint(s,&DnScreen,PA_RGB(0,0,0),UTF8);
	PA_WaitFor(Pad.Newpress.Anykey);

	vector<TXML> taglist;
	make_tag_list(s, taglist);
	sanitize_html(s, taglist);

    // Now evaluate each line
	explode('\n', s, lines);

	for (a = 0; a < lines.size(); a++)
	{
		parse_line(lines[a]);
	}

	string end;

    // Cleanup lists
	end = fix_list(end);
	if (end != "")
		lines.push_back(end);

    // Cleanup tables
	end = "";
	while (tables.size()) {
		end += tables[tables.size() - 1].close();
		tables.pop_back();
	}
	if (end != "")
		lines.push_back(end);

	// recombine
	for (b=0;b<lines.size();b++)
	{
		for (a=0;a<source_contents.size();a++)
		{
			replace_all(lines[b],dswiki_magic_phrase+FromUTF(magic_offset+nowiki_contents.size()+math_contents.size()+pre_contents.size()+a),
					"<source"+source_contents[a]+"</source>");
		}
	}
	for (b=0;b<lines.size();b++)
	{
		for (a=0;a<pre_contents.size();a++)
		{
			replace_all(lines[b],dswiki_magic_phrase+FromUTF(magic_offset+nowiki_contents.size()+math_contents.size()+a),"<pre>"+pre_contents[a]+"</pre>");
		}
	}
	for (b=0;b<lines.size();b++)
	{
		for (a=0;a<math_contents.size();a++)
		{
			replace_all(lines[b],dswiki_magic_phrase+FromUTF(magic_offset+nowiki_contents.size()+a),"<math>"+math_contents[a]+"</math>");
		}
	}
	for (b=0;b<lines.size();b++)
	{
		for (a=0;a<nowiki_contents.size();a++)
		{
			replace_all(lines[b],dswiki_magic_phrase+FromUTF(magic_offset+a),nowiki_contents[a]);
		}
	}
	for (b=0;b<lines.size();b++)
	{
		replace_all(lines[b],dswiki_magic_nowiki_open,"&lt;nowiki&gt;"); // normal treatment for nowiki in math
		replace_all(lines[b],dswiki_magic_nowiki_close,"&lt;/nowiki&gt;");
	}
}


WIKI2XML::WIKI2XML()
{
	list = "";
	lines.clear();

    // Now we remove evil HTML
	allowed_html.clear();

	allowed_html.push_back("p");
	allowed_html.push_back("br");
	allowed_html.push_back("hr");
	allowed_html.push_back("div");
	allowed_html.push_back("span");

	allowed_html.push_back("i");
	allowed_html.push_back("b");
	allowed_html.push_back("u");
	allowed_html.push_back("tt");
	allowed_html.push_back("small");
	allowed_html.push_back("strike");
	allowed_html.push_back("center");

	allowed_html.push_back("code");
	allowed_html.push_back("nowiki");
	allowed_html.push_back("pre");
	allowed_html.push_back("source");
	allowed_html.push_back("math");
	allowed_html.push_back("noinclude");
	allowed_html.push_back("ref");
	allowed_html.push_back("references");

	allowed_html.push_back("table");
	allowed_html.push_back("caption");
	allowed_html.push_back("tr");
	allowed_html.push_back("td");
	allowed_html.push_back("th");

	allowed_html.push_back("ul");
	allowed_html.push_back("ol");
	allowed_html.push_back("li");

	allowed_html.push_back("dl");
	allowed_html.push_back("dd");
	allowed_html.push_back("dt");

	allowed_html.push_back("h1");
	allowed_html.push_back("h2");
	allowed_html.push_back("h3");
	allowed_html.push_back("h4");
	allowed_html.push_back("h5");
	allowed_html.push_back("h6");
	allowed_html.push_back("h7");
	allowed_html.push_back("h8");
	allowed_html.push_back("h9");

	allowed_html.push_back("!--");
}


void WIKI2XML::get_xml()
{
	FILE* xmlout = fopen("fat:/dswiki/article.xml","wb");
	if (xmlout!=NULL)
	{
		string s = "<?xml version='1.0' encoding='UTF-8'?>\n<text>\n";
		fwrite(s.c_str(),strlen(s.c_str()),1,xmlout);
		for (int a=0; a<lines.size(); a++)
		{
			s = lines[a]+"\n";
			fwrite(s.c_str(),strlen(s.c_str()),1,xmlout);
		}
		s = "</text>\n";
		fwrite(s.c_str(),strlen(s.c_str()),1,xmlout);
		fclose(xmlout);
// 		PA_OutputText(1,0,10,"write OK");
	}
	else
	{
// 		PA_OutputText(1,0,10,"write error");
	}
}


void WIKI2XML::replace_part(string & s, int from, int to, string with)
{
	s = s.substr(0, from) + with + s.substr(to + 1, s.length() - to - 1);
}


void WIKI2XML::replace_part_sync(string & s, int from, int to, string with,
								 vector < TXML > &list)
{
	int a, b;
	replace_part(s, from, to, with);
	for (a = 0; a < list.size(); a++)
	{
		for (b = 0; b < with.length(); b++)
			list[a].insert_at(from);
		for (b = from; b <= to; b++)
			list[a].remove_at(from);
	}
}


void WIKI2XML::make_tag_list(string & s, vector < TXML > &list)
{
	list.clear();
	int a, b;
	for (a = 0; a < s.length(); a++)
	{
		if (s[a] == '>')	// Rogue >
		{
			s[a] = ';';
			s.insert(a, "&gt");
			continue;
		}
		else if (s[a] != '<')
			continue;
		b = find_next_unquoted('>', s, a);
		if (b == -1)		// Rogue <
		{
			s[a] = ';';
			s.insert(a, "&lt");
			continue;
		}
		list.push_back(TXML(a, b, s));
		a = list[list.size() - 1].to;
	}
}


void WIKI2XML::sanitize_html(string & s, vector < TXML > &taglist)
{
	int a, b;
	for (a = 0; a < taglist.size(); a++)
	{
		string tag = taglist[a].name;

		for (b = 0; b < allowed_html.size() && tag != allowed_html[b]; b++);

		if (b < allowed_html.size()) // Allowed tag
		{
			if (taglist[a].closing && taglist[a].selfclosing) // Correct closing and selfclosing tag
			{
				if ((tag=="br") || (tag=="hr"))
				{
					replace_part_sync(s, taglist[a].from, taglist[a].from+1, "<", taglist);
					taglist[a].to--;
					taglist[a].closing = false;
				}
				else
				{
					replace_part_sync(s, taglist[a].to-1, taglist[a].to, ">", taglist);
					taglist[a].to--;
					taglist[a].selfclosing = false;
				}
			}
			if ((tag=="br") || (tag=="hr"))
			{
				if (!taglist[a].selfclosing)
				{
					replace_part_sync(s, taglist[a].to, taglist[a].to, " />", taglist);
					taglist[a].selfclosing = true;
				}
			}
			continue;
		}
		replace_part_sync(s, taglist[a].from, taglist[a].from, "&lt;", taglist);
		replace_part_sync(s, taglist[a].to,   taglist[a].to,   "&gt;", taglist);
	}
}


string WIKI2XML::table_markup(string & l)
{
	int a;
	string ret;
	if (left(l, 2) == "{|")	// Open table
	{
		ret = "<table>";
		ret += xml_embed(l.substr(2, l.length() - 2), "wp");
		tables.push_back(TTableInfo());
	}
	else if (!tables.empty())
	{
		if (left(l, 2) == "|}") // Close table
		{
			ret = tables[tables.size() - 1].close();
			tables.pop_back();
		}
		else if (left(l, 2) == "|-") // New row
		{
			ret = tables[tables.size() - 1].new_row();
			for (a = 1; a < l.length() && l[a] == '-'; a++);
			ret += xml_params(l.substr(a, l.length() - a));
		}
		else
		{
			string init;
			if (left(l, 2) == "|+")
			{
				init = "caption";
				l = l.substr(2, l.length() - 2);
			}
			else if (l[0] == '!')
			{
				init = "header";
				l = l.substr(1, l.length() - 1);
			}
			else if (l[0] == '|')
			{
				init = "cell";
				l = l.substr(1, l.length() - 1);
			}
			vector < string > sublines;
			for (a = 0; a + 1 < l.length(); a++)
			{
				if (l[a] == '|' && l[a + 1] == '|')
				{
					sublines.push_back(left(l, a));
					l = l.substr(a + 2, l.length() - a);
					a = -1;
				}
			}
			if (l != "")
				sublines.push_back(l);
			for (a = 0; a < sublines.size(); a++)
			{
				l = sublines[a];
				parse_line_sub(l);
				string params;
				int b = find_next_unquoted('|', l);
				if (b != -1) {
					params = left(l, b);
					l = l.substr(b + 1, l.length() - b);
				}
				if (params != "")
					l = xml_params(params) + l;
				ret += tables[tables.size() - 1].new_cell(init);
				ret += l;
			}
		}
	}
	else
	{
		ret = l;
	}

	return ret;
}
