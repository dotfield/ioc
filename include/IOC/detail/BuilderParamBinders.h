#pragma once

#ifndef IOC_BUILDER_PARAM_BUILDERS_H_
#define IOC_BUILDER_PARAM_BUILDERS_H_

#include "../Builder.h"
#include "ObjectLoader.h"
#include <vector>
#include <map>
#include <set>
#include <functional>
#include <bitset>

#include <boost/shared_ptr.hpp> // also supported

// First we have some binder types
namespace IOC {

template< typename T > class ParameterBinder;
template < typename T > class Proxy;

// Note on this base class. This is rather tricky because bind() is polymorphic
// (actually implemented through doBind() ) but obj() cannot be polymorphic because
// the return type differs (essentially to whatever we need for our constructor).
// (returning boost::any is not an option). 
// This model also uses meta-programming techniques

// We could stick this detail away but it must be included from this file
class IOC_API ParamBinderBase
{
protected:
	
	size_t m_paramNum;
	explicit ParamBinderBase( size_t paramNum  =  0, ParamBinderBase** assignMe=NULL ) 
		: m_paramNum( paramNum )
	{
		if( assignMe )
		{
			assignMe[paramNum-1] = this;
		}
	}

	virtual ~ParamBinderBase();

public:
	// we could manage with this being non-virtual
	void bind( const ObjectLoader & loader, expr_cref expr );

protected:
	virtual void doBind( const ObjectLoader & loader, expr_cref expr ) = 0;

	void handleError( std::exception const& err, str_cref variable ) const;
};

template < typename T, typename SPTR_TYPE = std::shared_ptr<T> >
class ObjParamBinder : public ParamBinderBase
{
	typedef spns::shared_ptr< BuilderT< T, SPTR_TYPE > > builder_type; // used internally only
public:
	typedef SPTR_TYPE object_type; // important for meta-programming use
	
private:
	builder_type m_builder;
public:

	explicit ObjParamBinder( size_t paramNum = 0, ParamBinderBase** assignMe=NULL ) 
		: ParamBinderBase( paramNum, assignMe )
	{
	}

	// this step gets the builder for creating the object
	void doBind( const ObjectLoader & loader, expr_cref expr )
	{
		BuilderPtr param = loader.getBuilder( expr );
		builder_cast( param, m_builder );
	}

	// this method cannot be virtual although it does exist in all versions
	// as its return types are not co-variant. Until we have variadic templates /
	// initialization lists, we will need to call this in ones as below

	object_type obj() const
	{
		try
		{
			return m_builder->getObject();
		}
		catch( std::exception const& err )
		{
			handleError( err, m_builder->alias() );
			throw; 
			   // it won't throw the last error because handleError always throws a new one
				// but putting this throw in here shuts up the compiler about paths
				// not returning a value
		}
	}
};

// T should be one of
// int (all signed integer types)
// size_t (all unsigned integer types)
// double (all floating-point types)
// bool (not considered an integral type)
// std::string

template< typename T > 
class LitParamBinder : public ParamBinderBase
{
public:
	typedef T object_type;

private:
	T m_param;

public:
	explicit LitParamBinder( size_t paramNum = 0, ParamBinderBase** assignMe=NULL ) 
		: ParamBinderBase( paramNum, assignMe ), m_param()
	{
	}

	void doBind( const ObjectLoader & loader, expr_cref expr )
	{
		loader.convert( expr, m_param );
	}

	object_type obj() const // cannot throw here
	{
		return m_param;
	}
};

// this is supported but requires the user to define binder_traits for their
// own enumerated type, and the converter which must support operator() on the string
// and return the enumerated type

template <typename E, typename Converter>
class EnumParamBinder : public ParamBinderBase
{
public:
	typedef E object_type;

private:
	std::string m_param; // not type E

public:
	explicit EnumParamBinder( size_t paramNum = 0, ParamBinderBase** assignMe=NULL ) 
		: ParamBinderBase( paramNum, assignMe )
	{
	}

	void doBind( const ObjectLoader & loader, expr_cref expr )
	{
		m_param = loader.toEnum( expr );
	}

	object_type obj() const // calls user overload
	{
		Converter conv;
		return conv( m_param );
	}
};
		
// binder_traits and ParamBinder are both meta-programming structs, i.e.
// they just contain typedefs

template < typename T > struct binder_traits;

template <typename T>
struct binder_traits
{
	typedef ObjParamBinder< T > binder_type;
};

// we don't use shared pointers of shared pointers...
template <typename T> struct binder_traits< std::shared_ptr< T > >
{
	typedef ObjParamBinder< T, std::shared_ptr<T> > binder_type;
};

template <typename T> struct binder_traits< boost::shared_ptr< T > >
{
    typedef ObjParamBinder< T, boost::shared_ptr<T> > binder_type;
};

template <> struct binder_traits< size_t >
{
	typedef LitParamBinder< size_t > binder_type;
};

template <> struct binder_traits< int >
{
	typedef LitParamBinder< int > binder_type;
};

template <> struct binder_traits< double >
{
	typedef LitParamBinder< double > binder_type;
};

template <> struct binder_traits< float >
{
	typedef LitParamBinder< float > binder_type;
};

template <> struct binder_traits< std::string >
{
	typedef LitParamBinder< std::string > binder_type;
};

template <> struct binder_traits< std::wstring >
{
	typedef LitParamBinder< std::wstring > binder_type;
};

template <> struct binder_traits< bool >
{
	typedef LitParamBinder< bool > binder_type;
};

/*
// we could do the above for short etc. We don't have one for char,
// At present we don't support char as a literal type. We can add if required
// For now we will just enforce that you can't use these types in your builder definitions

// this isn't a "real" struct, it's just a group of typedefs
// we could manage without this class and just use binder_traits directly
// but using this simplifies subsequent code.
// 
template < typename T > struct ParamBinder
{
	typedef binder_traits<T> binder_traits_type;
	typedef typename binder_traits_type::binder_type binder_type;
	typedef typename binder_type::object_type object_type;
};
*/
// for vector types (called List in the config)
template < typename T >
class VecParamBinder : public ParamBinderBase
{
	typedef T element_type;
	typedef ParameterBinder<T> element_binder_type;
	typedef typename ParameterBinder<T>::object_type element_object_type;

	std::vector< element_binder_type > m_elementBinders;
public:
	VecParamBinder( size_t paramNum = 0, ParamBinderBase** assignMe=NULL ) 
		: ParamBinderBase( paramNum, assignMe )
	{
	}
	// what we actually produce
	typedef std::vector< element_object_type > object_type;

protected:

	void doBind( const ObjectLoader & loader, expr_cref expr )
	{
		std::vector< RecursiveExpressionPtr > elements;
		loader.toVector( expr, elements );

		for( size_t i = 0; i < elements.size(); ++i )
		{
			element_binder_type binder( i+1 );
			binder.bind( loader, *elements[i] );
			m_elementBinders.push_back( binder );
		}
	}

public:
	object_type obj() const
	{
		object_type res;
		try
		{
			std::transform( m_elementBinders.begin(), m_elementBinders.end(),
				std::back_inserter( res ), std::bind( &element_binder_type::obj,
				        std::placeholders::_1 ) );

		}
		catch( std::exception const& err )
		{
			handleError( err, "List" ); 
		}
		return res;
	}
};


template < typename T > struct binder_traits< std::vector<T> >
{
	typedef VecParamBinder< T > binder_type;
};


// Set: Type can only literal and must be unique

template< typename T >
class SetParamBinder : public ParamBinderBase
{
	typedef T element_type;

	// key must use LitParamBinder as its type or it's an error
	typedef LitParamBinder<T> element_binder_type;

	std::vector< element_binder_type > m_elementBinders;
	std::set< element_type > m_set;

public:
	typedef std::set< element_type > object_type;
	SetParamBinder( size_t paramNum = 0, ParamBinderBase** assignMe=NULL )
		: ParamBinderBase( paramNum, assignMe )
	{
	}
protected:
	void doBind( const ObjectLoader & loader, expr_cref expr )
	{
		std::vector< RecursiveExpressionPtr > elements;
		loader.toVector( expr, elements );

		for( size_t i = 0; i < elements.size(); ++i )
		{
			element_binder_type binder( i+1 );
			binder.bind( loader, *elements[i] );
			if (! m_set.insert( binder.obj() ).second )
			{
				std::ostringstream oss;
				oss << "Duplicate value " << binder.obj();
				throw std::invalid_argument( oss.str() );
			}
		}
	}

public:
	object_type obj() const
	{
		return m_set;
	}
};

template< typename T > struct binder_traits< std::set<T > >
{
	typedef SetParamBinder<T> binder_type;
};

// do the Map one next.
template< typename K, typename V >
class MapParamBinder : public ParamBinderBase
{
	typedef K key_type;
	typedef V value_type;
	
	// key must use LitParamBinder as its type or it's an error
	// This is resolved at compile time. Maps must have literal keys.
	// Note, you wouldn't normally use double as a key in a map and it's
	// not guaranteed to work but we don't catch that error here.
	typedef LitParamBinder<K> key_binder_type;
    typedef ParameterBinder<V> value_binder_type;
	typedef typename ParameterBinder<V>::object_type value_object_type;
	
	typedef std::map< key_type, value_binder_type > binder_map_type;
	binder_map_type m_valueBinders;

public:
	typedef std::map< key_type, value_object_type > object_type;
	MapParamBinder( size_t paramNum = 0, ParamBinderBase** assignMe=NULL ) 
		: ParamBinderBase( paramNum, assignMe )
	{
	}
protected:
	void doBind( const ObjectLoader & loader, expr_cref expr )
	{
		std::vector< std::pair< RecursiveExpressionPtr, RecursiveExpressionPtr > > pairs;
		loader.toMap( expr, pairs );
		
		for( size_t i = 0; i < pairs.size(); ++i )
		{
			key_binder_type keyBinder( i+ 1 ); // 1 base is used here for error purposes
			expr_cref keyExpr = *pairs[i].first;
			expr_cref valueExpr = *pairs[i].second;
			value_binder_type valueBinder( i + 1 );
			keyBinder.bind( loader, keyExpr );
			valueBinder.bind( loader, valueExpr );

			// insert into our map, checking that 
			if( !m_valueBinders.insert( std::make_pair( keyBinder.obj(), valueBinder ) ).second )
			{
				std::ostringstream oss;
				oss << "Duplicate key " << keyBinder.obj();
				throw std::invalid_argument( oss.str() );
			}
		}
	}
/*
	struct Transformer // for non C++11
	{
	public:
		typename object_type::value_type operator()( typename binder_map_type::const_reference p ) const
		{
			return object_type::value_type( p.first, p.second.obj() );
		}
	};
*/
public:
	object_type obj() const
	{
		object_type res;

		try
		{
			std::transform( m_valueBinders.begin(), m_valueBinders.end(), 
				std::inserter( res, res.end() ),
				[]( typename binder_map_type::const_reference keyValuePair )
				{
			        return typename object_type::value_type( keyValuePair.first, keyValuePair.second.obj() );
				} );
		}
		catch( std::exception const& err )
		{
			handleError( err, "Map" );
		}

		return res;
	}
};

template< typename K, typename V > struct binder_traits< std::map< K, V > >
{
	typedef MapParamBinder< K, V > binder_type;
};

// do the MultiMap one next. A lot of copy-pasted code from Map so consider refactor
template< typename K, typename V >
class MultiMapParamBinder : public ParamBinderBase
{
	typedef K key_type;
	typedef V value_type;

	typedef LitParamBinder<K> key_binder_type;
    typedef typename ParameterBinder<V>::binder_type value_binder_type;
	typedef typename ParameterBinder<V>::object_type value_object_type;

	typedef std::multimap< key_type, value_binder_type > binder_map_type;
	binder_map_type m_valueBinders;

public:
	typedef std::multimap< key_type, value_object_type > object_type;
	MultiMapParamBinder( size_t paramNum = 0, ParamBinderBase** assignMe=NULL )
		: ParamBinderBase( paramNum, assignMe )
	{
	}
protected:
	void doBind( const ObjectLoader & loader, expr_cref expr )
	{
		std::vector< std::pair< RecursiveExpressionPtr, RecursiveExpressionPtr > > pairs;
		loader.toMap( expr, pairs );

		for( size_t i = 0; i < pairs.size(); ++i )
		{
			key_binder_type keyBinder( i+ 1 ); // 1 base is used here for error purposes
			expr_cref keyExpr = *pairs[i].first;
			expr_cref valueExpr = *pairs[i].second;
			value_binder_type valueBinder( i + 1 );
			keyBinder.bind( loader, keyExpr );
			valueBinder.bind( loader, valueExpr );

			// difference from map. insert always works as we are allowed duplicates
			m_valueBinders.insert( std::make_pair( keyBinder.obj(), valueBinder ) );
		}
	}
/*
	struct Transformer // for non C++11
	{
	public:
		typename object_type::value_type operator()( typename binder_map_type::const_reference p ) const
		{
			return object_type::value_type( p.first, p.second.obj() );
		}
	};
*/
public:
	object_type obj() const
	{
		object_type res;

		try
		{
			std::transform( m_valueBinders.begin(), m_valueBinders.end(),
				std::inserter( res, res.end() ),
				[]( typename binder_map_type::const_reference keyValuePair )
				{
			        return typename object_type::value_type( keyValuePair.first, keyValuePair.second.obj() );
				} );
		}
		catch( std::exception const& err )
		{
			handleError( err, "Map" );
		}

		return res;
	}
};

template< typename K, typename V > struct binder_traits< std::multimap< K, V > >
{
	typedef MultiMapParamBinder< K, V > binder_type;
};


// RefParamBinder. This is meant for structs or other objects that are to be
// passed as parameters by reference. The object receiving them is expected
// to copy the object. Note that internally we create them with new, and they
// must take a constructor with their arguments as we don't support setters.
/* example use:
  struct MyStruct
  {
     std::string a;
	 int b;

	 MyStruct( std::string const& aParam, int bParam ) : a(aParam), b(bParam)
	 {
	 }
  };

  typedef Builder2Params< MyStruct, MyStruct, std::string, int, std::shared_ptr<MyStruct> > MyStructBuilder;

  class HasMyStruct
  {
     private:
	    MyStruct m_inst;

	public:
	  explicit HasMyStruct( MyStruct const& inst ) : m_inst( inst )
	    {
		}

		// other interface...
  };

  typedef Builder1Param< HasMyStruct, HasMyStruct, 
	RefParam<MyStruct,	boost::shared_ptr<MyStruct> >, 
	boost::shared_ptr< HasMyStruct > HasMyStructBuilder;
*/


template< typename T, typename SPTR_TYPE = std::shared_ptr<T> >
class RefParamBinder : public ParamBinderBase
{
	typedef spns::shared_ptr< BuilderT< T, SPTR_TYPE > > builder_type;

public:
	typedef T object_type;

private:
	builder_type m_builder;

public:
	explicit RefParamBinder( size_t paramNum = 0, ParamBinderBase** assignMe = NULL )
		: ParamBinderBase( paramNum, assignMe )
	{
	}

	void doBind( const ObjectLoader & loader, expr_cref expr )
	{
		BuilderPtr param = loader.getBuilder( expr );
		builder_cast( param, m_builder );
	}

	object_type const& obj() const
	{
		try
		{
			// smart pointer type must support dereference..
			return *(m_builder->getObject());
		}
		catch( std::exception const& err )
		{
			handleError( err, m_builder->alias() );
			throw; // as above just to prevent "not all paths return a value" warning
		}
	}
};

// this class is a dummy just to put in a parameter list
template< typename T, typename SPTR_TYPE=std::shared_ptr<T> >
class RefParam
{
};

template< typename T, typename S > struct binder_traits< RefParam<T,S> >
{
	typedef RefParamBinder<T,S> binder_type;
};

/*
  Proxy. If the object does not actually construct from parameters
  but is a struct that supports setters or something similar, you create a
  Proxy wrapper for it. You just derive it from Proxy<T> and get your constructor
  to set m_inst

  The underlying type should still normally be default constructible and copyable.
  However one can specialize Proxy<T> for a type T that isn't.

  Example:
    struct A
	{
	   std::string s;
	   int i;
	};

	struct ASetter : public IOC::Proxy< A >
	{
	   ASetter( std::string const& s, int i )
	   {
	       m_inst.s = s;
		   m_inst.i = i;
	   }
    };

    class B
	{
	  public:
	     explicit B( const A& arg );
		 // ...
    };

	typedef Builder2Params< ASetter, Proxy<A>, std::string, int > ABuilder;
	typedef Builder1Param< B, B, Proxy<A> > BBuilder;
*/
template < typename T >
class Proxy
{
public:
   typedef T value_type;
   virtual ~Proxy() {}

protected:
   T m_inst;
   // These two are accessible and can be exported as builders
   Proxy() : m_inst()
   {
   }

   explicit Proxy( T const& t ) : m_inst( t )
   {
   }


public:
   T const& get() const
   {
        return m_inst;
   }
};

template< typename PROXY_TYPE, typename SPTR_TYPE=std::shared_ptr< PROXY_TYPE > >
class ProxyParamBinder : public ParamBinderBase
{
	typedef spns::shared_ptr< BuilderT< PROXY_TYPE, SPTR_TYPE > > builder_type;
	typedef PROXY_TYPE proxy_type;
public:
	typedef typename PROXY_TYPE::value_type object_type;

private:
	builder_type m_builder;

public:
	explicit ProxyParamBinder( size_t paramNum = 0, ParamBinderBase** assignMe = NULL )
		: ParamBinderBase( paramNum, assignMe )
	{
	}

	void doBind( const ObjectLoader & loader, expr_cref expr )
	{
		BuilderPtr param = loader.getBuilder( expr );
		builder_cast( param, m_builder );
	}

	object_type obj() const
	{
		try
		{
			return m_builder->getObject()->get();
		}
		catch( std::exception const& err )
		{
			handleError( err, m_builder->alias() );
			throw; // as above just to prevent "not all paths return a value" warning
		}
	}
};

// Note: it will not "smart-pointer" TYPE for you. Because TYPE will often not be smart-pointered.
// Therefore you must smart-pointer it yourself if you want it that way.
// If you want to use a different smart-pointer for Proxy<TYPE> itself then you have to use
// that as the parameter and write your own binder_traits override for it to point it to
// ProxyParamBinder.

template< typename TYPE > 
	struct binder_traits< Proxy<TYPE> >
{
	typedef ProxyParamBinder< Proxy<TYPE> > binder_type;
};


// Can be constructed from any of the following:
// - an unsigned integer
// - a string comprising of 1 and 0 characters
// - for us also, a list of indexes - must bind to SetParam
// - a proxy to set<unsigned> but cannot be a proxy to an int or string
// If it turns out to be a proxy to a bitset of course that will be resolved later.

template< typename T > // T is std::bitset<N>
class BitsetParamBinder : public ParamBinderBase
{
public:
	typedef T object_type; // T is what we are constructing.

private:

	object_type m_bitset;

public:

	BitsetParamBinder( size_t paramNum = 0, ParamBinderBase** assignMe=NULL )
		: ParamBinderBase( paramNum, assignMe )
	{
	}

protected:
	void doBind( const ObjectLoader & loader, expr_cref expr )
	{
		// trial and error here
		expr_cref underlying = loader.underlying( expr );

		switch( underlying.type() )
		{
			case EInt:
			{
				size_t num = 0;
				loader.convert( underlying, num );
				m_bitset = object_type( num );
			}
			break;

			case EString: case EConcat:
			{
				std::string str;
				loader.convert( underlying, str );
				m_bitset = object_type( str );
			}
			break;

			case EList:
			{
				// complicated so use SetParamBinder
				SetParamBinder<size_t> binder( m_paramNum );
				binder.bind( loader, underlying );
				std::set< size_t > obj = binder.obj();
				for( size_t idx : obj )
				{
					m_bitset.set( idx );
				}
			}
			break;

			case EObject:
			{
				ObjParamBinder< Proxy< std::set<size_t> > > proxy( m_paramNum );
				proxy.bind( loader, underlying );
				std::set<size_t> obj = proxy.obj()->get();

				for( size_t idx : obj )
				{
					m_bitset.set( idx );
				}
			}
			break;

			default:
			{
				// create the exception but don't throw it. "bind" will throw it
				// Note: it can't be a Proxy for bitset because that would go through case EObject
				handleError( std::invalid_argument( "Invalid expression for bitset" ), underlying.value() );
			}
		}
	}

public:

	object_type obj() const
	{
		return m_bitset;
	}
};

template< size_t N >
struct binder_traits< std::bitset< N > >
{
	typedef BitsetParamBinder< std::bitset< N > > binder_type;
};

// NEW: ParameterBinder, replaces ParamBinder
// This IS a real struct (ok, class) and allows for the fact that a proxy can be used
template< typename T >
class ParameterBinder : public ParamBinderBase
{
public:
	typedef binder_traits<T> binder_traits_type;
	typedef typename binder_traits_type::binder_type binder_type;
	typedef typename binder_type::object_type object_type;
	typedef Proxy<object_type> proxy_type;
	typedef ObjParamBinder< proxy_type > proxy_binder_type;

private:
	binder_type binder;
	proxy_binder_type proxy_binder;
	bool usingProxy;
public:
	explicit ParameterBinder( size_t paramNum = 0, ParamBinderBase** assignMe = NULL )
			: ParamBinderBase( paramNum, assignMe ), usingProxy( false )
	{
	}

protected:

	void doBind( const ObjectLoader & loader, expr_cref expr )
	{
		try
		{
			// first try a regular bind. This is not really an ideal way to do it
			binder.bind( loader, expr );
		}
		catch( TypeError const& err ) // this means we have an issue with the type of our parameter
		{
			usingProxy = true;
			try
			{
				proxy_binder.bind( loader, expr );
			}
			catch( TypeError const & )
			{
				// we want to convert it back to invalid_argument
				// so the parent object does not get TypeError
				// We also throw the original error

				throw std::invalid_argument( err.what() );
			}
		}
	}

public:
	// get the object from whichever object we used
	object_type obj() const
	{
		return usingProxy ? proxy_binder.obj()->get() : binder.obj();
	}
};

}

#endif
