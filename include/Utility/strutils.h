/*
 * strconvert.h
 *
 *  Created on: 3 Feb 2014
 *      Author: neil
 */

#ifndef UTILITY_STRCONVERT_H_
#define UTILITY_STRCONVERT_H_

// This class provides conversion from string to basic types
// with a meaningful exception if it fails. The exception type
// is invalid_argument not bad_cast as bad_cast does not take a custom string
// and in any case is really intended for dynamic_cast and not lexical casts.

#include <sstream>
#include <stdexcept>
#include "getType.h"
#include <set>
#include <map>
#include <vector>
#include <algorithm>
#include <functional>
#include <locale>

namespace Utility
{

template< typename T >
void convert_str_to( std::string const& str, T& t )
{
	std::istringstream iss( str );
	std::string garbage;
	bool failed = false;
	if( iss >> t )
	{
		iss >> garbage;
	}
	else
		failed = true;

	if( failed || !garbage.empty() )
	{
		std::ostringstream oss;
		oss << "Error converting " << str << " to type " << getType( &t );
		throw std::invalid_argument( oss.str() );
	}
}

inline void convert_str_to( std::string const& str, std::string & target )
{
	target = str;
}

template< typename T >
T strconvert( std::string const& str )
{
	T t;
	convert_str_to( str, t );
	return t;
}

inline std::string strconvert( std::string const& str )
{
	return str;
}

class StrTuple
{
private:
	std::vector< std::string > m_tokens;
public:
	StrTuple() = default;
	explicit StrTuple( std::vector< std::string > tokens )
		: m_tokens( tokens )
	{
	}

	template< typename... Args >
	explicit StrTuple( Args... args )
		: m_tokens( std::forward( args... ) )
	{
	}

	typedef std::vector< std::string >::const_iterator const_iterator;

	bool empty() const
	{
		return m_tokens.empty();
	}

	size_t size() const
	{
		return m_tokens.size();
	}

	// returns empty if index out of range, does not throw
	std::string const& at( size_t idx ) const
	{
		static const std::string emptyString;
		return idx < m_tokens.size() ? m_tokens[ idx ] : emptyString;
	}

	std::string const& operator[]( size_t idx ) const
	{
		return at( idx );
	}

	// get as different types
	template< typename T > T get( size_t idx ) const
	{
		return strconvert<T>( at( idx ) );
	}

	template< typename T > void get( size_t idx , T& t ) const
	{
		convert_str_to( at(idx), t );
	}
};

inline std::string trim( std::string const & str )
{
	const char * whites = " \n\t\r";
	std::string::size_type start = str.find_first_not_of( whites );
	std::string::size_type end = str.find_last_not_of( whites );

	if( end != std::string::npos )
	{
		return str.substr( start, end + 1 - start );
	}
	else
		return std::string();
}

// trim2 trims anything from the ends that is not alphanumeric
// so ( hello ) should return hello
// but characters in the middle remain so you're doesn't remove the quote

inline std::string trim2( std::string const& str )
{
	const std::locale& classic( std::locale::classic() );
	auto isalnumLambda = [&classic](char x){ return std::isalnum(x, classic); };
	auto start = std::find_if( str.begin(), str.end(), isalnumLambda );
	if( start != str.end() )
	{
		auto strend = std::find_if( str.rbegin(), str.rend(), isalnumLambda );
		return std::string( start, strend.base() );
	}
	return std::string();
}

// could move this into cpp
inline StrTuple delimitedText( std::string const& line, char delim ) // populates m_tokens
{
	std::istringstream iss( line );
	std::string token;

	std::vector< std::string > tokens;

	while( std::getline( iss, token, delim ) )
	{
		tokens.push_back( token );
	}

	return StrTuple( std::move( tokens ) );
};

// to convert "into" the T use Converter<T *>
// and get the function to hold the T* and "set" it

template< typename T >
class Converter
{
public:
	typedef std::function< T( std::string const& ) > function_type;

private:

	 function_type m_func;

public:
	explicit Converter( function_type func )
		: m_func( func )
	{
	}

	T operator()( std::string const& str )
	{
		return m_func( str );
	}
};

// In case you want a Converter<T>
template< typename T >
Converter<T> lexical_converter()
{
	return Converter<T>( strconvert<T> );
}

inline Converter< StrTuple > delimitedConverter( char delim )
{
	return Converter< StrTuple >
		(
			[ delim ]( std::string const& str )
			{
				return delimitedText( str, delim );
			}
		);
}

// nothing to validate here. Note we cannot use std::vector<T> as the type
template< typename ITER >
void validateInsert( ITER iterator )
{
}

template< typename K, typename V >
void validateInsert( std::pair< typename std::map<K,V>::iterator, bool > res )
{
	if( !res.second )
	{
		std::ostringstream oss;
		oss << "Duplicate key in map: " << res.first->first;
		throw std::invalid_argument( oss.str() );
	}
}

template< typename T >
void validateInsert( std::pair< typename std::set<T>::iterator, bool > res )
{
	if( !res.second )
	{
		std::ostringstream oss;
		oss << "Duplicate value in set: " << *res.first;
		throw std::invalid_argument( oss.str() );
	}
}

template< typename COLL >
void checkedInsert( COLL & coll, typename COLL::const_reference element )
{
	// this sequence works for all collection types: vector, map and set
	// vector always succeeds, map and set fail if it's a duplicate
	validateInsert( coll.insert( coll.end(), element ) );
}

inline std::string capitalise( const std::string& word )
{
	bool first = true;
	std::string out;
	const std::locale& classic( std::locale::classic() );
	out.reserve( word.size() );
	for( char ch : word )
	{
		out.push_back( first ? std::toupper( ch, classic ) : std::tolower( ch, classic ) );
		first = false;
	}
	return out;
}

inline Converter<std::string> capitaliseConverter()
{
	return Converter<std::string>( capitalise );
}

}

#endif /* STRCONVERT_H_ */
