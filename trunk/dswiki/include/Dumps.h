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
	string basename;			// short name of the wiki,     e.g. enwiki-20081024
	string idxfile;				// long filename of the index, e.g. fat:/dswiki/enwiki-20081024.idx
	string ao1file;				// ...
	string ao2file;				// ...
	string ifofile;				// ...
	vector<string> dbsfiles;	// vector of all long filenames of the data files
} WikiDump;

class Dumps
{
	public:
		Dumps();
		vector<string> getPossibleWikis(bool internal = false);
		string get_ifo(string basename, bool internal = false);
		string get_idx(string basename, bool internal = false);
		string get_ao1(string basename, bool internal = false);
		string get_ao2(string basename, bool internal = false);
		vector<string> get_dbs(string basename, bool internal = false);
	private:
		void _gatherPossibleWikis();
		vector<WikiDump> _externalWikis;
		vector<WikiDump> _internalWikis;
};

#endif
