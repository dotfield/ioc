#pragma once
#ifndef IOCFWD_H_
#define IOCFWD_H_
// our only includes here. 

#include <memory>
#include <string> // only because forwardly-declaring it is too tricky

// I am using a namespace alias for the namespace from which we use shared_ptr
// because in the future the policy may be to use std::shared_ptr so we would
// only have to change one place - here with the namespace alias and the
// include above.

// This is the namespace we are using for this project. It can be changed to boost
// by putting in spns = boost and including that header here.

// Internally for 3rd party objects we support boost too.

namespace spns = std;

// the namespace from which we are using shared_ptr

// These are only classes that are to be used external, i.e. outside of the IOC project,
// essentially for 
namespace IOC
{
	class Builder;
	class BuilderFactory;
	class DLObject;
	class Library;
	class LibraryStaticImpl;
	class LibraryTable;
	class ObjectLoader;
	
	class Runnable;
	class RecursiveExpression;
	template< typename T, typename SPTR_TYPE = spns::shared_ptr<T> > class BuilderT;

	// We typedef these. We don't "instantiate" any of them in this library, because it
	// is actually the wrong template to instantiate. The correct one in boost anyway is 
	// boost::detail::sp_counted_impl_p but that is dependent on the implementation of shared_ptr so we don't.

	typedef spns::shared_ptr< Builder > BuilderPtr;
	typedef spns::shared_ptr< BuilderFactory > BuilderFactoryPtr;
	typedef spns::shared_ptr< Library > LibraryPtr;
	typedef spns::shared_ptr< RecursiveExpression > RecursiveExpressionPtr;
	typedef spns::shared_ptr< Runnable > RunnablePtr;

	// expose this to allow the user to control the Object Loading and to load things
	// manually other than the runnable. This can be left open if necessary
	typedef spns::shared_ptr< ObjectLoader > ObjectLoaderPtr;

	// declare this globally?
	typedef std::string const& str_cref;
	typedef RecursiveExpression const& expr_cref;

	// called from BuilderNParams.h, not meant to be called as an entry point.
	
	// plugin-writers should include BuilderNParams.h then declare their BuilderFactoryImpl instance.
	// the implementation file will implement this
}

#endif
