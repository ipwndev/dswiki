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

#include "chrlib.h"
#include "char_convert.h"
#include "main.h"

Dumps::Dumps()
{
	_setPossibleWikis();
}

vector<string> Dumps::getPossibleWikis()
{
	vector<string> possibleWikis;
	for (int i=0;i<_possibleWikis.size();i++)
	{
		possibleWikis.push_back(_possibleWikis[i].wikibase);
	}
	sort(possibleWikis.begin(),possibleWikis.end());
	return possibleWikis;
}

string Dumps::get_ifo(string basename)
{
	for (int i=0;i<_possibleWikis.size();i++)
	{
		if (_possibleWikis[i].wikibase == basename)
			return _possibleWikis[i].wikiifo;
	}
	return "";
}

string Dumps::get_idx(string basename)
{
	for (int i=0;i<_possibleWikis.size();i++)
	{
		if (_possibleWikis[i].wikibase == basename)
			return _possibleWikis[i].wikiidx;
	}
	return "";
}

string Dumps::get_ao1(string basename)
{
	for (int i=0;i<_possibleWikis.size();i++)
	{
		if (_possibleWikis[i].wikibase == basename)
			return _possibleWikis[i].wikiao1;
	}
	return "";
}

string Dumps::get_ao2(string basename)
{
	for (int i=0;i<_possibleWikis.size();i++)
	{
		if (_possibleWikis[i].wikibase == basename)
			return _possibleWikis[i].wikiao2;
	}
	return "";
}

vector<string> Dumps::get_dbs(string basename)
{
	for (int i=0;i<_possibleWikis.size();i++)
	{
		if (_possibleWikis[i].wikibase == basename)
			return _possibleWikis[i].wikidbs;
	}
	vector<string> dummy;
	return dummy;
}

void Dumps::_setPossibleWikis()
{
	_possibleWikis.clear();
// 	int line = 2;
	vector<string> filenames;
	struct stat st;
	char filename[256]; // to hold a full filename and string terminator

	DIR_ITER* dir = diropen("fat:/dswiki/");
	if (dir != NULL)
	{
// 		PA_OutputText(0,0,0,"Contents of \"/dswiki/\":");
// 		PA_OutputText(0,0,1,"-----------------------");
		while (dirnext(dir, filename, &st) == 0)
		{
			if (!(st.st_mode & S_IFDIR)) // regular file
			{
// 				PA_OutputText(0,0,line++,"%s",filename);
				string filenamestr(filename);
				filenames.push_back(filenamestr);
			}
		}
// 		for (int i=0;i<filenames.size();i++)
// 		{
// 			PA_OutputText(0,0,i,"%s",filenames[i].c_str());
// 		}
// 		PA_WaitFor(Pad.Newpress.Anykey);

		for (int i=0;i<filenames.size();i++)
		{
			int lastDot = filenames[i].rfind(".");
			if (lastDot != string::npos)
			{
				if (lowerPhrase(filenames[i].substr(lastDot+1))=="ifo")
				{
					WikiDump currentWiki;
					currentWiki.wikiifo = "";
					currentWiki.wikiidx = "";
					currentWiki.wikiao1 = "";
					currentWiki.wikiao2 = "";
					string wikiname = filenames[i].substr(0,lastDot);
					currentWiki.wikibase = wikiname;
					currentWiki.wikiifo = filenames[i];
// 					PA_OutputText(0,0,line++,"Found: %s",wikiname.c_str());
// 					PA_OutputText(1,0,8,"Found: %s                        ",wikiname.c_str());
// 					PA_OutputText(1,0,10,"%c1[ifo][idx][ao1][ao2][db?]");
// 					PA_Sleep(30);
// 					PA_OutputText(1,0,10,"%c2[ifo]");
// 					PA_Sleep(30);
					unsigned char foundidx = 0;
					unsigned char foundao1 = 0;
					unsigned char foundao2 = 0;
					unsigned char founddb  = 0;
					for (int j=0;j<filenames.size();j++)
					{
						int compareLastDot = filenames[j].rfind(".");
						string basename = filenames[j].substr(0,compareLastDot);
						if (basename == wikiname)
						{
							string lowerExt = lowerPhrase(filenames[j].substr(compareLastDot+1));
							if (lowerExt.length()==3)
							{
								if (lowerExt == "idx")
								{
									foundidx = 1;
									currentWiki.wikiidx = filenames[j];
// 									PA_OutputText(1,5,10,"%c2[idx]");
// 									PA_Sleep(30);
								}
								if (lowerExt == "ao1")
								{
									foundao1 = 1;
									currentWiki.wikiao1 = filenames[j];
// 									PA_OutputText(1,10,10,"%c2[ao1]");
// 									PA_Sleep(30);
								}
								if (lowerExt == "ao2")
								{
									foundao2 = 1;
									currentWiki.wikiao2 = filenames[j];
// 									PA_OutputText(1,15,10,"%c2[ao2]");
// 									PA_Sleep(30);
								}
								if ( (lowerExt.substr(0,2) == "db") && (lowerExt.substr(2).find_first_of("abcdefghijklmnopqrstuvwxyz")!=string::npos) )
								{
									founddb = 1;
									currentWiki.wikidbs.push_back(filenames[j]);
// 									PA_OutputText(1,20,10,"%c2[%s]",lowerExt.c_str());
// 									PA_Sleep(30);
								}
							}
						}
					}
// 					sort(currentWiki.wikidbs.begin(),currentWiki.wikidbs.end());
					if (foundidx && foundao1 && foundao2 && founddb)
					{
						_possibleWikis.push_back(currentWiki);
// 						PA_OutputText(1,0,12,"Adding %s, total: %d",wikiname.c_str(),_possibleWikis.size());
// 						PA_Sleep(30);
					}
				}
			}
		}
	}
	else
	{
		PA_OutputText(1,0,5,"%c1Error: dir");
	}
}
