#include "tinystr.h"

TiXmlString& TiXmlString::assign(const char* str, size_t len)
{
	if (len)
	{
		start = (char*) str;
		int_size = len;
	}
	else
	{
		start = NULL;
		int_size = 0;
	}
	return *this;
}


TiXmlString& TiXmlString::append(const char* str, size_t len)
{
	if (int_size)
	{
		int_size += len;
		return *this;
	}
	else
	{
		return assign(str,len);
	}
}
