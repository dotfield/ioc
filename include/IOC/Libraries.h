#pragma once
#ifndef IOC_LIBRARIES_H_
#define IOC_LIBRARIES_H_

#include "ioc_api.h"

namespace IOC {

class IOC_API Library
{
public:
	virtual ~Library();

	virtual const DLObject * getSymbol( str_cref name, bool throwIfNotFound=false ) const = 0;

	virtual std::string getAlias() const = 0;
	virtual std::string getPath() const = 0;
};

// this function must be implemented in the library implementation file
// It will always throw if the library could not be opened. This will either
// happen because it doesn't exist or because there was a fault with the library
LibraryPtr openLibrary( str_cref name, str_cref path );

class IOC_API LibraryTable
{
protected:
	LibraryTable();

public:
	virtual ~LibraryTable();
	virtual const Library & addLibrary( str_cref name, str_cref path ) = 0;

	// in your code you may define your own "Library" objects, which can provide symbols.
	// These are called "static" libraries because they are not dynamically loaded
	// (although they will possibly be compiled as DLLs, but it is their loading that
	// is static). The symbols in these do not need to be exported to be accessible
	virtual const Library & addStaticLibrary( str_cref name, LibraryPtr lib ) = 0;

	// no-throw version returns a NULL pointer if not found
	virtual const Library * getLibraryNoThrow( str_cref name ) const = 0;
	
	// this version throws if the library is not found
	const Library & getLibrary( str_cref name ) const;
};

}

#endif
