#pragma once

#ifndef IOC_BUILDER_N_PARAMS_H_
#define IOC_BUILDER_N_PARAMS_H_

#include "ioc_api.h" 
#include "Builder.h"
#include "detail/ObjectLoader.h"

// although users do not see this next file it must be included in here as it contains all the
// meta-types we use in the implementation below

#include "detail/BuilderParamBinders.h" 

// this file is really an extension of Builder and you need to include this file to create
// builders, not Builder.h

namespace IOC {

// Builder0Params is a special case which
// does not derive from BuilderNParams. That is because BuilderNParams
// implements bindParams() which is a no-op for Builder0Params

template < typename CLASS_TYPE, typename BASE_TYPE, typename SPTR_TYPE=spns::shared_ptr<BASE_TYPE> >
class Builder0Params : public BuilderT< BASE_TYPE, SPTR_TYPE >
{
public:
	Builder0Params( str_cref alias, expr_cref expr )
		: BuilderT<BASE_TYPE>( alias, expr )
	{
	}

	CLASS_TYPE * createObject() const
	{
		return new CLASS_TYPE;
	}
};

// The base class that implements bindParams() for any number of parameters (except 0)
template< typename BASE_TYPE, size_t N, typename SPTR_TYPE=spns::shared_ptr<BASE_TYPE> >
class BuilderNParams : public BuilderT< BASE_TYPE, SPTR_TYPE >
	// acts as base type to all the others (except 0)
	// Does not derive from Builder so you need to derive twice
{
    using BuilderT<BASE_TYPE, SPTR_TYPE>::alias;
    using BuilderT<BASE_TYPE, SPTR_TYPE>::m_creating;
    using BuilderT<BASE_TYPE, SPTR_TYPE>::raiseCircularReferenceError;

protected:
	ParamBinderBase * binders[N];

	BuilderNParams( str_cref alias, expr_cref expr )
		: BuilderT<BASE_TYPE, SPTR_TYPE>( alias, expr )
	{
	}

	typedef BuilderNParams< BASE_TYPE, N, SPTR_TYPE > base_type;

public:
	
	void bindParams( ObjectLoader const& loader )
	{
	    if( !this->alias().empty() )
	        std::clog << "Binding parameters for " << this->alias() << '\n';

		this->circularCheck();

		std::vector< RecursiveExpressionPtr > const& params = this->expr().params();
		if( params.size() != N )
		{
			this->raiseInvalidParameterCountError( N, params.size() );
		}

		// so we have a matching size, so...
		for( size_t i = 0; i < N; ++i )
		{
			binders[i]->bind( loader, *params[i] );
		}
		this->m_creating = false;
	}
};


template < typename CLASS_TYPE, typename BASE_TYPE, typename PARAM1, 
	typename SPTR_TYPE=spns::shared_ptr<BASE_TYPE> >
class Builder1Param : public BuilderNParams< BASE_TYPE, 1, SPTR_TYPE >
{
protected:
	ParameterBinder<PARAM1> binder1;

public:
	Builder1Param( str_cref alias, expr_cref expr )
		: BuilderNParams< BASE_TYPE, 1, SPTR_TYPE >( alias, expr ),
		  binder1( 1, this->binders )
	{
	}

protected:
	CLASS_TYPE * createObject() const
	{
		return new CLASS_TYPE( binder1.obj() );
	}
};

template
 < typename CLASS_TYPE, typename BASE_TYPE, 
	typename PARAM1, typename PARAM2,
	typename SPTR_TYPE=spns::shared_ptr<BASE_TYPE>
 > 
class Builder2Params : public BuilderNParams< BASE_TYPE, 2, SPTR_TYPE >
{
protected:
	ParameterBinder<PARAM1> binder1;
	ParameterBinder<PARAM2> binder2;

public:
	Builder2Params( str_cref alias, expr_cref expr )
		: BuilderNParams< BASE_TYPE, 2, SPTR_TYPE >( alias, expr ),
		  binder1( 1, this->binders ),
		  binder2( 2, this->binders )
	{
	}

protected:
	CLASS_TYPE * createObject() const
	{
		return new CLASS_TYPE( binder1.obj(), binder2.obj() );
	}
};



// The famous Builder3Params. Google for this and "variadic templates" and you'll
// find a post on comp.std.c++ or the archive of it here:
// http://www.mofeel.net/1176-comp-std-c++/1520.aspx
// Since then though I have improved it by getting the bindParams bit done polymorphic...
// Of course C++11 will have initializer lists and our classes might use those,
// so not sure Douglas Gregor's solution will be totally accurate

template
  < 
    typename CLASS_TYPE, typename BASE_TYPE, 
	typename PARAM1, typename PARAM2, typename PARAM3,
	typename SPTR_TYPE=spns::shared_ptr<BASE_TYPE>
  > 
class Builder3Params : public BuilderNParams< BASE_TYPE, 3, SPTR_TYPE >
{
protected:
	ParameterBinder<PARAM1> binder1;
	ParameterBinder<PARAM2> binder2;
	ParameterBinder<PARAM3> binder3;

public:
	Builder3Params( str_cref alias, expr_cref expr )
		: BuilderNParams< BASE_TYPE, 3, SPTR_TYPE >( alias, expr ),
		  binder1( 1, this->binders ),
		  binder2( 2, this->binders ),
		  binder3( 3, this->binders )
	{
	}

protected:
	CLASS_TYPE * createObject() const
	{
		return new CLASS_TYPE
		  ( 
			binder1.obj(), binder2.obj(), binder3.obj()
		  );
	}
};

template
  < 
    typename CLASS_TYPE, typename BASE_TYPE, 
	typename PARAM1, typename PARAM2, typename PARAM3,
	typename PARAM4,
	typename SPTR_TYPE=spns::shared_ptr<BASE_TYPE>
  > 
class Builder4Params : public BuilderNParams< BASE_TYPE, 4, SPTR_TYPE >
{
protected:
	ParameterBinder<PARAM1> binder1;
	ParameterBinder<PARAM2> binder2;
	ParameterBinder<PARAM3> binder3;
	ParameterBinder<PARAM4> binder4;

public:
	Builder4Params( str_cref alias, expr_cref expr )
		: BuilderNParams< BASE_TYPE, 4, SPTR_TYPE >( alias, expr ),
		  binder1( 1, this->binders ),
		  binder2( 2, this->binders ),
		  binder3( 3, this->binders ),
		  binder4( 4, this->binders )
	{
	}

protected:
	CLASS_TYPE * createObject() const
	{
		return new CLASS_TYPE
		  ( 
			binder1.obj(), binder2.obj(), binder3.obj(),
			binder4.obj()
		  );
	}
};

template
  < 
    typename CLASS_TYPE, typename BASE_TYPE, 
	typename PARAM1, typename PARAM2, typename PARAM3,
	typename PARAM4, typename PARAM5,
	typename SPTR_TYPE=spns::shared_ptr<BASE_TYPE>
  > 
class Builder5Params : public BuilderNParams< BASE_TYPE, 5, SPTR_TYPE >
{
protected:
	ParameterBinder<PARAM1> binder1;
	ParameterBinder<PARAM2> binder2;
	ParameterBinder<PARAM3> binder3;
	ParameterBinder<PARAM4> binder4;
	ParameterBinder<PARAM5> binder5;

public:
	Builder5Params( str_cref alias, expr_cref expr )
		: BuilderNParams< BASE_TYPE, 5, SPTR_TYPE >( alias, expr ),
		  binder1( 1, this->binders ),
		  binder2( 2, this->binders ),
		  binder3( 3, this->binders ),
		  binder4( 4, this->binders ),
		  binder5( 5, this->binders )
	{
	}

protected:
	CLASS_TYPE * createObject() const
	{
		return new CLASS_TYPE
		  ( 
			binder1.obj(), binder2.obj(), binder3.obj(),
			binder4.obj(), binder5.obj()
		  );
	}
};

template
  < 
    typename CLASS_TYPE, typename BASE_TYPE, 
	typename PARAM1, typename PARAM2, typename PARAM3,
	typename PARAM4, typename PARAM5, typename PARAM6,
	typename SPTR_TYPE=spns::shared_ptr<BASE_TYPE>
  > 
class Builder6Params : public BuilderNParams< BASE_TYPE, 6, SPTR_TYPE >
{
protected:
	ParameterBinder<PARAM1> binder1;
	ParameterBinder<PARAM2> binder2;
	ParameterBinder<PARAM3> binder3;
	ParameterBinder<PARAM4> binder4;
	ParameterBinder<PARAM5> binder5;
	ParameterBinder<PARAM6> binder6;

public:
	Builder6Params( str_cref alias, expr_cref expr )
		: BuilderNParams< BASE_TYPE, 6, SPTR_TYPE >( alias, expr ),
		  binder1( 1, this->binders ),
		  binder2( 2, this->binders ),
		  binder3( 3, this->binders ),
		  binder4( 4, this->binders ),
		  binder5( 5, this->binders ),
		  binder6( 6, this->binders )
	{
	}

protected:
	CLASS_TYPE * createObject() const
	{
		return new CLASS_TYPE
		  ( 
			binder1.obj(), binder2.obj(), binder3.obj(),
			binder4.obj(), binder5.obj(), binder6.obj()
		  );
	}
};

template
  < 
    typename CLASS_TYPE, typename BASE_TYPE, 
	typename PARAM1, typename PARAM2, typename PARAM3,
	typename PARAM4, typename PARAM5, typename PARAM6,
	typename PARAM7,
	typename SPTR_TYPE=spns::shared_ptr<BASE_TYPE>
  > 
class Builder7Params : public BuilderNParams< BASE_TYPE, 7, SPTR_TYPE >
{
protected:
	ParameterBinder<PARAM1> binder1;
	ParameterBinder<PARAM2> binder2;
	ParameterBinder<PARAM3> binder3;
	ParameterBinder<PARAM4> binder4;
	ParameterBinder<PARAM5> binder5;
	ParameterBinder<PARAM6> binder6;
	ParameterBinder<PARAM7> binder7;

public:
	Builder7Params( str_cref alias, expr_cref expr )
		: BuilderNParams< BASE_TYPE, 7, SPTR_TYPE >( alias, expr ),
		  binder1( 1, this->binders ),
		  binder2( 2, this->binders ),
		  binder3( 3, this->binders ),
		  binder4( 4, this->binders ),
		  binder5( 5, this->binders ),
		  binder6( 6, this->binders ),
		  binder7( 7, this->binders )
	{
	}

protected:
	CLASS_TYPE * createObject() const
	{
		return new CLASS_TYPE
		  ( 
			binder1.obj(), binder2.obj(), binder3.obj(),
			binder4.obj(), binder5.obj(), binder6.obj(),
			binder7.obj()
		  );
	}
};

template
  < 
    typename CLASS_TYPE, typename BASE_TYPE, 
	typename PARAM1, typename PARAM2, typename PARAM3,
	typename PARAM4, typename PARAM5, typename PARAM6,
	typename PARAM7, typename PARAM8,
	typename SPTR_TYPE=spns::shared_ptr<BASE_TYPE>
  > class Builder8Params : public BuilderNParams< BASE_TYPE, 8, SPTR_TYPE >
{
protected:
	ParameterBinder<PARAM1> binder1;
	ParameterBinder<PARAM2> binder2;
	ParameterBinder<PARAM3> binder3;
	ParameterBinder<PARAM4> binder4;
	ParameterBinder<PARAM5> binder5;
	ParameterBinder<PARAM6> binder6;
	ParameterBinder<PARAM7> binder7;
	ParameterBinder<PARAM8> binder8;

public:
	Builder8Params( str_cref alias, expr_cref expr )
		: BuilderNParams< BASE_TYPE, 8, SPTR_TYPE >( alias, expr ),
		  binder1( 1, this->binders ),
		  binder2( 2, this->binders ),
		  binder3( 3, this->binders ),
		  binder4( 4, this->binders ),
		  binder5( 5, this->binders ),
		  binder6( 6, this->binders ),
		  binder7( 7, this->binders ),
		  binder8( 8, this->binders )
	{
	}

protected:
	CLASS_TYPE * createObject() const
	{
		return new CLASS_TYPE
		  ( 
			binder1.obj(), binder2.obj(), binder3.obj(),
			binder4.obj(), binder5.obj(), binder6.obj(),
			binder7.obj(), binder8.obj()
		  );
	}
};

template
  < 
    typename CLASS_TYPE, typename BASE_TYPE, 
	typename PARAM1, typename PARAM2, typename PARAM3,
	typename PARAM4, typename PARAM5, typename PARAM6,
	typename PARAM7, typename PARAM8, typename PARAM9,
	typename SPTR_TYPE=spns::shared_ptr<BASE_TYPE>
  > class Builder9Params : public BuilderNParams< BASE_TYPE, 9, SPTR_TYPE >
{
protected:
	ParameterBinder<PARAM1> binder1;
	ParameterBinder<PARAM2> binder2;
	ParameterBinder<PARAM3> binder3;
	ParameterBinder<PARAM4> binder4;
	ParameterBinder<PARAM5> binder5;
	ParameterBinder<PARAM6> binder6;
	ParameterBinder<PARAM7> binder7;
	ParameterBinder<PARAM8> binder8;
	ParameterBinder<PARAM9> binder9;

public:
	Builder9Params( str_cref alias, expr_cref expr )
		: BuilderNParams< BASE_TYPE, 9, SPTR_TYPE >( alias, expr ),
		  binder1( 1, this->binders ),
		  binder2( 2, this->binders ),
		  binder3( 3, this->binders ),
		  binder4( 4, this->binders ),
		  binder5( 5, this->binders ),
		  binder6( 6, this->binders ),
		  binder7( 7, this->binders ),
		  binder8( 8, this->binders ),
		  binder9( 9, this->binders )
	{
	}

protected:
	CLASS_TYPE * createObject() const
	{
		return new CLASS_TYPE
		  ( 
			binder1.obj(), binder2.obj(), binder3.obj(),
			binder4.obj(), binder5.obj(), binder6.obj(),
			binder7.obj(), binder8.obj(), binder9.obj()
		  );
	}
};
		

// going up to 10, but could go higher. (Did I go up to 12 before?)

template 
  < 
    typename CLASS_TYPE, typename BASE_TYPE, 
	typename PARAM1, typename PARAM2, typename PARAM3,
	typename PARAM4, typename PARAM5, typename PARAM6,
	typename PARAM7, typename PARAM8, typename PARAM9,
	typename PARAM10,
	typename SPTR_TYPE=spns::shared_ptr<BASE_TYPE>
  > class Builder10Params : public BuilderNParams< BASE_TYPE, 10, SPTR_TYPE >
{
protected:
	ParameterBinder<PARAM1> binder1;
	ParameterBinder<PARAM2> binder2;
	ParameterBinder<PARAM3> binder3;
	ParameterBinder<PARAM4> binder4;
	ParameterBinder<PARAM5> binder5;
	ParameterBinder<PARAM6> binder6;
	ParameterBinder<PARAM7> binder7;
	ParameterBinder<PARAM8> binder8;
	ParameterBinder<PARAM9> binder9;
	ParameterBinder<PARAM10> binder10;

public:
	Builder10Params( str_cref alias, expr_cref expr )
		: BuilderNParams< BASE_TYPE, 10, SPTR_TYPE >( alias, expr ),
		  binder1( 1, this->binders ),
		  binder2( 2, this->binders ),
		  binder3( 3, this->binders ),
		  binder4( 4, this->binders ),
		  binder5( 5, this->binders ),
		  binder6( 6, this->binders ),
		  binder7( 7, this->binders ),
		  binder8( 8, this->binders ),
		  binder9( 9, this->binders ),
		  binder10( 10, this->binders )
	{
	}

protected:
	CLASS_TYPE * createObject() const
	{
		return new CLASS_TYPE
		  ( 
			binder1.obj(), binder2.obj(), binder3.obj(),
			binder4.obj(), binder5.obj(), binder6.obj(),
			binder7.obj(), binder8.obj(), binder9.obj(),
			binder10.obj()
		  );
	}
};

// that's it. 
// When you create a builder you just do this:
/*

// note you can use vector<shared_ptr<Bar> > but you can use vector<Bar> even if Bar
// is not a type that can fit in vector because in reality you will get vector<shared_ptr<Bar> >
// same with the second parameter of map.

--
typedef IOC::Builder6Params< MyDerived, MyBase, 
	std::string, bool, double, Foo, std::vector<Bar>, std::map<int, Baz> 
	> MyBuilder;

	// this part must not be in a namespace. declspec bit can be replaced with a macro
	// or use a .def file as an alternative.

__declspec(dllexport) IOC::BuilderFactoryImpl< MyBuilder > myBuilder;

// That really is all you need to do... 

*/

}

#endif
