#pragma once

#include <IOC/Builder.h>

namespace IOC { namespace detail {

struct LibraryInfo
{
	std::string m_path;
	Library * m_library;

};

class ClassInfo
{
	std::string m_name;
	BuilderFactory const * m_factory;
public:
	ClassInfo( std::string const& name, BuilderFactory const * factory );
	// will create a different builder every time it is called
	str_cref name() const
	{
		return m_name;
	}

	BuilderPtr createBuilder( str_cref name, expr_cref expr ) const;
};

typedef spns::shared_ptr< ClassInfo > ClassInfoPtr;

// Think about how we are going to name "temporary" objects
// When we build we do want to know what the master object was we were building
// and which parameter.

class ObjectInfo
{
private:
	std::string m_name;
	expr_cref m_expr;
	ClassInfo const & m_classInfo;
	BuilderPtr m_builder;

public:

	explicit ObjectInfo( str_cref name, expr_cref expr, ClassInfo const& classInfo,
		BuilderPtr builder );

	std::string name() const
	{
		return m_name;
	}

	str_cref className() const
	{
		return m_classInfo.name();
	}

	BuilderPtr getBuilder() const
	{
		return m_builder;
	}

	expr_cref expr() const
	{
		return m_expr;
	}
};

typedef spns::shared_ptr< ObjectInfo > ObjectInfoPtr;

}

}
