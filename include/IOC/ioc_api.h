#pragma once

// file should be included from external users
// Internal libraries will already define this 

#include "iocfwd.h"

#if defined _WIN32 || defined _WIN64

#ifndef IOC_API
#define IOC_API __declspec( dllimport )

#ifdef _DEBUG

#pragma comment( lib, "IOCD.lib" )

#else

#pragma comment( lib, "IOC.lib" )

#endif
#endif

#elif __GNUC__ >= 4
#define IOC_API __attribute__ ((visibility ("default")))
#endif

namespace IOC {
	// main entry point for application that wants to run the system
	RunnablePtr IOC_API getRunnable( str_cref filePath, str_cref name );

	// this should be done once if you want to use SequentialRunnableList or
	// ParallelRunnableList, if you don't want to use them there is no need to call it.
	// If you call it more than once it does not matter. If it's called it must be
	// called before getRunnable().

	void IOC_API initIOCObjLibrary();

	typedef LibraryTable& LibraryTableRef;
	LibraryTableRef IOC_API libraryTableInstance();

	ObjectLoaderPtr IOC_API getObjectLoader( str_cref filePath );

}

namespace Utility {
	inline const char * const getType( const IOC::Runnable * )
	{
		return "IOC::Runnable";
	}
}
