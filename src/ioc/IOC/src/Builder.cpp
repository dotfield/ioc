// contains all required virtual destructor implementations
#include "stdafx.h"
#include <IOC/detail/BuilderParamBinders.h>

namespace IOC 
{
	Builder::~Builder()
	{
	}

    TypeError::TypeError( std::string const& err )  : std::invalid_argument( err )
    {
    }

	DLObject::~DLObject()
	{
	}

	BuilderFactory::~BuilderFactory()
	{
	}

	std::string getType( const Runnable * )
	{
		return "IOC::Runnable";
	}

	ObjectLoader::~ObjectLoader()
	{
	}

	void Builder::raiseInvalidParameterCountError( size_t expected, size_t found ) const
	{
		std::ostringstream oss;
		oss << alias() << " type " << type() << " expects " << expected 
				<< " parameters but has " << found;

		throw std::invalid_argument( oss.str() );
	}

	void Builder::circularCheck() const
    {
        if( m_creating )
        {
            raiseCircularReferenceError();
        }
        m_creating = true;
    }

	void Builder::raiseCircularReferenceError() const
	{
		std::ostringstream oss;
		oss << "Circular reference detected";
		if( !alias().empty() ) // should not normally happen as it's impossible
			// to create a circular reference for an anonymous object
		{
			oss << " for " << alias() << '\n';
		}
		throw std::invalid_argument( oss.str() );
	}

	ParamBinderBase::~ParamBinderBase()
	{
	}

	void ParamBinderBase::bind( const ObjectLoader & loader, expr_cref expr )
	{
		try
		{
			doBind( loader, expr );
		}
		catch( TypeError const& err )
		{
		    throw; // do not handle this here
		}
		catch( std::exception const& err )
		{
			handleError( err, expr.value() );
		}
	}

	void ParamBinderBase::handleError( std::exception const& err, str_cref variable ) const
	{
		std::ostringstream oss;
		oss << err.what();
		if( m_paramNum > 0 )
		{
			if( !variable.empty() )
			{
				oss << " evaluating " << variable;
			}
			oss << ", which is parameter " << m_paramNum << '\n';
		}
		throw std::invalid_argument( oss.str() );
	}

}
