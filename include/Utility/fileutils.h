/*
 * fileutils.h
 *
 *  Created on: 3 Feb 2014
 *      Author: neil
 */

#ifndef UTILITY_FILEUTILS_H_
#define UTILITY_FILEUTILS_H_

#include <boost/filesystem/fstream.hpp>
#include "strutils.h"

namespace Utility {

template < typename COLL >
COLL fileBasedCollection( std::string const& filepath,
		Converter< typename COLL::value_type > converter )
{
	boost::filesystem::ifstream ifs( filepath ); // throws if fails to open

	COLL coll;
	std::string line;
	while ( std::getline( ifs, line ) )
	{
		line = trim( line );
		if( !line.empty() )
		{
			checkedInsert( coll, converter( line ) );
		}
	}

	return coll;
}

// writes into the collection then returns the pointer
template < typename COLL >
COLL* fileBasedCollection( std::string const& filepath, COLL * coll,
		Converter< typename COLL::value_type > converter )
{
	std::ifstream ifs( filepath ); // throws if fails to open

	if( !ifs.is_open() )
	{
		std::ostringstream oss;
		oss << "Failed to open input file " << filepath;
		throw std::runtime_error( oss.str() );
	}

	std::string line;
	while ( std::getline( ifs, line ) )
	{
		line = trim( line );
		if( !line.empty() )
		{
			checkedInsert( *coll, converter( line ) );
		}
	}

	return coll;
}

}


#endif /* FILEUTILS_H_ */
