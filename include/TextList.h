#ifndef _TEXTLIST_H
#define _TEXTLIST_H
//
// C++ Interface: TextList
//
// Description: 
//
//
// Author: Oliver Gronau <ogronau@web.de>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <PA9.h>
#include <string>
#include <vector>

#include "main.h"
#include "struct.h"
#include "api.h"
#include "chrlib.h"

class TextList
{
	public:
		static s32 show(string headline, vector<string> lines, s32 highlightedLine, s32 firstLine, u8 wrapAround);
	private:
};

#endif
