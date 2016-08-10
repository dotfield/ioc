/*
 * FileBasedList.h
 *
 *  Created on: 7 Jan 2014
 *      Author: neil
 */

#ifndef FILEBASEDLIST_H_
#define FILEBASEDLIST_H_

#include "api.h"

#include <IOC/detail/BuilderParamBinders.h> // mostly for IOC::Proxy and also brings in vector and set
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <Utility/fileutils.h>


namespace IOC { namespace Utils {
/*
template< typename T >
void processLine( std::vector< T > & coll, std::string const& line )
{
	try
	{
		T val = boost::lexical_cast< T >( line );
		coll.push_back( val );
	}
	catch( std::bad_cast )
	{
		std::ostringstream oss;
		oss << "Error converting " << line << " to type " << getType( static_cast<const T *>( nullptr ) );
		throw std::invalid_argument( oss.str() );
	}
}

template< typename T >
void processLine( std::set< T > & coll, std::string const& line )
{
	try
	{
		T val = boost::lexical_cast< T >( line );
		if( !coll.insert(val).second )
		{
			std::ostringstream oss;
			oss << "Duplicate value " << line << " in set";
			throw std::invalid_argument( oss.str() );
		}
	}
	catch( std::bad_cast )
	{
		std::ostringstream oss;
		oss << "Error converting " << line << " to type " << getType( static_cast<const T *>( nullptr ) );
		std::cerr << __FILE__ << ", line " << __LINE__ << " input line " << line << " casting error " << oss.str () << std::endl;
		throw std::invalid_argument( oss.str() );
	}
}

// COLL_TYPE is either vector or set.

template< typename COLL_TYPE >
class FileBasedList : public Proxy< COLL_TYPE >
{
public:
	typedef Proxy< COLL_TYPE > proxy_type;

	FileBasedList( std::string const& filepath )
	{
		COLL_TYPE coll = fileBasedCollection(
		std::ifstream ifs( filepath );
		if( !ifs.is_open() )
		{
			std::ostringstream oss;
			oss << "Failure opening " << filepath;
			throw std::invalid_argument( oss.str() );
		}

		std::string line;
		while( std::getline( ifs, line ) ) // each line is a string
		{
			line = trim(line);
			if( !line.empty() )
			{
				processLine( this->m_inst, line );
			}
		}
	}
};

*/

template< typename COLL_TYPE >
class FileBasedList : public IOC::Proxy< COLL_TYPE >
{
public:
	typedef IOC::Proxy< COLL_TYPE > proxy_type;

	explicit FileBasedList( std::string const& filepath )
	{
		Utility::fileBasedCollection( filepath, &this->m_inst,
				Utility::lexical_converter< typename COLL_TYPE::value_type >() );
	}
};

typedef FileBasedList<std::vector<int> > FileBasedIntVector;
typedef FileBasedList<std::vector<std::string > > FileBasedStringVector;
typedef FileBasedList<std::set<int> > FileBasedIntSet;
typedef FileBasedList<std::set<std::string> > FileBasedStringSet;

} }

#endif /* FILEBASEDLIST_H_ */
