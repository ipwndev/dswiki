#ifndef TIXML_STRING_INCLUDED
#define TIXML_STRING_INCLUDED


#include <assert.h>
#include <string.h>
#include <string>
#include <vector>
#include <PA9.h>


/*
   TiXmlString is an emulation of a subset of the std::string template.
   Its purpose is to allow compiling TinyXML on compilers with no or poor STL support.
   Only the member functions relevant to the TinyXML project have been implemented.
   The buffer allocation is made by a simplistic power of 2 like mechanism : if we increase
   a string and there's no more room, we allocate a buffer twice as big as we need.
*/

class TiXmlString
{
  public :
	// Error value for find primitive
	static const size_t npos; // = -1;

	// TiXmlString empty constructor
	TiXmlString () { _parts.clear(); }

	// TiXmlString copy constructor
	TiXmlString ( const TiXmlString & copy) { } // TODO

	// TiXmlString constructor, based on a string
	TiXmlString ( const char * copy)
	{
		_parts.clear();
		Part temp;
		temp.start = (char*) copy;
		temp.len=strlen(copy);
		_parts.push_back(temp);
	}

	// TiXmlString constructor, based on a string
	TiXmlString ( const char * str, size_t len)
	{
		_parts.clear();
		Part temp;
		temp.start = (char*) str;
		temp.len=len;
		_parts.push_back(temp);
	}

	// TiXmlString destructor
	~TiXmlString ()
	{
		_parts.clear();
	}

	// = operator
	TiXmlString& operator = (const char * copy)
	{
		TiXmlString* tiStr = new TiXmlString(copy);
		return *tiStr;
	}

	// = operator
	TiXmlString& operator = (const TiXmlString & copy)
	{
		TiXmlString* tiStr = new TiXmlString(copy);
		return *tiStr;
	}

	// += operator. Maps to append
	TiXmlString& operator += (const char * suffix) { return append(suffix, static_cast<size_t>( strlen(suffix) )); }

	// += operator. Maps to append
	TiXmlString& operator += (char single) { return append(&single, 1); }

	// += operator. Maps to append
	TiXmlString& operator += (const TiXmlString & suffix) { return append(suffix.data(), suffix.length()); }

	// Convert a TiXmlString into a null-terminated char *
	const char * c_str () const { }

	// Convert a TiXmlString into a char * (need not be null terminated).
	const char * data () const { }

	// Return the length of a TiXmlString
	size_t length () const { }

	// Alias for length()
	size_t size () const { }

	// Checks if a TiXmlString is empty
	bool empty () const { }

	// Return capacity of string
	size_t capacity () const { }

	// single char extraction
	const char& at (size_t index) const { }

	// [] operator
	char& operator [] (size_t index) const { }

	// find a char in a string. Return TiXmlString::npos if not found
	size_t find (char lookup) const { }

	// find a char in a string from an offset. Return TiXmlString::npos if not found
	size_t find (char tofind, size_t offset) const { }

	void clear ()
	{
		_parts.clear();
	}

	/*	Function to reserve a big amount of data when we know we'll need it. Be aware that this
		function DOES NOT clear the content of the TiXmlString if any exists.
	*/
	void reserve (size_t cap);

	TiXmlString& assign (const char* str, size_t len);

	TiXmlString& append (const char* str, size_t len);

	void swap (TiXmlString& other) { }

	std::string ValueStr()
	{
		std::string str;
		for (int a=0;a<_parts.size();a++)
			str.append(_parts[a].start,_parts[a].len);
		return str;
	}

  private:

	struct Part
	{
		char* start;
		size_t len;
	};

	std::vector < Part > _parts;

};


inline bool operator == (const TiXmlString & a, const TiXmlString & b) { }
inline bool operator <  (const TiXmlString & a, const TiXmlString & b) { }

inline bool operator != (const TiXmlString & a, const TiXmlString & b) { }
inline bool operator >  (const TiXmlString & a, const TiXmlString & b) { }
inline bool operator <= (const TiXmlString & a, const TiXmlString & b) { }
inline bool operator >= (const TiXmlString & a, const TiXmlString & b) { }

inline bool operator == (const TiXmlString & a, const char* b) { }
inline bool operator == (const char* a, const TiXmlString & b) { }
inline bool operator != (const TiXmlString & a, const char* b) { }
inline bool operator != (const char* a, const TiXmlString & b) { }

TiXmlString operator +  (const TiXmlString & a, const TiXmlString & b);
TiXmlString operator +  (const TiXmlString & a, const char* b);
TiXmlString operator +  (const char* a, const TiXmlString & b);

#endif	// TIXML_STRING_INCLUDED
