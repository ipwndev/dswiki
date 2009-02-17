#ifndef TIXML_STRING_INCLUDED
#define TIXML_STRING_INCLUDED

#include <assert.h>
#include <string.h>
#include <string>

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
	// TiXmlString empty constructor
	TiXmlString ()
	{
	}

	// TiXmlString copy constructor
	TiXmlString ( const TiXmlString & copy)
	{
		start = copy.start;
		int_size = copy.int_size;
	}

	// TiXmlString constructor, based on a C-string
	TiXmlString ( const char * copy)
	{
		if (strlen(copy))
		{
			start = (char*) copy;
			int_size = strlen(copy);
		}
	}

	// TiXmlString constructor, based on a C-string
	TiXmlString ( const char * str, size_t len)
	{
		if (len)
		{
			start = (char*) str;
			int_size = len;
		}
	}

	// TiXmlString destructor
	~TiXmlString ()
	{
	}

	// = operator
	TiXmlString& operator = (const char * copy)
	{
		return assign( copy, (size_t)strlen(copy));
	}

	// += operator. Maps to append
	TiXmlString& operator += (char single)
	{
		return append(&single, 1);
	}

	// Convert a TiXmlString into a std::string
	std::string ValueStr() { std::string tmp; for (int a=0;a<(int)int_size;a++) tmp += start[a]; return tmp; }

	// Convert a TiXmlString into a null-terminated char *
	const char * c_str () const { return start; }

	// Convert a TiXmlString into a char * (need not be null terminated).
	const char * data () const { return start; }

	// Return the length of a TiXmlString
	size_t length () const { return int_size; }

	// Alias for length()
	size_t size () const { return int_size; }

	// Checks if a TiXmlString is empty
	bool empty () const { return int_size == 0; }

	// single char extraction
	const char& at (size_t index) const
	{
		assert( index < length() );
		return start[ index ];
	}

	// [] operator
	char& operator [] (size_t index) const
	{
		assert( index < length() );
		return start[ index ];
	}

	TiXmlString& assign (const char* str, size_t len);

	TiXmlString& append (const char* str, size_t len);

  private:

	size_t int_size;
	char* start;

} ;

#endif	// TIXML_STRING_INCLUDED
