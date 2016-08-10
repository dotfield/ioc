#include "stdafx.h"

#include <IOC/Libraries.h>
#include <IOC/DLObject.h>
#include <dlfcn.h>
#include <sstream>
#include <algorithm> 
#include <stdexcept>
#include <boost/noncopyable.hpp>
// Aside from our use of #pragma once, this is the only section in IOC that is Windows-specific.
// If anyone wants to run IOC on UNIX/Linux then write a new version this file 

namespace {

}

namespace IOC {


// full class is in here.
class LibraryUnixImpl : public Library, private boost::noncopyable
{
private:

	void * m_handle;
	std::string m_alias;
	std::string m_path;

public:
	LibraryUnixImpl( str_cref alias, str_cref path ) :
		m_handle( NULL ),
		m_alias( alias ),
		m_path( path )
	{
		if( !( m_handle = dlopen( path.c_str(), RTLD_LAZY | RTLD_GLOBAL ) ) )
		{
			std::ostringstream oss;
			oss << "Failed to open library " << alias
				<< " at " << path << "\n\t" << dlerror();

			throw std::invalid_argument( oss.str() );
		}
	}

		// Technically we should call dlclose on the handle, but we let it leak on purpose
		// because we cannot guarantee the tear-down order and objects in one library may have
		// shared_ptr references to those from another
	~LibraryUnixImpl()
	{
//		dlclose( m_handle );
	}

	// implementation of virtual method. Any symbol loaded must be an object derived from
	// DLObject.

	const DLObject * getSymbol( str_cref symbol, bool throwIfNotFound ) const
	{
		// reinterpet_cast on its own cannot cause a bug. If the symbol exists but does
		// not derive from DLObject, it will only fail when we try using it.

		const DLObject * pSymbol = reinterpret_cast< const DLObject * >
			( dlsym( m_handle, symbol.c_str() ) );

		if( !pSymbol && throwIfNotFound )
		{
			std::ostringstream oss;
			oss << "Symbol " << symbol << " not found in library " <<
				m_alias << " path " << m_path;

			throw std::invalid_argument( oss.str() );
		}
		
		return pSymbol;
	}

	std::string getPath() const
	{
		return m_path;
	}

	std::string getAlias() const
	{
		return m_alias;
	}
};

LibraryPtr openLibrary( str_cref name, str_cref path )
{
	return LibraryPtr( new LibraryUnixImpl( name, path ) );
}
		
}

