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

vector<string> Dumps::getPossibleWikis(bool internal)
{
	vector<WikiDump> * list;
	if (internal)
		list = &_internalWikis;
	else
		list = &_externalWikis;

	vector<string> possibleWikis;
	for (int i=0;i< (int) (*list).size();i++)
	{
		possibleWikis.push_back((*list)[i].basename);
	}
	sort(possibleWikis.begin(),possibleWikis.end());
	return possibleWikis;
}

string Dumps::get_ifo(string basename, bool internal)
{
	vector<WikiDump> * list;
	if (internal)
		list = &_internalWikis;
	else
		list = &_externalWikis;

	for (int i=0;i< (int) list->size();i++)
	{
		if ((*list)[i].basename == basename)
			return (*list)[i].ifofile;
	}
	return "";
}

string Dumps::get_idx(string basename, bool internal)
{
	vector<WikiDump> * list;
	if (internal)
		list = &_internalWikis;
	else
		list = &_externalWikis;

	for (int i=0;i< (int) list->size();i++)
	{
		if ((*list)[i].basename == basename)
			return (*list)[i].idxfile;
	}
	return "";
}

string Dumps::get_ao1(string basename, bool internal)
{
	vector<WikiDump> * list;
	if (internal)
		list = &_internalWikis;
	else
		list = &_externalWikis;

	for (int i=0;i< (int) list->size();i++)
	{
		if ((*list)[i].basename == basename)
			return (*list)[i].ao1file;
	}
	return "";
}

string Dumps::get_ao2(string basename, bool internal)
{
	vector<WikiDump> * list;
	if (internal)
		list = &_internalWikis;
	else
		list = &_externalWikis;

	for (int i=0;i< (int) list->size();i++)
	{
		if ((*list)[i].basename == basename)
			return (*list)[i].ao2file;
	}
	return "";
}

vector<string> Dumps::get_dbs(string basename, bool internal)
{
	vector<WikiDump> * list;
	if (internal)
		list = &_internalWikis;
	else
		list = &_externalWikis;

	for (int i=0;i< (int) list->size();i++)
	{
		if ((*list)[i].basename == basename)
			return (*list)[i].dbsfiles;
	}
	vector<string> dummy;
	return dummy;
}

void Dumps::_gatherPossibleWikis()
{
	vector<string> filenames;
	struct stat st;
	char filename[256]; // to hold a full filename and string terminator

	// external case, search for all neccessary files
	string pathname = "fat:/dswiki/";

	DIR_ITER* dir = diropen(pathname.c_str());
	if (dir != NULL)
	{
		while (dirnext(dir, filename, &st) == 0)
		{
			if (!(st.st_mode & S_IFDIR)) // regular file
			{
				string filenamestr(filename);
				filenames.push_back(filenamestr);
				if (debug)
				{
					PA_ClearTextBg(1);
					PA_OutputText(1,0,0,"file found in %c3fat:/dswiki/%c0:\n%c3%s",filenamestr.c_str());
					PA_WaitFor(Pad.Newpress.Anykey);
				}
			}
		}

		for (int i=0;i< (int) filenames.size();i++)
		{
			int lastDot = filenames[i].rfind(".");
			if (lastDot != (int) string::npos)
			{
				if (lowerPhrase(filenames[i].substr(lastDot+1))=="ifo")
				{
					WikiDump currentWiki;
					currentWiki.basename = filenames[i].substr(0,lastDot);
					currentWiki.ifofile  = pathname + filenames[i];

					if (debug)
					{
						PA_ClearTextBg(1);
						PA_OutputText(1,0,0,"ifo-file found on disk:\n%c3%s",filenames[i].c_str());
						PA_OutputText(1,0,3,"we try to find all files now:");
						PA_OutputText(1,0,4,"*.idx:");
						PA_OutputText(1,0,5,"*.ao1:");
						PA_OutputText(1,0,6,"*.ao2:");
						PA_OutputText(1,0,7,"*.db?:");
						PA_WaitFor(Pad.Newpress.Anykey);
					}

					for (int j = 0; j < (int) filenames.size(); j++)
					{
						int lastDot2 = filenames[j].rfind(".");
						string basename2 = filenames[j].substr(0,lastDot2);
						if (basename2 == currentWiki.basename)
						{
							string lowerExt2 = lowerPhrase(filenames[j].substr(lastDot2+1));
							if (lowerExt2.length()==3)
							{
								if (lowerExt2 == "idx")
								{
									currentWiki.idxfile = pathname + filenames[j];
									if (debug)
									{
										PA_OutputText(1,8,4,"%c2found");
										PA_WaitFor(Pad.Newpress.Anykey);
									}
								}
								else if (lowerExt2 == "ao1")
								{
									currentWiki.ao1file = pathname + filenames[j];
									if (debug)
									{
										PA_OutputText(1,8,5,"%c2found");
										PA_WaitFor(Pad.Newpress.Anykey);
									}
								}
								else if (lowerExt2 == "ao2")
								{
									currentWiki.ao2file = pathname + filenames[j];
									if (debug)
									{
										PA_OutputText(1,8,6,"%c2found");
										PA_WaitFor(Pad.Newpress.Anykey);
									}
								}
								else if ( (lowerExt2.substr(0,2) == "db") && (lowerExt2.substr(2).find_first_of("abcdefghijklmnopqrstuvwxyz")!=string::npos) )
								{
									if (debug)
									{
										PA_OutputText(1,8,7,"%s %c2found",lowerExt2.c_str());
										PA_WaitFor(Pad.Newpress.Anykey);
									}
									currentWiki.dbsfiles.push_back(pathname + filenames[j]);
									sort(currentWiki.dbsfiles.begin(),currentWiki.dbsfiles.end());
								}
							}
						}
					}
					if ((!currentWiki.idxfile.empty()) && (!currentWiki.ao1file.empty()) && (!currentWiki.ao2file.empty()) && (!currentWiki.dbsfiles.empty()))
					{
						if (debug)
						{
							PA_OutputText(1,0,9,"Found everything, so we add\n%c3%s",(pathname+currentWiki.basename).c_str());
							PA_OutputText(1,0,12,"%s",currentWiki.basename.c_str());
							PA_OutputText(1,0,13,"%s",currentWiki.ifofile.c_str());
							PA_OutputText(1,0,14,"%s",currentWiki.idxfile.c_str());
							PA_OutputText(1,0,15,"%s",currentWiki.ao1file.c_str());
							PA_OutputText(1,0,16,"%s",currentWiki.ao2file.c_str());
							for (int a=0; a < (int) currentWiki.dbsfiles.size(); a++)
								PA_OutputText(1,0,17+a,"%s",currentWiki.dbsfiles[a].c_str());
							PA_WaitFor(Pad.Newpress.Anykey);
						}
						_externalWikis.push_back(currentWiki);
					}
				}
			}
		}
		dirclose(dir);
	}

	// the internal case is much easier, we simply trust that all files are really there
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
					currentWiki.basename =              filenamestr.substr(0,filenamestr.length()-4);
					currentWiki.ifofile  =  pathname  + filenamestr.substr(0,filenamestr.length()-4) + ".ifo";
					currentWiki.idxfile  =  pathname  + filenamestr.substr(0,filenamestr.length()-4) + ".idx";
					currentWiki.ao1file  =  pathname  + filenamestr.substr(0,filenamestr.length()-4) + ".ao1";
					currentWiki.ao2file  =  pathname  + filenamestr.substr(0,filenamestr.length()-4) + ".ao2";
					currentWiki.dbsfiles.push_back(pathname + filenamestr.substr(0,filenamestr.length()-4) + ".dba");
					_internalWikis.push_back(currentWiki);

					if (debug)
					{
						PA_ClearTextBg(1);
						PA_OutputText(1,0,0,"dump found in %c3efs:/dswiki/%c0:\n%c3%s",currentWiki.basename.c_str());
						PA_WaitFor(Pad.Newpress.Anykey);
					}
				}
			}
		}
		dirclose(dir);
	}

	if (debug)
		PA_ClearTextBg(1);
}
