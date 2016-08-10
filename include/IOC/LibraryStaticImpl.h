#pragma once
#ifndef IOC_LIBRARY_STATIC_IMPL_H_
#define IOC_LIBRARY_STATIC_IMPL_H_

#include "iocfwd.h"
#include "Libraries.h"

extern template class IOC_API std::map< std::string, const IOC::DLObject * >;

namespace IOC
{

// in reality this is no more than a wrapper for the map above but it wraps the Library interface
// (which is definitely NOT simply a std::map)

class IOC_API LibraryStaticImpl : public Library
{
private:
	std::string m_name;
	std::string m_path; // does not have to be the full path of this library

	std::map< std::string, const DLObject * > m_objects;
public:
	LibraryStaticImpl( str_cref name, str_cref path )
		: m_name( name ), m_path( path )
	{
	}

	std::string getAlias() const
	{
		return m_name;
	}

	std::string getPath() const 
	{
		return m_path;
	}

	void addSymbol( str_cref name, const DLObject * obj );

	const DLObject * getSymbol( str_cref name, bool throwIfNotFound=false ) const;
};

}

#endif
