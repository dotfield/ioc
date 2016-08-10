#pragma once

#ifndef IOC_BUILDER_H_
#define IOC_BUILDER_H_

#include "DLObject.h"
#include "detail/ObjectLoader.h"
#include "detail/RecursiveExpression.h"
#include <typeinfo>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <Utility/getType.h>

namespace IOC {


// TypeError means the user has specified the wrong type and is used only internally.
// Users of IOC will get invalid_argument

class IOC_API TypeError : public std::invalid_argument
{
public:
	explicit TypeError( std::string const& err );
};

// A single builder can only ever build one object.
// You can have multiple instances of a builder that each creates an object.
//
// All builders must be loaded prior to the start of the run phase. If you want
// to create objects after that you need Factories not Builders, but you can of
// course have builders to factories.

class IOC_API Builder
{
protected:

	std::string m_alias; // its name if it's a named object otherwise empty
	expr_cref m_expr; // the expression 
	 // used in diagnostic error messages only
	mutable bool m_creating;

	explicit Builder( str_cref alias, expr_cref expr )
		: m_alias(alias), m_expr( expr ), m_creating( false )
	{
	}

public:
	virtual ~Builder();

	// this has a default empty implementation, because it might not have any params
	virtual void bindParams( const ObjectLoader & objectLoader )
	{
	}
	
	std::string type() const
	{
		return m_expr.value();
	}

	std::string alias() const
	{
		return m_alias;
	}

	expr_cref expr() const
	{
		return m_expr;
	}

protected:
	void raiseInvalidParameterCountError( size_t expected, size_t found ) const;
	void raiseCircularReferenceError() const;
    void circularCheck() const;
};

// Traits type for smart pointers.
 
// Note: You virtually never write one of these, instead you use BuilderNParams 
template< typename T, typename SPTR_TYPE >
class BuilderT : public Builder
{

public:
	typedef T value_type;
	typedef SPTR_TYPE sptr_type;


private:
	mutable sptr_type m_object;

protected:
	
	BuilderT( str_cref alias, expr_cref expr )
		: Builder( alias, expr )
	{
	}

  // final class must implement the instance of the shared_ptr
	virtual value_type * createObject() const = 0;

public:
	sptr_type getObject() const
	{
		if( !m_object )
		{
		    circularCheck();
			m_creating = true;
			if( !alias().empty() )
			{
				std::clog << "Creating " << alias() << '\n';
			}

			// assumed to be supported by all smart pointers, while
			// .reset() might not be.

			m_object = SPTR_TYPE( createObject() );
			
			if( !alias().empty() )
			{
				std::clog << "Finished creating " << alias() << '\n';
			}
			m_creating = false;
		}
		return m_object;
	}
};

template < typename BUILDER_TYPE > void builder_cast
  (
     BuilderPtr source,
	 spns::shared_ptr< BUILDER_TYPE > & target
  )
{
	// This always uses spns::dynamic_pointer_cast. The builders themselves
	// are always stored in boost pointers, it is the objects that they are
	// building that can be put into other types of smart pointer.
	

	target = spns::dynamic_pointer_cast< BUILDER_TYPE >( source );
	if( !target )
	{
		const typename BUILDER_TYPE::value_type * t = NULL;
		std::ostringstream oss;
		oss << "Type mismatch: " << source->alias() << " of type " <<
			source->type() << " expected type " << Utility::getType( t );

		throw TypeError( oss.str() );
	}
}

// BuilderFactory is actually what you end up creating an instance of
// actually BuilderFactoryImpl

class IOC_API BuilderFactory : public DLObject
{
protected:
	explicit BuilderFactory()
	{
	}

	virtual ~BuilderFactory();
public:
	virtual Builder * create( str_cref alias, expr_cref expr ) const = 0;	
};

template < typename BUILDER_T >
class BuilderFactoryImpl : public BuilderFactory
{
public:
	BuilderFactoryImpl() 
	{
	}

	Builder * create( str_cref alias, expr_cref expr ) const
	{
		return new BUILDER_T( alias, expr );
	}
};

}

#endif
