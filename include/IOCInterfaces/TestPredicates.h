#pragma once

#ifndef IOC_TEST_PREDICATES_H
#define IOC_TEST_PREDICATES_H

// These are predicates that are generated with macros but you can also create them
// manually or write your own.
#include "Test.h" // for base class Predicate
#include <functional> // for std::equal_to etc.
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <Utility/numeric.h>

namespace IOC { namespace Test {


// most basic predicate. Just asserts that the expression does not evaluate to zero / false / null.
// The macro will catch what the expression is
// The expression is already evaluated on calculation though so the macro should try..catch

template< typename T >
class PredNonZero : public Predicate
{
	std::string m_expr1;
	const char * m_expr2;
	T m_value;

public:
	// expr1 never null, expr2 can be null
	PredNonZero( const char * expr1, const char * expr2, T value )
		: m_expr1( expr1 ), m_expr2( expr2 ), m_value( value )
	{
	}

	bool eval() const
	{
		return !!m_value; // guaranteed to work as safe conversion to bool
	}

	std::string desc() const
	{
		if( m_expr2 )
		{
			return m_expr1 + ": " + m_expr2;
		}
		else
		{
			return m_expr1;
		}
	}
};

template< typename T >
PredNonZero<T> predNonZero( const char * expr1, const char * expr2, T value )
{
	return PredNonZero<T>( expr1, expr2, value );
}


// opposite of the above

template< typename T >
class PredZero : public Predicate
{
	std::string m_expr1;
	const char * m_expr2;
	T m_value;

public:
	// expr1 never null, expr2 can be null
	PredZero( const char * expr1, const char * expr2, T value )
		: m_expr1( expr1 ), m_expr2( expr2 ), m_value( value )
	{
	}

	bool eval() const
	{
		return !m_value; // guaranteed to work as safe conversion to bool
	}

	std::string desc() const
	{
		if( m_expr2 )
		{
			return m_expr1 + ": " + m_expr2;
		}
		else
		{
			return m_expr1;
		}
	}
};

template< typename T >
PredZero<T> predZero( const char * expr1, const char * expr2, T value )
{
	return PredZero<T>( expr1, expr2, value );
}

// A Predicate class for a compare that takes 2 values and compares them with any binary predicate,
// typically one of std::equal_to, not_equal_to, greater, greater_equal, less or less_equal

// notet that T must be printable
template< typename T, typename COMPARE >
class PredCompare : public Predicate
{
	T m_left;
	T m_right;
	COMPARE m_comp; 
	const char * m_lstr;
	const char * m_rstr;
	const char * m_compStr;
	mutable std::string m_desc;

public:
	
	PredCompare( T const& left, T const& right, const char * lstr, const char * rstr, const char * compStr )
		: m_left( left ), m_right(right), m_lstr( lstr ), m_rstr( rstr ), m_compStr( compStr )
	{
	}

	PredCompare( T const& left, T const& right, COMPARE comp, const char * lstr, const char * rstr, const char * compStr )
		: m_left( left ), m_right(right), m_comp(comp), m_lstr( lstr ), m_rstr( rstr ), m_compStr( compStr )
	{
	}

	std::string desc() const
	{
		if( m_desc.empty() )
		{
			std::ostringstream oss;
			oss << '(' << m_lstr << ',' << m_left << ") " << m_compStr << " (" << m_rstr << ',' << m_right << ')';
			m_desc = oss.str();
		}
		return m_desc;
	}

	bool eval() const
	{
		return m_comp( m_left, m_right );
	}
};

// functions to cover the 6 predicates. We need to use 2 template paramter types in case the right one doesn't
// match the left in which case it they won't resolve. Note that a macro alone won't resolve the type of LEFT

template <typename LEFT, typename RIGHT >
PredCompare<LEFT, std::equal_to<LEFT> >
	predEqual( LEFT const& l, RIGHT const& r, const char * lstr, const char * rstr)
{
	return PredCompare<LEFT, std::equal_to<LEFT> >( l, r, lstr, rstr, "==" );
}

template <typename LEFT, typename RIGHT >
PredCompare<LEFT, std::not_equal_to<LEFT> >
	predNotEqual( LEFT const& l, RIGHT const& r, const char * lstr, const char * rstr )
{
	return PredCompare<LEFT, std::not_equal_to<LEFT> >( l, r, lstr, rstr, "!=" );
}

template <typename LEFT, typename RIGHT >
PredCompare<LEFT, std::less<LEFT> >
	predLess( LEFT const& l, RIGHT const& r, const char * lstr, const char * rstr )
{
	return PredCompare<LEFT, std::less<LEFT> >( l, r, lstr, rstr, "<" );
}

template <typename LEFT, typename RIGHT >
PredCompare<LEFT, std::less_equal<LEFT> >
	predLessEqual( LEFT const& l, RIGHT const& r, const char * lstr, const char * rstr )
{
	return PredCompare<LEFT, std::less_equal<LEFT> >( l, r, lstr, rstr, "<=" );
}

template <typename LEFT, typename RIGHT >
PredCompare<LEFT, std::greater<LEFT> >
	predGreater( LEFT const& l, RIGHT const& r, const char * lstr, const char * rstr )
{
	return PredCompare<LEFT, std::greater<LEFT> >( l, r, lstr, rstr, ">" );
}

template <typename LEFT, typename RIGHT >
PredCompare<LEFT, std::greater_equal<LEFT> >
	predGreaterEqual( LEFT const& l, RIGHT const& r, const char * lstr, const char * rstr )
{
	return PredCompare<LEFT, std::greater_equal<LEFT> >( l, r, lstr, rstr, ">=" );
}

inline PredCompare< double, Utility::DoubleDiffCompare >
	predDoubleDiff( double l, double r, const char * lstr, const char * rstr, double tol )
{
	return PredCompare< double, Utility::DoubleDiffCompare >( l, r, Utility::DoubleDiffCompare( tol ), lstr, rstr, "~=" );
}

inline PredCompare< double, Utility::DoubleRatioCompare >
	predDoubleRatio( double l, double r, const char * lstr, const char * rstr, double tol )
{
	return PredCompare< double, Utility::DoubleRatioCompare >( l, r, Utility::DoubleRatioCompare( tol ), lstr, rstr, "~=" );
}

// PredThrows throws an exception on eval(). It is called when you are "wrapping" an call that has
// thrown. This is used in a situation where you do not expect an exception to occur.
// This is used in a macro where you check a call succeeds (does not throw). Any other value associated
// with such a call should then be checked in a separate assertion.
// PredThrows always throws RequireException. This concludes the test case

class PredThrows : public Predicate
{
private:
	std::string m_desc;
	std::string m_err;
public:
	PredThrows( std::string const& desc, std::string const& err ) :
	  m_desc( desc ), m_err( err )
	 {
	 }

	  std::string desc() const
	  {
		  return m_desc;
	  }

	  bool eval() const
	  {
		  throw RequireException( m_err );
	  }
};

class PredSuccess : public Predicate // always returns true and is used when we want to show something
	// that has worked
{
private:
	std::string m_desc;

public:
	explicit PredSuccess( std::string const& desc ) : 
		 m_desc( desc )
	{
	}

	std::string desc() const
	{
		return m_desc;
	}

	bool eval() const
	{
		return true;
	}
};

class PredFail : public Predicate // always returns false and is used when we want to show something
	// that has failed
{
private:
	std::string m_desc;

public:
	explicit PredFail( std::string const& desc ) : 
		 m_desc( desc )
	{
	}

	std::string desc() const
	{
		return m_desc;
	}

	bool eval() const
	{
		return false;
	}
};

} }

#endif
