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

FATBrowser::FATBrowser(bool internal)
{
	if (internal)
		currentDir = "efs:/";
	else
		currentDir = "fat:/";
}

FATBrowser::~ FATBrowser()
{
}

std::string FATBrowser::selectFile()
{
	std::string ret = "";

	chdir(currentDir.c_str());

	while(1)
	{
		vector<string> dirnames;
		vector<string> filenames;
		int dirs = 0;
		int files = 0;

		struct stat st;
		char filename[1024]; // to hold a full filename and string terminator

		DIR_ITER* dir = diropen(currentDir.c_str());
		if (dir != NULL)
		{
			while (dirnext(dir, filename, &st) == 0)
			{
				if (!(st.st_mode & S_IFDIR)) // regular file
				{
					string filenamestr(filename);
					filenames.push_back(filenamestr);
					files++;
				}
				else
				{
					string filenamestr(filename);
					if (filenamestr != ".")
					{
						dirnames.push_back("[" + filenamestr + "]");
						dirs++;
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
			tb.allowCancel(true);
			tb.allowSingleElement();
			tb.maximize();

			int choice = tb.run();

			if (choice == -1)
			{
				return "";
			}
			else if (choice < dirs)
			{
				// dir chosen
				string chosenDir = dirnames[choice];
				chosenDir = chosenDir.substr(1,chosenDir.length()-2);
				chdir(chosenDir.c_str());
				char pathname[1024];
				getcwd(pathname,1024);
				currentDir = pathname;
			}
			else
			{
				// file chosen
				string chosenFile = filenames[choice-dirs];
				ret = currentDir + chosenFile;
				return ret;
			}
		}
		else
		{
			// Should not happen
			return "";
		}
	}
	return "";
}

