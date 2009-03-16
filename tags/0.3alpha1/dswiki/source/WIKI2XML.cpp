#include "WIKI2XML.h"
#include "main.h"
#include "char_convert.h"
#include "fat.h"
#include "efs_lib.h"
#include "Globals.h"
#include "WikiMarkupGetter.h"
#include "PercentIndicator.h"
#include "Statusbar.h"
#include "TitleIndex.h"

TTableInfo::TTableInfo()
{
	tr_open = false;
	td_open = false;
	th_open = false;
}

string TTableInfo::close()
{
	string ret;
	if (th_open)
		ret += "</th>";
	if (td_open)
		ret += "</td>";
	if (tr_open)
		ret += "</tr>";
	ret += "</table>";
	return ret;
}

string TTableInfo::new_row(string attributes)
{
	string ret;
	if (th_open)
		ret += "</th>";
	if (td_open)
		ret += "</td>";
	if (tr_open)
		ret += "</tr>";
	ret += xml_embed("", "tr", attributes, true);
	th_open = false;
	td_open = false;
	tr_open = true;
	return ret;
}

string TTableInfo::new_cell(string attributes)
{
	string ret;
	if ((!tr_open) || th_open)
		ret += new_row();
	if (td_open)
		ret += "</td>";
	ret += xml_embed("", "td", attributes, true);
	th_open = false;
	td_open = true;
	return ret;
}

string TTableInfo::new_headercell(string attributes)
{
	string ret;
	if ((!tr_open) || td_open)
		ret += new_row();
	if (th_open)
		ret += "</th>";
	ret += xml_embed("", "th", attributes, true);
	th_open = true;
	td_open = false;
	return ret;
}

string TTableInfo::new_caption(string caption, string attributes)
{
	string ret;
	if (th_open)
		ret += "</th>";
	if (td_open)
		ret += "</td>";
	if (tr_open)
		ret += "</tr>";
	trim(caption);
	ret += xml_embed(caption, "caption", attributes);
	th_open = false;
	td_open = false;
	tr_open = false;
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
	if (l.empty())
		return;
	int a,b;
	vector < string > parts;

	b = l.find("''");
	if (b== (int) string::npos)
		return;
	parts.push_back(l.substr(0,b));
	a = b;
	while (a < (int) l.length())
	{
		for (b=a;b< (int) l.length() && l[b] == '\''; b++);
		parts.push_back(l.substr(a,b-a));
		a = b;
		b = l.find("''",a);
		if (b!= (int) string::npos)
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

	for (a=1;a<(int) parts.size();a+=2)
	{
		if (parts[a].length() == 4)
		{
			parts[a-1] += "&apos;";
			parts[a] = "'''";
		}
		else if (parts[a].length() > 5)
		{
			for (b=0;b< (int) parts[a].length() - 5;b++)
				parts[a-1] += "&apos;";
			parts[a] = "'''''";
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
		for ( a = 1; a < (int) parts.size(); a += 2 )
		{
			if ( parts[a].length() == 3 )
			{
				string x1, x2;
				x1 = parts[a-1].substr(parts[a-1].length()-1);
				if (parts[a-1].length()>1)
					x2 = parts[a-1].substr(parts[a-1].length()-2,1);
				else
					x2 = "";

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
			parts[firstsingleletterword - 1] += "&apos;";
			parts[firstsingleletterword] = "''";
		}
		// If not, but there's a multi-letter word, use that one.
		else if (firstmultiletterword > -1)
		{
			parts[firstmultiletterword - 1] += "&apos;";
			parts[firstmultiletterword] = "''";
		}
		// ... otherwise use the first one that has neither.
		// (notice that it is possible for all three to be -1 if, for example,
		// there is only one pentuple-apostrophe in the line)
		else if (firstspace > -1)
		{
			parts[ firstspace-1 ] += "&apos;";
			parts[ firstspace ] = "''";
		}

	}

	l.clear();
	string buffer = "";
	string state  = "";

	for (a = 0; a < (int) parts.size(); a++)
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
					state.clear();
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
					state.clear();
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
					state.clear();
				}
				else if (state == "ib")
				{
					l += "</b></i>";
					state.clear();
				}
				else if (state == "both")
				{
					l += "<i><b>" + buffer + "</b></i>";
					state.clear();
				}
				else // (state == "")
				{
					buffer.clear();
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


bool WIKI2XML::is_list_char(chart c)
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
	if (c == '*')
		ret = "ul";
	if (c == '#')
		ret = "ol";
	if (c == ':')
		ret = "dl";
	if (!ret.empty())
	{
		string itemname = "li";
		if (c == ':')
		{
			itemname = "dd";
		}

		if (open)
		{
			ret = "<" + ret + "><" + itemname + ">";
		}
		else
		{
			ret = "</" + itemname + "></" + ret + ">";
		}
	}
	return ret;
}


string WIKI2XML::fix_list(string & l)
{
	int a, common;
	string newlist, pre;

	for (a = 0; a < (int) l.length() && is_list_char(l[a]); a++);

	// split l into list leading list characters (->newlist) and the rest (->l)
	if (a > 0)
	{
		newlist = left(l, a);
		while (a < (int) l.length() && l[a] == ' ')
			a++;		// Removing leading blanks
		l.erase(0,a);
	}

	if (list.empty() && newlist.empty())
		return "";

	bool someClosed = false;
	bool someOpened = false;

	// The common part, if any
	for (common = 0; common < (int) list.length() && common < (int) newlist.length() && list[common] == newlist[common]; common++);

	if ( common < (int) list.length() )
	{
		someClosed = true;
		for (a = common; a < (int) list.length(); a++)
			pre.insert(0, get_list_tag(list[a], false));	// Close old list tags
	}

	if ( common < (int) newlist.length() )
	{
		someOpened = true;
		for (a = common; a < (int) newlist.length(); a++)
			pre += get_list_tag(newlist[a], true);	// Open new ones
	}

	if ( !newlist.empty() && (!someOpened) )
	{
		string itemname = "li";
		if (right(newlist, 1) == ":")
			itemname = "dd";
		pre += "</" + itemname + "><" + itemname + ">";
	}

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
	for (a = from; a < (int) l.length(); a++)
	{
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
	if (!is_external_link_protocol(protocol))
		return;
	int to = scan_url(l, a);
	string url = l.substr(a, to - a);
	string replacement;
	replacement += xml_embed(url, "url");
	replacement += xml_embed(url, "title");
	replacement = xml_embed(replacement, "wl", "type=\"ext\" protocol=\"" + protocol + "\"");
	replace_part(l, a, to, replacement);
	from = a + replacement.length() - 1;
}


void WIKI2XML::parse_external_link(string & l, int &from)
{
	string protocol = upper(before_first(':', l.substr(from + 1, l.length() - from)));
	if (!is_external_link_protocol(protocol))
		return;
	int to;
	for (to = from + 1; to < (int) l.length() && l[to] != ']'; to++);
	if (to == (int) l.length())
		return;
	string url = l.substr(from + 1, to - from - 1);
	string title = after_first(' ', url);
	url = before_first(' ', url);
	string replacement;
	replacement += xml_embed(url, "url");
	if (title.empty())
		replacement += xml_embed(url, "title");
	else
		replacement += xml_embed(title, "title");

	replacement = xml_embed(replacement, "wl", "type=\"ext\" protocol=\"" + protocol + "\"");
	replace_part(l, from, to, replacement);
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

	// Scan the target for disallowed characters,
	// stop when the alternative text or the end is reached
	for (a = from + 2; a + 1 < (int) l.length(); a++)
	{
		if (l[a] == '<')
			return;
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

	for (a = from + 1; cnt > 0 && a + 1 < (int) l.length(); a++)
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
	if (mode == 'L')
	{
		string lowerImageNamespace = lowerPhrase(_globals->getTitleIndex()->ImageNamespace());

		for (a=0; a < (int) link.length() && link[a]==' '; a++);
		if (lowerPhrase(link.substr(a,lowerImageNamespace.length())) == lowerImageNamespace)
		{
			a += lowerImageNamespace.length();
			for (a=a; a < (int) link.length() && link[a]==' '; a++);
			if (link[a] == ':')
			{
				x.name = "wi";
				link.erase(0,a+1);
			}
			else
			{
				x.name = "wl";
			}
		}
		else
		{
			x.name = "wl";
		}
	}
	else if (mode == 'T')
	{
		x.name = "wt";
	}

	vector < string > parts;
	explode("|", link, parts);

	for (a = 0; a < (int) parts.size(); a++)
	{
		string p = parts[a];

		if (a==0)
		{
			trim(p);
			trimDoubleSpaces(p);
		}

		if ((a > 0) && (mode == 'T'))
		{
			string key, value;
			vector<string> subparts;
			explode("=", p, subparts);
			if (subparts.size() == 1)
			{
				value = xml_embed(p, "val");
			}
			else
			{
				key = xml_embed(subparts[0], "key");
				subparts.erase(subparts.begin());
				implode("=", subparts, value);
				value = xml_embed(value, "val");
			}
			p = key + value;
		}

		x.text += xml_embed(p, "wp");
	}

	if (mode == 'L')		// Try link trail
	{
		string trail;
		for (a = to + 2; a < (int) l.length() && is_text_char(l[a]); a++)
			trail += l[a];
		to = a - 2;
		if (!trail.empty())
			x.text += xml_embed(trail, "trail");
	}

	string replacement = x.get_string();
	parse_line_sub(replacement);

	replace_part(l, from, to + 1, replacement);

	from = from + replacement.length() - 1;
}


void WIKI2XML::parse_line_sub(string & l)
{
	int a;
	for (a = 0; a < (int) l.length(); a++)
	{
		if (l[a] == '[' && a + 1 < (int) l.length() && l[a + 1] == '[')	// [[Link]]
		{
			parse_link(l, a, 'L');
		}
		else if (l[a] == '{' && a + 1 < (int) l.length() && l[a + 1] == '{')	// {{Template}}
		{
			parse_link(l, a, 'T');
		}
		else if (l[a] == '[')	// [External link]
		{
			parse_external_link(l, a);
		}
		else if (a + 2 < (int) l.length() && l[a] == ':' && l[a + 1] == '/' && l[a + 2] == '/')	// External freelink
		{
			parse_external_freelink(l, a);
		}
	}
}


void WIKI2XML::parse_line(string & l)
{
	int a;
	string pre;

	pre += fix_list(l);

	doQuotes(l);

	if (l.empty())	// Paragraph
	{
		l += "<p />";
	}
	else if (left(l, 4) == "----")	// <hr>
	{
		for (a = 0; a < (int) l.length() && l[a] == l[0]; a++);
		pre += "<hr />";
		l.erase(0,a);
	}
	else if (l[0] == '=')	// Heading
	{
		for (a = 0; a < (int) l.length() && l[a] == '=' && l[l.length() - a - 1] == '='; a++);
		string h = "h0";
		if (a >= (int) l.length())
			h.clear();		// No heading
		else if (a < 1 || a > 9)
			h.clear();
		if (!h.empty())
		{
			l.erase(0,a);
			l.erase(l.length() - a);
			trim(l);
			h[1] += a;
			l = xml_embed(l, h);
		}
	}
	else if (l[0] == ' ')	// Pre-formatted text
	{
		for (a = 0; a < (int) l.length() && l[a] == ' '; a++);
		if (a== (int) l.length())
		{
			l.clear();
			l += "<pre></pre>";
		}
		else
		{
			string spaces = l.substr(1,a-1);
			l.erase(0,a);
			parse_line_sub(l);
			pre += xml_embed(spaces + l, "pre");
			l.clear();
		}
	}
	else if ( (left(l, 2) == "{|") || (left(l, 2) == "|}") || (tables.size() > 0 && (!l.empty()) && (l[0] == '|' || l[0] == '!')) )
	{
		pre += table_markup(l);
		l.clear();
	}

	if (!l.empty())
		parse_line_sub(l);

	if (!pre.empty())
		l.insert(0,pre);
}


void WIKI2XML::parse(string & s, int type)
{
	int a,b,c,d;
	string substring;

	switch (type)
	{
		default:
		case FULL_PARSE:
		{
			// In order to get a valid XML-document, we have to treat all environments
			// which may legally contain unmasked XML-entities ('&','<','>','"',''').
			// The biggest problem for make_tags are '<' and '>'.
			// These environments are separated out. Later they get recombined with
			// the rest of the text. These environments are: nowiki, math, pre, source
			nowiki_contents.clear();
			a = s.find("<nowiki>");
			while (a != (int) string::npos)
			{
				b = s.find("</nowiki>",a);
				if (b == (int) string::npos)
					b = s.length()+1;
				substring = s.substr(a+8,b-a-8);
				exchangeSGMLEntities(substring); // &amp; -> &
				// mask all XML-entities
				replace_all(substring,"&","&amp;");
				replace_all(substring,"<","&lt;");
				replace_all(substring,">","&gt;");
				replace_all(substring,"\"","&quot;");
				replace_all(substring,"'","&apos;");
				replace_all(substring,"\n"," ");
				trimDoubleSpaces(substring);
				nowiki_contents.push_back(substring);
				substring.clear();
				replace_part(s, a, b+8, dswiki_magic_phrase+FromUTF(magic_offset+nowiki_contents.size()-1));
				a = s.find("<nowiki>",a+1);
			}

			math_contents.clear();
			a = s.find("<math>");
			while (a != (int) string::npos)
			{
				b = s.find("</math>",a);
				if (b == (int) string::npos)
					b = s.length()+1;
				substring = s.substr(a+6,b-a-6);
				replace_all(substring,"&","&amp;");
				for (c=0;c< (int) nowiki_contents.size();c++)
				{
					replace_all(substring,dswiki_magic_phrase+FromUTF(magic_offset+c),dswiki_magic_nowiki_open+nowiki_contents[c]+dswiki_magic_nowiki_close);
				}
				replace_all(substring,"<","&lt;");
				replace_all(substring,">","&gt;");
				replace_all(substring,"\"","&quot;");
				replace_all(substring,"'","&apos;");
				replace_all(substring,"\n"," ");
				trim(substring);
				trimDoubleSpaces(substring);
				math_contents.push_back(substring);
				substring.clear();
				replace_part(s, a, b+6, dswiki_magic_phrase+FromUTF(magic_offset+nowiki_contents.size()+math_contents.size()-1));
				a = s.find("<math>",a+1);
			}

			pre_contents.clear();
			a = s.find("<pre>");
			while (a != (int) string::npos)
			{
				b = s.find("</pre>",a);
				if (b == (int) string::npos)
					b = s.length()+1;
				substring = s.substr(a+5,b-a-5);
				exchangeSGMLEntities(substring); // &amp; -> &
				replace_all(substring,"&","&amp;");
				for (c=0;c< (int) math_contents.size();c++)
				{
					replace_all(substring,dswiki_magic_phrase+FromUTF(magic_offset+nowiki_contents.size()+c),"<math>"+math_contents[c]+"</math>");
				}
				// special treatment for nowiki in math (when both in pre)
				replace_all(substring,dswiki_magic_nowiki_open,"");
				replace_all(substring,dswiki_magic_nowiki_close,"");
				for (c=0;c< (int) nowiki_contents.size();c++)
				{
					replace_all(substring,dswiki_magic_phrase+FromUTF(magic_offset+c),nowiki_contents[c]);
				}
				replace_all(substring,"<","&lt;");
				replace_all(substring,">","&gt;");
				replace_all(substring,"\"","&quot;");
				replace_all(substring,"'","&apos;");
				trimSpacesBeforeLinebreaks(substring);
				pre_contents.push_back(substring);
				replace_part(s, a, b+5, dswiki_magic_phrase+FromUTF(magic_offset+nowiki_contents.size()+math_contents.size()+pre_contents.size()-1));
				a = s.find("<pre>",a+1);
			}


			// every '<source ...>' tag we find here, is NOT contained in a nowiki, math or pre environment, that's sure
			source_contents.clear();
			a = s.find("<source");
			while (a != (int) string::npos)
			{
				c = s.find(">",a);
				if (c== (int) string::npos)
					break;
				b = s.find("</source>",c);
				if (b == (int) string::npos)
					b = s.length()+1;
				substring = s.substr(c+1,b-c-1);
				replace_all(substring,"&","&amp;");
				for (d=0;d< (int) pre_contents.size();d++)
				{
					replace_all(substring,dswiki_magic_phrase+FromUTF(magic_offset+nowiki_contents.size()+math_contents.size()+d),"&lt;pre&gt;"+pre_contents[d]+"&lt;/pre&gt;");
				}
				for (d=0;d< (int) math_contents.size();d++)
				{
					replace_all(substring,dswiki_magic_phrase+FromUTF(magic_offset+nowiki_contents.size()+d),"&lt;math&gt;"+math_contents[d]+"&lt;/math&gt;");
				}
				for (d=0;d< (int) nowiki_contents.size();d++)
				{
					replace_all(substring,dswiki_magic_phrase+FromUTF(magic_offset+d),"&lt;nowiki&gt;"+nowiki_contents[d]+"&lt;/nowiki&gt;");
				}
				replace_all(substring,"<","&lt;");
				replace_all(substring,">","&gt;");
				replace_all(substring,"\"","&quot;");
				replace_all(substring,"'","&apos;");
				trimSpacesBeforeLinebreaks(substring);
				source_contents.push_back(s.substr(a+7,c-a-6)+substring);
				replace_part(s, a, b+8, dswiki_magic_phrase+FromUTF(magic_offset+nowiki_contents.size()+math_contents.size()+pre_contents.size()+source_contents.size()-1));
				a = s.find("<source",a+1);
			}

			// In the remaining text, we exchange all uncritical named SGML-entities,
			// As, for example, "&apos;" should not be interpretated as wiki-markup, we must exclude it
			exchangeSGMLEntities(s,true);

			// remove linebreaks from within multi-line template tags, so that parse_line can process them as a normal tag
			a = s.find("{{");
			while (a != (int) string::npos)
			{
				int open = 1;
				for (b=a+2; b+1 < (int) s.length(); b++)
				{
					if (s[b] == '{' && s[b+1] == '{')
					{
						open++;
						b+=2;
					}
					if (s[b] == '}' && s[b+1] == '}')
					{
						open--;
						b+=2;
					}
					if (open==0)
					{
						substring = s.substr(a+2,b-a-4);
						replace_all(substring,"\n"," ");
						replace_part(s,a+2,b-3,substring);
						break;
					}
				}
				a = s.find("{{",b);
			}

			// Remove HTML-Comments
			a = s.find("<!--");
			while (a != (int) string::npos)
			{
				b = s.find("-->",a);
				if (b == (int) string::npos)
					b = s.length() - 1;
				else
					b += 2;

				replace_part(s, a, b, "");
				a = s.find("<!--",a+1);
			}


			// Everything was separated out

			// Help 'make_tag_list' and mask literal '&' and '<'
			a = s.find("&");
			while (a != (int) string::npos)
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
			while (a != (int) string::npos)
			{
				for (b=0;b< (int) allowed_html.size();b++)
				{
					if ((s.substr(a+1,allowed_html[b].length()) == allowed_html[b])
										||(s.substr(a+1,allowed_html[b].length()+1) == "/"+allowed_html[b]))
						break;
				}
				if (b== (int) allowed_html.size())
				{
					s.replace(a,1,"&lt;");
				}
				a = s.find("<",a+1);
			}

			// Now we remove evil HTML
			vector<TXML> taglist;
			make_tag_list(s, taglist);
			sanitize_html(s, taglist);

			trimSpacesBeforeLinebreaks(s);

			// Now evaluate each line
			_globals->getPercentIndicator()->update(0);
			a = 0;
			b = s.find("\n",a);
			while (b != (int) string::npos)
			{
				_globals->getPercentIndicator()->update(b*100/s.length());
				substring = s.substr(a,b-a);
				parse_line(substring);
				replace_part(s,a,b,substring+"\n");
				a += substring.length()+1;
				b = s.find("\n",a);
			}
			_globals->getPercentIndicator()->update(100);
			substring = s.substr(a);
			parse_line(substring);
			replace_part(s,a,s.length()-1,substring);

			string end;

			// Cleanup lists
			end = fix_list(end);
			if (!end.empty())
				s += end;

			// Cleanup tables
			end.clear();
			while (tables.size())
			{
				end += tables[tables.size() - 1].close();
				tables.pop_back();
			}
			if (!end.empty())
				s += end;

			// recombine
			for (a=0;a< (int) source_contents.size();a++)
			{
				replace_all(s,dswiki_magic_phrase+FromUTF(magic_offset+nowiki_contents.size()+math_contents.size()+pre_contents.size()+a),
							"<source"+source_contents[a]+"</source>");
			}
			source_contents.clear();
			for (a=0;a< (int) pre_contents.size();a++)
			{
				replace_all(s,dswiki_magic_phrase+FromUTF(magic_offset+nowiki_contents.size()+math_contents.size()+a),"<pre>"+pre_contents[a]+"</pre>");
			}
			pre_contents.clear();
			for (a=0;a< (int) math_contents.size();a++)
			{
				replace_all(s,dswiki_magic_phrase+FromUTF(magic_offset+nowiki_contents.size()+a),"<math>"+math_contents[a]+"</math>");
			}
			math_contents.clear();
			for (a=0;a< (int) nowiki_contents.size();a++)
			{
				replace_all(s,dswiki_magic_phrase+FromUTF(magic_offset+a),nowiki_contents[a]);
			}
			nowiki_contents.clear();
			replace_all(s,dswiki_magic_nowiki_open,"&lt;nowiki&gt;"); // 'special' normal treatment for nowiki in math
			replace_all(s,dswiki_magic_nowiki_close,"&lt;/nowiki&gt;");
			break;
		}
		case MEDIUM_PARSE:
		{
			replace_all(s,"&","&amp;");
			replace_all(s,"<","&lt;");
			replace_all(s,">","&gt;");
			replace_all(s,"\"","&quot;");
			replace_all(s,"'","&apos;");

			a = 0;
			b = s.find("\n",a);
			while (b != (int) string::npos)
			{
				_globals->getPercentIndicator()->update(b*100/s.length());
				substring = s.substr(a,b-a);
				parse_line(substring);
				replace_part(s,a,b,substring+"\n");
				a += substring.length()+1;
				b = s.find("\n",a);
			}
			_globals->getPercentIndicator()->update(100);
			substring = s.substr(a);
			parse_line(substring);
			replace_part(s,a,s.length()-1,substring);

			string end;

			// Cleanup lists
			end = fix_list(end);
			if (!end.empty())
				s += end;

			// Cleanup tables
			end.clear();
			while (tables.size())
			{
				end += tables[tables.size() - 1].close();
				tables.pop_back();
			}
			if (!end.empty())
				s += end;

			break;
		}
		case TEXT_PARSE:
		{
			_globals->getStatusbar()->display("l="+val(s.length()));
			_globals->getPercentIndicator()->update(0);
			replace_all(s,"&","&amp;");
			_globals->getStatusbar()->display("l="+val(s.length()));
			_globals->getPercentIndicator()->update(17);
			replace_all(s,"<","&lt;");
			_globals->getStatusbar()->display("l="+val(s.length()));
			_globals->getPercentIndicator()->update(33);
			replace_all(s,">","&gt;");
			_globals->getStatusbar()->display("l="+val(s.length()));
			_globals->getPercentIndicator()->update(50);
			replace_all(s,"\"","&quot;");
			_globals->getStatusbar()->display("l="+val(s.length()));
			_globals->getPercentIndicator()->update(67);
			replace_all(s,"'","&apos;");
			_globals->getStatusbar()->display("l="+val(s.length()));
			_globals->getPercentIndicator()->update(83);
			replace_all(s,"\n\n","\n<p />\n");
			_globals->getStatusbar()->display("l="+val(s.length()));
			_globals->getPercentIndicator()->update(100);
			break;
		}
	}

	s.insert(0,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<text>");
	s.append("<p /></text>");
}


WIKI2XML::WIKI2XML()
{
	list.clear();

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
	allowed_html.push_back("big");
	allowed_html.push_back("strike");
	allowed_html.push_back("sub");
	allowed_html.push_back("sup");
	allowed_html.push_back("center");

	allowed_html.push_back("code");
	allowed_html.push_back("nowiki");
	allowed_html.push_back("pre");
	allowed_html.push_back("source");
	allowed_html.push_back("math");
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

	allowed_html.push_back("noinclude");
	allowed_html.push_back("includeonly");

	allowed_html.push_back("!--");
}


void WIKI2XML::replace_part(string & l, int from, int to, string with)
{
	l.replace(from,to-from+1,with);
}


void WIKI2XML::replace_part_sync(string & l, int from, int to, string with, vector < TXML > &list)
{
	int a, b;
	replace_part(l, from, to, with);
	for (a = 0; a < (int) list.size(); a++)
	{
		for (b = 0; b < (int) with.length(); b++)
			list[a].insert_at(from);
		for (b = from; b <= to; b++)
			list[a].remove_at(from);
	}
}


void WIKI2XML::make_tag_list(string & s, vector < TXML > &list)
{
	list.clear();
	int a, b;
	for (a = 0; a < (int) s.length(); a++)
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
	for (a = 0; a < (int) taglist.size(); a++)
	{
		string tag = taglist[a].name;

		for (b = 0; b < (int) allowed_html.size() && tag != allowed_html[b]; b++);

		if (b < (int) allowed_html.size()) // Allowed tag
		{
			if ((tag=="br") || (tag=="hr"))
			{
				if (!taglist[a].selfclosing)
				{
					replace_part_sync(s, taglist[a].to, taglist[a].to, " />", taglist);
					taglist[a].selfclosing = true;
				}
				if (taglist[a].closing)
				{
					replace_part_sync(s, taglist[a].from, taglist[a].from+1, "<", taglist);
					taglist[a].to--;
					taglist[a].closing = false;
				}
			}

			if (taglist[a].closing && taglist[a].selfclosing) // Correct closing and selfclosing tag
			{
				replace_part_sync(s, taglist[a].to-1, taglist[a].to, ">", taglist);
				taglist[a].to--;
				taglist[a].selfclosing = false;
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
// 		ret = xml_embed("", "table", l.substr(2, l.length() - 2), true);
		ret = xml_embed("", "table", "", true);
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
			for (a = 1; a < (int) l.length() && l[a] == '-'; a++);
// 			ret = tables[tables.size() - 1].new_row(l.substr(a, l.length() - a));
			ret = tables[tables.size() - 1].new_row("");
		}
		else
		{
			string type;
			if (left(l, 2) == "|+")
			{
				type = "caption";
				l.erase(0,2);
			}
			else if (l[0] == '!')
			{
				type = "header";
				l.erase(0,1);
			}
			else if (l[0] == '|')
			{
				type = "cell";
				l.erase(0,1);
			}

			vector < string > sublines;
			for (a = 0; a + 1 < (int) l.length(); a++)
			{
				if (l[a] == '|' && l[a + 1] == '|')
				{
					sublines.push_back(left(l, a));
					l.erase(0, a + 2);
					a = -1;
				}
			}
			if (!l.empty())
				sublines.push_back(l);

			for (a = 0; a < (int) sublines.size(); a++)
			{
				l.clear();
				l += sublines[a];
				parse_line_sub(l);
				string params;
				int b = find_next_unquoted('|', l);
				if (b != -1)
				{
					params = left(l, b);
					l.erase(0, b + 1);
				}
				if (type == "cell")
				{
// 					ret += tables[tables.size() - 1].new_cell(params);
					ret += tables[tables.size() - 1].new_cell("");
					ret += l;
				}
				else if (type == "header")
				{
// 					ret += tables[tables.size() - 1].new_headercell(params);
					ret += tables[tables.size() - 1].new_headercell("");
					ret += l;
				}
				else if (type == "caption")
				{
// 					ret += tables[tables.size() - 1].new_caption(l,params);
					ret += tables[tables.size() - 1].new_caption(l,"");
				}
			}
		}
	}
	else // no table
	{
		ret = l;
	}

	return ret;
}

void WIKI2XML::setGlobals(Globals* globals)
{
	_globals = globals;
}
