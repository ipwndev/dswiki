//
// C++ Implementation: FATBrowser
//
// Description:
//
//
// Author: Oliver Gronau <ogronau@web.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "FATBrowser.h"

FATBrowser::FATBrowser()
{
	currentDir = "fat:/";
}

FATBrowser::~ FATBrowser()
{
}

std::string FATBrowser::selectFile()
{
	vector<string> dirnames;
	vector<string> filenames;

	struct stat st;
	char filename[256]; // to hold a full filename and string terminator

	DIR_ITER* dir = diropen(currentDir.c_str());
	if (dir != NULL)
	{
		while (dirnext(dir, filename, &st) == 0)
		{
			if (!(st.st_mode & S_IFDIR)) // regular file
			{
				string filenamestr(filename);
				filenames.push_back(filenamestr);
			}
			else
			{
				string filenamestr(filename);
				if (filenamestr != ".")
				{
					dirnames.push_back("[" + filenamestr + "]");
				}
			}
		}
		dirclose(dir);
		sort(dirnames.begin(),dirnames.end());
		sort(filenames.begin(),filenames.end());
		for (int a = 0; a < (int) filenames.size(); a++ )
		{
			dirnames.push_back(filenames[a]);
		}
		TextBox tb(dirnames);
		tb.setGlobals(_globals);
		tb.setTitle(currentDir);
		tb.maximize();
		tb.run();
	}
}

