
#include "stdafx.h"
#include "StructuredConfigData.h"
#include <IOC/Builder.h>

namespace IOC { namespace detail {


ClassInfo::ClassInfo( std::string const& name, BuilderFactory const * factory ) :
  m_name( name ), m_factory( factory )
{
}
	// will create a different builder every time it is called
BuilderPtr ClassInfo::createBuilder( str_cref name, expr_cref expr ) const
{
	BuilderPtr builder( m_factory->create( name, expr ) );
	return builder;
}


// Think about how we are going to name "temporary" objects
// When we build we do want to know what the master object was we were building
// and which parameter.


ObjectInfo::ObjectInfo( str_cref name, expr_cref expr, ClassInfo const& classInfo, BuilderPtr builder ) :
	m_name( name ), m_expr( expr ), m_classInfo( classInfo ), m_builder( builder )
{
}


} }

	



	
