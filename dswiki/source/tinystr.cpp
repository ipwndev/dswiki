#include "tinystr.h"

// Error value for find primitive
const size_t TiXmlString::npos = static_cast< size_t >(-1);


void TiXmlString::reserve (size_t cap)
{
}


TiXmlString& TiXmlString::assign(const char* str, size_t len)
{
	return *this;
}


TiXmlString& TiXmlString::append(const char* str, size_t len)
{
	if (_parts.empty())
	{
		Part temp;
		temp.start = (char*) str;
		temp.len=len;
		_parts.push_back(temp);
	}
	else
	{
		_parts[_parts.size()-1].len += len;
	}
	return *this;
}


TiXmlString operator + (const TiXmlString & a, const TiXmlString & b) { }

TiXmlString operator + (const TiXmlString & a, const char* b) { }

TiXmlString operator + (const char* a, const TiXmlString & b) { }
