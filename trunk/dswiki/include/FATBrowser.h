//
// C++ Interface: FATBrowser
//
// Description:
//
//
// Author: Oliver Gronau <ogronau@web.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef _FATBROWSER_H
#define _FATBROWSER_H

#include <PA9.h>
#include <fat.h>
#include "efs_lib.h"
#include <unistd.h>
#include <sys/dir.h>
#include <string>
#include <vector>
#include <algorithm>
#include "main.h"
#include "TextBox.h"
#include "Globals.h"

class FATBrowser
{
	public:
		FATBrowser(bool internal = false);
		~FATBrowser();
		std::string selectFile();
		void	setGlobals(Globals* globals) { _globals = globals; };
	private:
		string currentDir;
		Globals* _globals;
};

#endif
