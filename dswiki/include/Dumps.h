//
// C++ Interface: Dumps
//
// Description:
//
//
// Author: Oliver Gronau <ogronau@web.de>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef _DUMPS_H
#define _DUMPS_H

#include <PA9.h>
#include <fat.h>
#include <string>
#include <vector>

using namespace std;

typedef struct
{
	string basename;
	string idxfile;
	string ao1file;
	string ao2file;
	string ifofile;
	vector<string> dbsfiles;
} WikiDump;

class Dumps
{
	public:
		Dumps();
		vector<string> getPossibleWikis();
		string get_ifo(string basename);
		string get_idx(string basename);
		string get_ao1(string basename);
		string get_ao2(string basename);
		vector<string> get_dbs(string basename);
	private:
		void _gatherPossibleWikis();
		vector<WikiDump> _possibleWikis;
};

#endif
