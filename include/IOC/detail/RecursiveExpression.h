#pragma once

#ifndef IOC_RECURSIVE_EXPRESSION_H_
#define IOC_RECURSIVE_EXPRESSION_H_

#include "../ioc_api.h"
#include "../ioc_enum.h"
#include <vector>
#include <string>
#include <stdexcept>

namespace IOC {

#if defined _WIN32 || defined _WIN64
// suppress these stupid warnings in Windows
#pragma warning ( disable : 4251 )
#pragma warning ( disable : 4231 )
#pragma warning ( disable : 4275 )
#endif

/* 
 This class is an integral part of the IOC system, although it will not be used
  "directly" by programmers.
*/

class IOC_API RecursiveExpression 
{
public:
	
private:
	friend class RecursiveExpressionParser;
	typedef ExpressionType Type;
	std::string m_value;
	std::vector< RecursiveExpressionPtr > m_params;

	Type m_type;
	RecursiveExpression * m_parent;

	void setError( std::string const& value );

	void addToParent();

	RecursiveExpression( RecursiveExpression * parent, std::string const& value, Type type );

public:
	// types that we parsed in
	// Function for this purpose means an expression that has parentheses and members
	// Error means it failed parsing
	


	// if created with non-NULL parent there is no need to manage its lifetime as its parent will
	// If created with NULL parent you must manage its lifetime
	static RecursiveExpression * create( RecursiveExpression * parent, std::string const& value, Type type );
	static RecursiveExpression * create();

	static RecursiveExpressionPtr parse( std::string const& line, std::string const& currentDir );
	
	std::string const& value() const
	{
		return m_value;
	}

	Type type() const
	{
		return m_type;
	}

	std::vector< RecursiveExpressionPtr > const& params() const
	{
		return m_params;
	}

	// not bounds checked
	RecursiveExpressionPtr param( size_t idx ) const
	{
		return m_params[idx];
	}
};

}

//extern template class IOC_API std::vector< IOC::RecursiveExpressionPtr >;

#endif
