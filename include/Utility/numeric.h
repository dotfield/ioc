/*
 * numeric.h
 *
 *  Created on: 6 Jan 2014
 *      Author: neil
 */

#ifndef UTILITY_NUMERIC_H_
#define UTILITY_NUMERIC_H_

#include "api.h"
#include <cmath> // for fabs
#include <functional>
#include <vector>
#include <array>
#include <utility>
#include <map>

namespace Utility {

// These are predicates to compare doubles, where you cannot expect accuracy
// The predicates return true if they two values are "equal enough"

// DoubleDiffCompare uses absolute tolerance
class DoubleDiffCompare : public std::binary_function< double, double, bool >
{
	double m_tol; // succeeds if abs( x - y ) <= tolerance

public:
	DoubleDiffCompare( double tol ) : m_tol( tol )
	{
	}

	bool operator()( double lhs, double rhs ) const
	{
		return std::fabs( lhs - rhs ) <= m_tol;
	}
};

// DoubleRatioCompare uses relative tolerance.
// Two values are considered equal if either they are both 0 or if the absolute value of
// their ratio - 1 is less than the tolerance

class DoubleRatioCompare : public std::binary_function< double, double, bool >
{
	double m_tol; // succeeds if lhs == rhs == 0 or abs( (lhs/rhs) - 1 ) <= m_tol

public:
	DoubleRatioCompare( double tol ) : m_tol( tol )
	{
	}

	bool operator()( double lhs, double rhs ) const
	{
		if( rhs == 0.0 )
			return lhs == 0.0;
		else
			return( fabs( lhs / rhs - 1.0 ) <= m_tol );
	}
};

double UTILITY_API brentMethodSolve
(
	std::function<double(double)> func, // a function you want to produce targetF
	double lowerX, // lower bound such that func(lowerX) < targetF
	double upperX, // upper bound such that func(upperX) > targetF. Could be < lowerX if func is decreasing
	double lowerF, // known value of func(lowerX)
	double upperF, // known value of func(upperX)
	double targetF, // value we are trying to achieve
	double tol // tolerance for DoubleRatioCompare
);

class UTILITY_API CubicSpline // this, once constructed can be used as a function.
	// It is not intended to be an iterative solver as Brent is used for that.
	// With this you put in several known values once then interpolate
{
public:
	typedef std::pair< double, double > Point;
	typedef std::array<double, 4> Coefficients;
	typedef std::vector< Point > vector_type;

private:
	vector_type m_points;
	std::vector< Coefficients > m_coeffs;

	void construct_coeffs();
	double eval( double x, Coefficients const& coeffs ) const;

public:

	CubicSpline() = default;
	explicit CubicSpline( std::vector< Point > const& points );
	explicit CubicSpline( std::map< double, double > const& points );

	double operator()( double ) const;

	// gets the coefficients and the lower bound for the input value
	std::pair< double, Coefficients > range( double ) const;

	void swap( CubicSpline & other )
	{
		m_points.swap( other.m_points );
		m_coeffs.swap( other.m_coeffs );
	}
};

// linear regression estimates the points to a straight line.
// You can get the Rsqr (is that the same as chi-squared?) and Yres to see
// the discrepencies to see if it is a good fit.

class UTILITY_API LinearRegression
{
public:

	typedef std::pair< double, double > Point;

private:
	double m_slope = 0.0;
	double m_intercept = 0.0;
	double m_rsq = 0.0;

public:

	double slope() const
	{
		return m_slope;
	}

	double intercept() const
	{
		return m_intercept;
	}

	double rsq() const // coefficient of determination. Closer to 1 means good fit.
	{
		return m_rsq;
	}

	double operator()( double d ) const
	{
		return d * m_slope + m_intercept;
	}

	double solve( double d ) const // find an x that evaluates to y
	{
		return (d - m_intercept) / m_slope;
	}

	explicit LinearRegression( std::vector< Point > const& points )
	{
		regress( points );
	}

	explicit LinearRegression ( std::map< double, double > const& points );

	LinearRegression()  = default;

	void regress( std::vector< Point > const& points );


};

}

#endif /* NUMERIC_H_ */
