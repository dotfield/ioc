#include "stdafx.h"

#include <IOC/Builder.h>
#include <IOC/detail/ObjectLoader.h>
#include "StructuredConfigData.h"
#include <IOC/Libraries.h>
#include "Utility/mapLookup.h"
#include <IOC/detail/RecursiveExpression.h>
#include <IOC/Runnable.h>
#include <IOC/ioc_api.h>
#include <boost/lexical_cast.hpp>
// ConfigObjectLoader uses Config to provide the values.
// Config itself is just in essence a map<string,string>

const char * const typestr[] = { "String", "Bool", "Int", "Real", "Void", "Variable",
	"List", "Map", "Concat", "Library", "Class", "Pair", "Object", "CurrentDir", "Error" };


namespace IOC { namespace detail {

	// This function implemented in LoadIOCConfig.cpp
	// In some ways this decouples the actual format of the config file with the ConfigObjectLoader itself.

void loadIOCConfigInto( str_cref filePath, std::map< std::string, RecursiveExpressionPtr > & config );

// we deal with expressions, not strings
// We therefore need to redesign this.

class ConfigObjectLoader : public ObjectLoader
{
private:
	// this is a map created by loading the config
	std::map< std::string, RecursiveExpressionPtr > m_config;

	// We lazy-load everything so all these must be mutable
	// Note this is all done single-threaded so there is no danger or need for boost::once etc.
	
	mutable std::map< std::string, ClassInfoPtr > m_classes;
	mutable std::map< std::string, ObjectInfoPtr > m_objects;

	// we could also map all the others for efficiency if we want to be efficient
	// especially the more complex ones like maps. It is not important to be efficient though.
	// We store the two above because these should be unique, i.e. we want ONE ClassInfo per
	// definition and one class instance.
	LibraryTable & theLibraryTable;

public:
	ConfigObjectLoader( LibraryTable & libraries, str_cref filePath )
		: theLibraryTable( libraries ) 
	{
		loadIOCConfigInto( filePath, m_config );
	}

	expr_cref lookup( str_cref name ) const;
	const RecursiveExpression * lookupNoThrow( str_cref name ) const;
	// This is always returned as a vector of strings. If they are objects, you then
	// need to resolve the objects. If you want numbers they must be lexically cast.

protected:
	// these may either lexically cast or return a variable that has the value

	// toString does not simply return the string that is input.
	// Input is either a quoted string in which case it removes the quotes, or it is
	// an alias to a constant in which case it returns the constant.
	// It will also resolve embedded values

	virtual double toDouble( expr_cref value ) const;
	virtual int toInt( expr_cref value ) const;
	virtual size_t toUInt( expr_cref value ) const;
	virtual bool toBool( expr_cref value ) const;
	virtual std::string toString( expr_cref value ) const;
	virtual std::wstring toWString( expr_cref value ) const;

public:
	// to convert a vector we have to go through this...
	virtual void toVector( expr_cref value, std::vector< RecursiveExpressionPtr > & res ) const;
	// after which w convert each expression returned

	// to convert to a map we have to go through this...
	virtual void toMap( expr_cref value, 
		std::vector< std::pair< RecursiveExpressionPtr, RecursiveExpressionPtr > > & res ) const;

	virtual std::string toEnum( expr_cref value ) const;
	virtual BuilderPtr getBuilder( expr_cref value ) const;

	// this gets the underlying expression. If lastvar is non-null, it gives the string
	// that defined this expression. This is needed for ObjectInfo where one names the
	// object and others are aliases. We map against the one that actually names it.

	virtual expr_cref underlying( expr_cref start, std::string * lastvar = NULL, bool throwIfNotFound=true ) const;
private:
	ObjectInfoPtr getObjectInfo( expr_cref expr, str_cref name ) const;
	ClassInfoPtr getClassInfo( str_cref name ) const;
	const Library & getLibrary( str_cref name ) const;

};


RecursiveExpression const* ConfigObjectLoader::lookupNoThrow( str_cref name ) const
{
	RecursiveExpressionPtr expr;
	if( Utility::mapLookup( m_config, name, expr ) )
	{
		return expr.get();
	}
	else
	{
		return NULL;
	}
}

expr_cref ConfigObjectLoader::lookup( str_cref name ) const
{
	const RecursiveExpression * expr = lookupNoThrow( name );
	if( expr )
	{
		return *expr;
	}
	else
	{
		std::ostringstream oss;
		oss << "Undefined value or unexpected enum " << name;
		throw std::invalid_argument( oss.str() );
	}
}

expr_cref ConfigObjectLoader::underlying( expr_cref start, std::string * lastvar, bool throwIfNotFound ) const
{
	const RecursiveExpression * pexpr = &start;

	std::set< std::string > seen;
	// this will throw if any are not found
	while( pexpr->type() == EVariable )
	{
		std::string varname = pexpr->value();
		if( !seen.insert( varname ).second )
		{
			std::ostringstream oss;
			oss << "Circular reference resolving " << varname;
			throw std::invalid_argument( oss.str() );
		}

		if( lastvar )
			*lastvar = varname;

		if( throwIfNotFound )
		{
			pexpr = &lookup(varname);
		}
		else
		{
			const RecursiveExpression * next = lookupNoThrow(varname);
			if( next )
			{
				pexpr = next;
			}
			else
			{
				return *pexpr;
			}
		}
	}
	return *pexpr;
}

int ConfigObjectLoader::toInt( expr_cref value ) const
{
	expr_cref expr = underlying( value );
	
	// If we want an int it has to be that. Not a double.
	if( expr.type() != EInt )
	{
		std::ostringstream oss;
		oss << "expected int, got " << expr.value() << " interpreted as " << typestr[expr.type()];
		throw TypeError( oss.str() );
	}
	return boost::lexical_cast<int>( expr.value() );
}

double ConfigObjectLoader::toDouble( expr_cref value ) const
{
	expr_cref expr = underlying( value );
	
	// we want a double. If we get an int, that will suffice. All ints are also doubles.
	if( expr.type() != EReal && expr.type() != EInt )
	{
		std::ostringstream oss;
		oss << "expected a number, got " << expr.value() << " interpreted as " << typestr[expr.type()];
		throw TypeError( oss.str() );
	}
	return boost::lexical_cast<double>( expr.value() );
}

size_t ConfigObjectLoader::toUInt( expr_cref value ) const
{
	expr_cref expr = underlying( value );
	
	if( expr.type() != EInt )
	{
		std::ostringstream oss;
		oss << "expected unsigned int, got " << expr.value() << " interpreted as " << typestr[expr.type()];
		throw TypeError( oss.str() );
	}
	try
	{
		return boost::lexical_cast<size_t>( expr.value() );
	}
	catch( std::exception const& ) // in case it's a negative number in which case I assume it won't cast
	{
		// I want to throw my own exception.
		std::ostringstream oss;
		oss << "Expected unsigned int, got " << expr.value() << " Which did not convert (is it negative?)";
		throw std::invalid_argument( oss.str() );
	}
}


bool ConfigObjectLoader::toBool( expr_cref value ) const
{
	expr_cref expr = underlying( value );
	if( expr.type() != EBool )
	{
		std::ostringstream oss;
		oss << "expected boolean, got " << expr.value() << " interpreted as " << typestr[expr.type()] << 
			" (note, numbers do not convert to booleans)";
		throw TypeError( oss.str() );
	}
	
	return ( expr.value() == "true" ); // there are stored as "true" and "false". Anything else would not be interpreted as boolean
}

// For string it is more complex as we have to handle Concat expressions too
std::string ConfigObjectLoader::toString( expr_cref value ) const
{
	expr_cref expr = underlying( value );
	if( expr.type() == EString )
	{
		return expr.value();
	}
	else if( expr.type() == EConcat ) 
	{
		std::vector< RecursiveExpressionPtr > const& params = expr.params();
		std::string res;
		for( RecursiveExpressionPtr pexp : params )
		{
			res.append( toString( *pexp ) );
		}
		return res;
	}
	else
	{
		std::ostringstream oss;
		oss << "Could not interpret " << expr.value() << " as a string: interpreted as " << typestr[expr.type()];
		throw TypeError( oss.str() );
	}
}

std::wstring ConfigObjectLoader::toWString( expr_cref value ) const
{
	std::string utf8str( toString( value ) );

	// not really a proper conversion, should really use locales
	return std::wstring( utf8str.begin(), utf8str.end() );
}


std::string ConfigObjectLoader::toEnum( IOC::expr_cref value ) const
{
	std::string last;
	expr_cref expr = underlying( value, &last, false );
	if( expr.type() != EVariable )
	{
		std::ostringstream oss;
		oss << last << " has been defined to " << expr.value() << " of type " <<
			typestr[expr.type()] << " expecting an enumeration";
		throw TypeError( oss.str() );
	}

	return last; // which should be the same as expr.value()
}

void ConfigObjectLoader::toVector( expr_cref value, std::vector< RecursiveExpressionPtr > & res ) const
{
	// this is actually fairly straightforward
	// once again traverse down through any variables
	expr_cref expr = underlying( value );

	// it must be expressed as List.
	if( expr.type() != EList )
	{
		std::ostringstream oss;
		oss << "expected a list, got " << expr.value() << " interpreted as " << typestr[expr.type()];
		throw TypeError( oss.str() );
	}

	// now just populate it
	res = expr.params();
}

struct PairConverter
{
	ConfigObjectLoader const * m_loader;

	PairConverter( ConfigObjectLoader const * loader ) : m_loader( loader )
	{
	}

	std::pair< RecursiveExpressionPtr, RecursiveExpressionPtr > operator()( RecursiveExpressionPtr value ) const
	{
		expr_cref expr = m_loader->underlying( *value );

		if( expr.type() != EPair )
		{
			std::ostringstream oss;
			oss << "Items in a Map must be a Pair: Got " << expr.value() << " type " << typestr[expr.type()];

			// not TypeError here, you cannot create a proxy for a pair
			throw std::invalid_argument( oss.str() );
		}

		return std::make_pair( expr.param(0), expr.param(1) );
	}
};
		// we know a Pair always has 2 parameters and that has already been checked, there is no need to check that.


// To return an actual std::map at this stage would be far too complex. We returns a vector of pairs
// which will then be 
void ConfigObjectLoader::toMap( expr_cref value, 
		std::vector< std::pair< RecursiveExpressionPtr, RecursiveExpressionPtr > > & res ) const
{
	expr_cref expr = underlying( value );
	
	if( expr.type() != EMap )
	{
		std::ostringstream oss;
		oss << "expected a map, got " << expr.value() << " interpreted as " << typestr[expr.type()]
		<< " (note: a List of Pair items is not a map)";

		throw TypeError( oss.str() );
	}

	std::vector<RecursiveExpressionPtr> const& params = expr.params();

	res.resize( params.size() );
	std::transform( params.begin(), params.end(), res.begin(), PairConverter( this ) );
}

const Library & ConfigObjectLoader::getLibrary( str_cref name ) const
{
	// first check if it is already loaded

	// this next line can't throw but several other calls here could
	const Library * pLib = theLibraryTable.getLibraryNoThrow( name );
	if( !pLib )
	{
		expr_cref expr = lookup( name );
		if( expr.type() != ELibrary ) 
		{
			std::ostringstream oss;
			oss << name << " is not a library";
			throw std::invalid_argument( oss.str() );
		}
		// a library will always have just 1 parameter and there is no need to check it
		// It can throw if the parameter cannot evaluate to a string. It may be a Concat and often will be
		// if you wish to specify a root path in a variable then make all the others come off that
		std::string path = toString( *(expr.param(0)) );
		
		// this can throw if what is in "path" cannot be loaded through LoadLibrary
		pLib = &( theLibraryTable.addLibrary( name, path ) );
	}
	return *pLib;
}

// checks if the class info has been loaded
ClassInfoPtr ConfigObjectLoader::getClassInfo( str_cref name ) const
{
	ClassInfoPtr cls;
	if( Utility::mapLookup( m_classes, name, cls ) )
	{
		return cls;
	}
	else
	{
		expr_cref expr = lookup( name );
		
		// you cannot realias a class so there is no need to recurse this. It must be of type Class
		// i.e. you can't do
/*
   Foo = Class( SomeLib, SomeSymbol );
   Bar = Foo;
   Obj = Bar( whatever );
*/

		if( expr.type() != EClass )
		{
			std::ostringstream oss;
			oss << name << " is not a class";
			throw std::invalid_argument( oss.str() );
		}

		// A class will always have 2 parameters, a library and a symbol
		// The first must be the name of a library
		RecursiveExpressionPtr libExp = expr.param(0);
		if( libExp->type() != EVariable )
		{
			std::ostringstream oss;
			oss << "First parameter of class " << name << " must be a library alias";
			throw std::invalid_argument( oss.str() );
		}

		// resolve the second one to a string
		std::string symbol;
		try
		{
			// rethrow with more context if it fails
			symbol = toString( *expr.param(1) );
		}
		catch( std::invalid_argument const& err )
		{
			std::ostringstream oss;
			oss << err.what() << "\n\t:in 2nd parameter of class definition " << name;
			throw oss.str();
		}

		const DLObject * sym = NULL;
		std::string libName = libExp->value();
		try
		{
			// try loading the symbol. If that fails, give error context.
			const Library & lib = getLibrary( libName );
			sym = lib.getSymbol( symbol, true ); // we do want to throw if it can't load

		}
		catch( std::invalid_argument const& err )
		{
			std::ostringstream oss;
			oss << err.what() << "\n\t whilst loading symbol " 
				<< symbol << " from library " << libName << " class " << name;
			throw std::invalid_argument( oss.str() );
		}

			// if we are here, so far so good
		const BuilderFactory * fact = dynamic_cast< const BuilderFactory * >(sym);
		if( !fact )
		{
			std::ostringstream oss;
			oss << "Symbol " << symbol << " in library " << libName << 
				" is not a BuilderFactory, whilst loading class definition " << name;
			throw std::invalid_argument( oss.str() );
		}

		cls.reset( new ClassInfo( name, fact ) );
		m_classes[name] = cls;
	}

	return cls;
}

// objects can be anonymous, they don't have to have an alias
// Objects created embedded in expressions are never shared (even though we use shared_ptr) nor stored
// in any cache here. They do however have an ObjectInfoPtr with a class etc.

ObjectInfoPtr ConfigObjectLoader::getObjectInfo( expr_cref expr, str_cref name ) const
{
	if( !name.empty() )
	{
		ObjectInfoPtr obj;

		// already found.
		if( Utility::mapLookup( m_objects, name, obj ) )
		{
			return obj;
		}
	}

	if( expr.type() == EObject ) // it's an object expression
	{
		std::string className = expr.value();
		ClassInfoPtr classInfo = getClassInfo( className );
		BuilderPtr builder = classInfo->createBuilder( name, expr );
        ObjectInfoPtr obj( new ObjectInfo( name, expr, *classInfo, builder ) );

        // It needs to be in the map before we bind its parameters so a circular reference can be detected
        m_objects[name] = obj;
        builder->bindParams( *this );
		return obj;
	}
	else if( expr.type() == EVariable )
	{
		// this could recurse indefinitely with circular reference..

		// NOTE: if anything throws here, let ParamBinderBase manage the context of the error.

		std::string objName;
		expr_cref expr2 = underlying( expr, &objName );
		return getObjectInfo( expr2, objName );
	}
	else // this isn't a class. It might be a map or list or whatever but isn't an object
	{
		std::ostringstream oss;
		oss << expr.value() << " is not an object";

		// Do not throw TypeError here. If it isn't an object it can't be a proxy either
		throw std::invalid_argument( oss.str() );
	}

}
		
BuilderPtr ConfigObjectLoader::getBuilder( expr_cref value ) const
{
	std::string name;
	if( value.type() == EVariable )
	{
		name = value.value();
	}

	return getObjectInfo( value, name )->getBuilder();
}

}  // close namespace detail

RunnablePtr getRunnable( str_cref filePath, str_cref name )
{
	// any exceptions are passed on
	detail::ConfigObjectLoader theLoader( libraryTableInstance(), filePath );
	RecursiveExpressionPtr expr( RecursiveExpression::create( NULL, name, EVariable ) );
	BuilderPtr builder = theLoader.getBuilder( *expr );
	spns::shared_ptr< BuilderT< Runnable > > runnableBuilder;
	builder_cast( builder, runnableBuilder );
	

	// this next call will actually cause all other objects and values required to get built or resolved

	return runnableBuilder->getObject();
}

ObjectLoaderPtr getObjectLoader( str_cref filePath )
{
    return ObjectLoaderPtr( new detail::ConfigObjectLoader( libraryTableInstance(), filePath ) );
}


} // close namespace IOC
