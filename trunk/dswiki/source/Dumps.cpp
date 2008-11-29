//
// C++ Implementation: Dumps
//
// Description:
//
//
// Author: Oliver Gronau <ogronau@web.de>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "Dumps.h"

#include <sys/dir.h>
#include <algorithm>
#include <vector>
#include <string>

#include "char_convert.h"
#include "main.h"

Dumps::Dumps()
{
	_gatherPossibleWikis();
}

vector<string> Dumps::getPossibleWikis()
{
	vector<string> possibleWikis;
	for (int i=0;i<_possibleWikis.size();i++)
	{
		if (_possibleWikis[i].basename.substr(0,8)!="_dswiki_")
		{
			possibleWikis.push_back(_possibleWikis[i].basename);
		}
	}
	sort(possibleWikis.begin(),possibleWikis.end());
	return possibleWikis;
}

string Dumps::get_ifo(string basename)
{
	for (int i=0;i<_possibleWikis.size();i++)
	{
		if (_possibleWikis[i].basename == basename)
			return _possibleWikis[i].ifofile;
	}
	return "";
}

string Dumps::get_idx(string basename)
{
	for (int i=0;i<_possibleWikis.size();i++)
	{
		if (_possibleWikis[i].basename == basename)
			return _possibleWikis[i].idxfile;
	}
	return "";
}

string Dumps::get_ao1(string basename)
{
	for (int i=0;i<_possibleWikis.size();i++)
	{
		if (_possibleWikis[i].basename == basename)
			return _possibleWikis[i].ao1file;
	}
	return "";
}

string Dumps::get_ao2(string basename)
{
	for (int i=0;i<_possibleWikis.size();i++)
	{
		if (_possibleWikis[i].basename == basename)
			return _possibleWikis[i].ao2file;
	}
	return "";
}

vector<string> Dumps::get_dbs(string basename)
{
	for (int i=0;i<_possibleWikis.size();i++)
	{
		if (_possibleWikis[i].basename == basename)
			return _possibleWikis[i].dbsfiles;
	}
	vector<string> dummy;
	return dummy;
}

void Dumps::_gatherPossibleWikis()
{
	_possibleWikis.clear();

	vector<string> filenames;
	struct stat st;
	char filename[256]; // to hold a full filename and string terminator

	string pathname = "fat:/dswiki/";

	DIR_ITER* dir = diropen(pathname.c_str());
	if (dir != NULL)
	{
		while (dirnext(dir, filename, &st) == 0)
		{
			if (!(st.st_mode & S_IFDIR)) // regular file
			{
				string filenamestr(filename);
				if (filenamestr.substr(0,8)!="_dswiki_")
					filenames.push_back(filenamestr);
			}
		}

		for (int i=0;i<filenames.size();i++)
		{
			int lastDot = filenames[i].rfind(".");
			if (lastDot != string::npos)
			{
				if (lowerPhrase(filenames[i].substr(lastDot+1))=="ifo")
				{
					WikiDump currentWiki;
					currentWiki.basename = filenames[i].substr(0,lastDot);
					currentWiki.ifofile  = pathname + filenames[i];
					for (int j = 0; j < filenames.size(); j++)
					{
						int lastDot2 = filenames[j].rfind(".");
						string basename2 = filenames[j].substr(0,lastDot2);
						if (basename2 == currentWiki.basename)
						{
							string lowerExt2 = lowerPhrase(filenames[j].substr(lastDot2+1));
							if (lowerExt2.length()==3)
							{
								if (lowerExt2 == "idx")
									currentWiki.idxfile = pathname + filenames[j];
								if (lowerExt2 == "ao1")
									currentWiki.ao1file = pathname + filenames[j];
								if (lowerExt2 == "ao2")
									currentWiki.ao2file = pathname + filenames[j];
								if ( (lowerExt2.substr(0,2) == "db") && (lowerExt2.substr(2).find_first_of("abcdefghijklmnopqrstuvwxyz")!=string::npos) )
									currentWiki.dbsfiles.push_back(pathname + filenames[j]);
							}
						}
					}
					if ((!currentWiki.idxfile.empty()) && (!currentWiki.ao1file.empty()) && (!currentWiki.ao2file.empty()) && (!currentWiki.dbsfiles.empty()))
					{
						_possibleWikis.push_back(currentWiki);
					}
				}
			}
		}
		dirclose(dir);
	}

	pathname = "efs:/dswiki/";

	dir = diropen(pathname.c_str());
	if (dir != NULL)
	{
		while (dirnext(dir, filename, &st) == 0)
		{
			if (!(st.st_mode & S_IFDIR)) // regular file
			{
				string filenamestr(filename);
				if (filenamestr.substr(filenamestr.length()-3)=="ifo")
				{
					WikiDump currentWiki;
					currentWiki.basename = "_dswiki_" + filenamestr.substr(0,filenamestr.length()-4);
					currentWiki.ifofile  =  pathname  + filenamestr.substr(0,filenamestr.length()-4) + ".ifo";
					currentWiki.idxfile  =  pathname  + filenamestr.substr(0,filenamestr.length()-4) + ".idx";
					currentWiki.ao1file  =  pathname  + filenamestr.substr(0,filenamestr.length()-4) + ".ao1";
					currentWiki.ao2file  =  pathname  + filenamestr.substr(0,filenamestr.length()-4) + ".ao2";
					currentWiki.dbsfiles.push_back(pathname + filenamestr.substr(0,filenamestr.length()-4) + ".dba");
					_possibleWikis.push_back(currentWiki);
				}
			}
		}
		dirclose(dir);
	}
}
