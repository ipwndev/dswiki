#ifndef _GLOBAL_FUNCTIONS_H_
#define _GLOBAL_FUNCTIONS_H_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h>

using namespace std;

#define SINGLE_QUOTE 39
#define DOUBLE_QUOTE '"'

typedef string::value_type chart;	// Char type

string upper(string s);
bool is_text_char(chart ch);
string val(int a);
string unquote(chart quote, string & s);
bool submatch(string & main, string & sub, int from);

void explode(string pattern, string & s, vector < string > &parts);
void implode(string pattern, vector < string > &parts, string & s);

string right(string & s, int num);
string left(string & s, int num);
string before_first(chart c, string s);
string before_last(chart c, string s);
string after_first(chart c, string s);
string after_last(chart c, string s);

void trim(string & s);
void trimLeft(string & s);
void trimRight(string & s);
void trimDoubleSpaces(string & s);
void trimSpacesBeforeLinebreaks(string & s);

int find_next_unquoted(chart c, string & s, int start = 0);
string xml_embed(string inside, string tag, string param = "", bool openonly = false);

void replaceLinebreaks(string & s);

#endif
