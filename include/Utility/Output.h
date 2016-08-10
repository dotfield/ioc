/*
 * Output.h
 *
 *  Created on: 27 Nov 2013
 *      Author: neil
 */

#pragma once

#ifndef UTILITY_OUTPUT_H_
#define UTILITY_OUTPUT_H_

#include "api.h"
#include <ostream>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

// should be somewhere central? We use this alias in IOC but we don't want your
// interfaces to have to include IOC, but some more centralised config to specify
// what shared pointer library you are using (which must be either boost or std)

#include <memory>
#include <mutex>

namespace spns = std;

namespace Utility {

template< typename CHAR_TYPE >
class UTILITY_API BasicOutput
{
public:
	// Note: preference is actually to create a regular DLL for this
	// to ensure ODR is properly enforced.
	virtual ~BasicOutput();
	virtual std::basic_ostream<CHAR_TYPE> & os() = 0; // get an ostream object to output
	virtual void flush() = 0; // specific to each implementation
};

template class BasicOutput< char >;
template class BasicOutput< wchar_t >;

typedef BasicOutput< char > Output;
typedef BasicOutput< wchar_t > WOutput;

typedef spns::shared_ptr< Output > OutputPtr;
typedef spns::shared_ptr< WOutput > WOutputPtr;
// this might not work with endl.
template< typename CHAR_TYPE, typename T >
BasicOutput<CHAR_TYPE>& operator<<( BasicOutput<CHAR_TYPE>& bo, T val )
{
	bo.os() << val;
	return bo;
}

inline Output& operator<<( Output& bo, std::wstring const& val )
{
	bo.os() << std::string(val.begin(), val.end());
	return bo;
}

inline WOutput& operator<<( WOutput& bo, std::string const& val )
{
	bo.os() << std::wstring(val.begin(), val.end());
	return bo;
}

// Create outputs for file, console-out and console-error, and a function-based
// As with all the other implementations, consider moving or exposing only some kind of factory
// (As well as IOC builders, we can use alternative factories)

template< typename E >
class UTILITY_API BasicFileOutput : public BasicOutput<E>
{
public:
	BasicFileOutput( std::string fileName, bool toAppend )
		: m_fileName( fileName )
	{
		// throw if it fails to open
		m_fstream.exceptions( std::ios_base::badbit );

		m_fstream.open( fileName.c_str(), std::ios::out | (toAppend ? std::ios::app : std::ios::trunc) );

		// turn off throwing if subsequent write operations fail
		m_fstream.exceptions( std::ios_base::goodbit );
	}

	std::basic_ostream<E> & os()
	{
		return m_fstream;
	}

	// for this method just flush the file-write
	void flush()
	{
		m_fstream.flush();
	}

private:
	std::string m_fileName;
	std::basic_ofstream<E> m_fstream;
};

typedef BasicFileOutput<char> FileOutput;
typedef BasicFileOutput<wchar_t> WFileOutput;

template class BasicFileOutput<char>;
template class BasicFileOutput<wchar_t>;

// they are small but the duplication is horrible. However there is no basic_cout<E>...

class UTILITY_API ConsoleOutput : public Output
{
public:
	std::ostream & os() // can't be const because it overrides a virtual that isn't const in general
	{
		return std::cout;
	}

	void flush()
	{
		std::cout.flush();
	}
};

class UTILITY_API ConsoleError : public Output
{
public:
	std::ostream & os() // can't be const because it overrides a virtual that isn't const in general
	{
		return std::cerr;
	}

	void flush()
	{
		std::cerr.flush();
	}
};

class UTILITY_API WConsoleOutput : public WOutput
{
public:
	std::wostream & os() // can't be const because it overrides a virtual that isn't const in general
	{
		return std::wcout;
	}

	void flush()
	{
		std::wcout.flush();
	}
};

class UTILITY_API WConsoleError : public WOutput
{
public:
	std::wostream & os() // can't be const because it overrides a virtual that isn't const in general
	{
		return std::wcerr;
	}

	void flush()
	{
		std::wcerr.flush();
	}
};

// this one remains abstract as flush() is not implemented
template< typename E >
class UTILITY_API BasicStringOutput : public BasicOutput<E>
{
protected:
	typedef std::basic_string<E> string_type;

public:

	std::basic_ostream<E>& os()
	{
		return m_oss;
	}

	// for this implementation, flush() writes the data to the string (appending it)
	// and clears itself

protected:
	std::basic_ostringstream<E> m_oss;
};

// this uses a shared_ptr to a std::string. On flush() it writes to that string and clears the stringstream
template< typename E >
class UTILITY_API BasicSharedStringOutput : public BasicStringOutput<E>
{
	typedef std::basic_string<E> string_type;
public:

	explicit BasicSharedStringOutput( spns::shared_ptr< string_type > sharedString ) :
		m_sharedString( sharedString )
	{
	}

	std::basic_ostream<E>& os()
	{
		return m_oss;
	}

	// for this implementation, flush() writes the data to the string (appending it)
	// and clears itself

	void flush()
	{
		m_sharedString->append( m_oss.str() );
		m_oss.str( string_type() );
	}

protected:
	std::basic_ostringstream<E> m_oss;
	spns::shared_ptr< string_type > m_sharedString;
};

typedef BasicSharedStringOutput< char > SharedStringOutput;
typedef BasicSharedStringOutput< wchar_t > SharedWStringOutput;

template class BasicSharedStringOutput< char >;
template class BasicSharedStringOutput< wchar_t >;

// This one is implemented by invoking a user-supplied callback
// This one will not work with IOC as IOC has no support for std::function.
// For IOC you should write your own implementation with flush() being the callback,
// and your class taking any necessary parameters that are built through IOC

template< typename E >
class UTILITY_API BasicCallbackOutput : public BasicStringOutput<E>
{
	typedef std::basic_string<E> string_type;
	typedef std::function< void( string_type const& ) > callback_type;

public:
	explicit BasicCallbackOutput( callback_type callback )
		: m_callback( callback )
	{
	}

	std::basic_ostream<E>& os()
	{
		return m_oss;
	}

	void flush()
	{
		m_callback( m_oss.str() );
		m_oss.str( string_type() );
	}

private:
	std::basic_ostringstream<E> m_oss;
	callback_type	m_callback;
};

template< typename E >
class UTILITY_API BasicMTOutput // not derived from BasicOutput, it has one
{
	std::mutex m_mutex;

	spns::shared_ptr< BasicOutput<E> > m_output;

public:
	typedef std::unique_lock< std::mutex > unique_lock;

	explicit BasicMTOutput( spns::shared_ptr< BasicOutput<E> > output ) :
		m_output( output )
	{
	}
	// user should acquire and store the result in a scoped variable
	unique_lock acquire()
	{
		return unique_lock( m_mutex ); // this is "moved" to the owner
	}

	std::basic_ostream<E>& os()
	{
		return m_output->os();
	}

	void flush()
	{
		m_output->flush();
	}
};

typedef BasicMTOutput<char> MTOutput;
typedef BasicMTOutput<wchar_t> MTWOutput;

typedef spns::shared_ptr< MTOutput > MTOutputPtr;
typedef spns::shared_ptr< MTWOutput > MTWOutputPtr;


inline const char * getType( Utility::Output const* )
{
	return "Output";
}

inline const char * getType( Utility::WOutput const* )
{
	return "WOutput";
}

typedef BasicCallbackOutput<char> CallbackOutput;
typedef BasicCallbackOutput<wchar_t> WCallbackOutput;

} // namespace Utility

#endif /* OUTPUT_H_ */
