/*
 * FileStringList.cpp
 *
 *  Created on: 27 Dec 2013
 *      Author: neil
 */

// A file that contains a string per line.

#include "stdafx.h"

#include <IOC/BuilderNParams.h>
#include <set>
#include <vector>

#include <IOCInterfaces/FileBasedList.h>
// file-opening and casting with meaningful exceptions should be in Utility.

namespace IOC { namespace Utils {

typedef Builder1Param< FileBasedIntVector, FileBasedIntVector::proxy_type, std::string > FileBasedIntVectorBuilder;
typedef Builder1Param< FileBasedStringVector, FileBasedStringVector::proxy_type, std::string > FileBasedStringVectorBuilder;
typedef Builder1Param< FileBasedIntSet, FileBasedIntSet::proxy_type, std::string > FileBasedIntSetBuilder;
typedef Builder1Param< FileBasedStringSet, FileBasedStringSet::proxy_type, std::string > FileBasedStringSetBuilder;

/*
// This gets complicated now. FileBasedList of FileBasedList. The outer one must be strings, and we assume vector of strings
// Each one of those is a filename..

// It would however make sense to move these classes into IOCInterfaces so it can be used by IOC libraries
// Ideally also most of the functionality of reading CSV files and converting to collections could go into Utility and we just put
// a CSV wrapper around it to create classes that derive from IOC::Proxy< collection >

template< typename COLL_TYPE >
class FileBasedListOfLists : public Proxy< std::vector< COLL_TYPE > >
{
public:
	typedef Proxy< std::vector< COLL_TYPE > > proxy_type;

	explicit FileBasedListOfLists( std::string const& filepath ) // this contains a list of file names, each of which contains a collection of the same type
	{
		std::ifstream ifs( filepath );
		if( !ifs.is_open() )
		{
			std::ostringstream oss;
			oss << "Failure opening " << filepath;
			throw std::invalid_argument( oss.str() );
		}

		std::string line;
		while( std::getline( ifs, line ) )
		{
			line = trim(line);
			if( !line.empty() )
			{
				FileBasedList< COLL_TYPE > fbl( line );
				this->m_inst.push_back( fbl.get() );
			}
		}
	}
};

typedef FileBasedListOfLists<std::vector<int> > FileBasedListOfIntVector;
typedef FileBasedListOfLists<std::vector<std::string > > FileBasedListOfStringVector;
typedef FileBasedListOfLists<std::set<int> > FileBasedListOfIntSet;
typedef FileBasedListOfLists<std::set<std::string> > FileBasedListOfStringSet;

typedef Builder1Param< FileBasedListOfIntVector, FileBasedListOfIntVector::proxy_type, std::string > FileBasedListOfIntVectorBuilder;
typedef Builder1Param< FileBasedListOfStringVector, FileBasedListOfStringVector::proxy_type, std::string > FileBasedListOfStringVectorBuilder;
typedef Builder1Param< FileBasedListOfIntSet, FileBasedListOfIntSet::proxy_type, std::string > FileBasedListOfIntSetBuilder;
typedef Builder1Param< FileBasedListOfStringSet, FileBasedListOfStringSet::proxy_type, std::string > FileBasedListOfStringSetBuilder;
*/

} }

extern "C" {
	using namespace IOC::Utils;
	using IOC::BuilderFactoryImpl;

	IOC_API BuilderFactoryImpl< FileBasedIntVectorBuilder > g_FileBasedIntVector;
	IOC_API BuilderFactoryImpl< FileBasedStringVectorBuilder > g_FileBasedStringVector;
	IOC_API BuilderFactoryImpl< FileBasedIntSetBuilder > g_FileBasedIntSet;
	IOC_API BuilderFactoryImpl< FileBasedStringSetBuilder > g_FileBasedStringSet;
}
