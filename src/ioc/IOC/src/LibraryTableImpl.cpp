#include "stdafx.h"
#include <IOC/Libraries.h>
#include <Utility/mapLookup.h>
#include <stdexcept>

namespace IOC {

LibraryTable::LibraryTable()
{
}

LibraryTable::~LibraryTable()
{
}

Library::~Library()
{
}

const Library& LibraryTable::getLibrary( str_cref name ) const
{
	const Library * pLib = getLibraryNoThrow( name );
	if( !pLib )
	{
		std::ostringstream oss;
		oss << "Library " << name << " not defined";

			// We could give a dump of what has been...
		throw std::invalid_argument( oss.str() );
	}

	return *pLib;
}

class LibraryTableImpl : public LibraryTable
{
private:

	std::map< std::string, LibraryPtr > m_map;

	LibraryPtr & prepare( str_cref name, str_cref path )
	{
		LibraryPtr & pLib = m_map[ name ];

		if( pLib )
		{
			if( pLib->getPath() != path )
			{
				std::ostringstream oss;
				oss << "Library with alias " << name << " previously defined at " 
					<< pLib->getPath() << ", now at " << path;

				throw std::invalid_argument( oss.str() );
			}
		}
		return pLib;
	}

public:

	// really we should only attempt to add a library once,
	// however if we opened it previously at the same path as the
	// one we try now, we allow it
	const Library & addLibrary( str_cref name, str_cref path )
	{
		LibraryPtr & pLib = prepare( name, path );
		if( !pLib )
		{
			// becase this can throw we are slightly overcautious to put it into a 
			// local variable and only swap it after it has been shown to work.
			LibraryPtr openLib = openLibrary( name, path );
			openLib.swap( pLib );

		}
		return *pLib;
	}

	const Library & addStaticLibrary( str_cref name, LibraryPtr lib )
	{
		LibraryPtr & pLib = prepare( name, lib->getPath() );
		if( !pLib )
		{
			pLib.swap( lib );
		}
		return *pLib;
	}

	const Library* getLibraryNoThrow( str_cref name ) const
	{
		LibraryPtr pLib;
		Utility::mapLookup( m_map, name, pLib );
		return pLib.get();
	}


};

LibraryTable& libraryTableInstance()
{
	// for this use static initialisation, we are not in multi-threaded model
	// It has to outlive the ConfigLoader, at least the libraries in it must
	// albeit we don't bother freeing their handles.

	static LibraryTableImpl inst;
	return inst;
}

}
