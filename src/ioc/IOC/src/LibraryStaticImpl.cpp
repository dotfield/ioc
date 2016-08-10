#include "stdafx.h"
#include <IOC/LibraryStaticImpl.h>
#include "Utility/mapLookup.h"
#include <cassert>
#include <stdexcept>
// there is no need to add DLObject.h

// instantiate its instance here.
template class std::map< std::string, const IOC::DLObject * >;

namespace IOC 
{

// this function is added from code and therefore it is code-incorrect if you add 
// two different objects with the same name

void LibraryStaticImpl::addSymbol( str_cref name, const DLObject * obj )
{
	if( ! m_objects.insert( std::make_pair( name, obj ) ).second )
	{
	    std::ostringstream oss;
	    oss << "Symbol " << name << " multiply defined in library " << getAlias();
	    throw std::invalid_argument( oss.str() );
	}
}

const DLObject * LibraryStaticImpl::getSymbol( str_cref sym, bool throwIfNotFound ) const
{
	const DLObject * symbol = NULL;
	if( Utility::mapLookup( m_objects, sym, symbol ) || !throwIfNotFound )
	{
		return symbol;
	}
	else 
	{
		std::ostringstream oss;
		oss << "Symbol " << sym << " not found in library " << getAlias();
		throw std::invalid_argument( oss.str() );
	}
}

}
