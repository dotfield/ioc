/*
 * numeric.cpp
 *
 *  Created on: 6 Jan 2014
 *      Author: neil
 */

#include <Utility/numeric.h>
#include <limits>
#include <Utility/logging.h>
#include <algorithm>

namespace Utility
{

// adapted from BrentsMethodSolve on wikipedia
double brentMethodSolve
	(
		std::function<double(double)> func, // a function you want to produce targetF
		double lowerX,
		double upperX,
		double lowerF,
		double upperF,
		double targetF,
		double tol
	)
{
	double a = lowerX;
	double b = upperX;

	double d = std::numeric_limits<double>::infinity();

	double fa = lowerF - targetF;
	double fb = upperF - targetF; // one of fa and fb is positive and the other negative

	double s = 0;
	double fs = 0;

	// fb should be closer to 0 than fa

	if( fabs( fa ) < fabs( fb ) )
	{
		std::swap( a, b );
		std::swap( fa, fb );
	}

	double c = a;
	double fc = fa;

	bool mflag = true;
	DoubleRatioCompare approx( tol );

	while( !approx( a, b ) )
	{
		if ((fa != fc) && (fb != fc))
			// Inverse quadratic interpolation
			s = a * fb * fc / (fa - fb) / (fa - fc)
					+ b * fa * fc / (fb - fa) / (fb - fc)
					+ c * fa * fb / (fc - fa) / (fc - fb);
		else
			// Secant Rule
			s = b - fb * (b - a) / (fb - fa);

		double tmp2 = (3 * a + b) / 4;
		if ((!(((s > tmp2) && (s < b)) || ((s < tmp2) && (s > b))))
				|| (mflag && (fabs(s - b) >= (fabs(b - c) / 2)))
				|| (!mflag && (fabs(s - b) >= (fabs(c - d) / 2))))
		{
			s = (a + b) / 2;
			mflag = true;
		}
		else
		{
			if( ( mflag && approx(b,c) ) || (!mflag && approx(c, d) ) )
			{
				s = (a + b) / 2;
				mflag = true;
			}
			else
				mflag = false;
		}

		fs = func(s);

		// We could log this iteration here
		DEBUGLOG_STREAM( "f(" << s << ")=" << fs );

		if( approx( fs, targetF ) )
			return s;
		else
			fs -= targetF; // to bring it close to 0

		d = c;
		c = b;
		fc = fb;

		if (fa * fs < 0)
		{
			b = s;
			fb = fs;
		}
		else
		{
			a = s;
			fa = fs;
		}

		if (fabs(fa) < fabs(fb))
		{
			std::swap( a, b );
			std::swap( fa, fb );
		}
	}
	return b;  // rather than a because fb is closer to 0 than fa
}

// Source for this formula: http://michonline.com/ryan/csc/m510/splinepresent.html

CubicSpline::CubicSpline( std::vector< Point > const& points )
	: m_points( points )
{
	construct_coeffs();
}

CubicSpline::CubicSpline( std::map< double, double > const& points )
	: m_points( points.begin(), points.end() )
{
	construct_coeffs();
}

// The coefficient for a value P1 + x which is less than P2
// are actually the coefficients for the x, not the P1+x value.

void CubicSpline::construct_coeffs()
{
	typedef std::vector< double > vectord;

	size_t nRanges = m_points.size() - 1;
	m_coeffs.resize( nRanges );

	vectord h( nRanges ); // difference between points
	vectord yDiff( nRanges ); // differences between values
	vectord y( nRanges );
	vectord u( nRanges );
	vectord z( nRanges );
	vectord l( nRanges, 1.0 );
	vectord c( nRanges + 1, 0.0 );

	for( size_t i = 0; i < nRanges; ++i )
	{
		m_coeffs[ i ][ 0 ] = m_points[ i ].second;

		h[i] = m_points[i+1].first - m_points[i].first;
		yDiff[i] = m_points[i+1].second - m_points[i].second;

		if( i > 0 )
		{
			y[i] = 3 * ( yDiff[i] / h[i]  -  yDiff[i-1] / h[i-1] );
		}
	}
	for( size_t i = 1; i < nRanges; ++i )
	{
		l[i] = 2 * ( m_points[i+1].first - m_points[i-1].first) - ( h[i-1] * u[i-1] );
		u[i] = h[i]/l[i];
		z[i] = ( y[i] - h[i-1] * z[i-1] ) / l[i];
	}

	for( int i = nRanges - 1; i >= 0; --i )
	{
		c[i] = z[i] - u[i]* c[i+1];
		m_coeffs[i][2] = c[i];
		m_coeffs[i][1] = yDiff[i]/h[i] - h[i] * ( c[i+1] + 2*c[i] ) / 3.0;
		m_coeffs[i][3] = ( c[i+1] - c[i] ) / (h[i] * 3);
	}
}

std::pair< double, CubicSpline::Coefficients > CubicSpline::range( double d ) const
{
	Point res( d, 0.0 );
	vector_type::const_iterator cit = std::upper_bound( m_points.begin(), m_points.end(), res );

// As upper_bound returns the value next highest than ours, we find the point that is the end of the range,
// then go back one.
	if( cit != m_points.begin() && cit != m_points.end() )
	{
		size_t idx = cit - m_points.begin() - 1;
		--cit;

		return std::make_pair( cit->first, m_coeffs[idx] );
	}
	else
	{
		return std::make_pair( std::numeric_limits<double>::quiet_NaN(), Coefficients() );
	}
}

double CubicSpline::operator() ( double d ) const
{
	auto rangeData = range( d );
	if( std::isfinite( rangeData.first ) )
	{
		return eval( d - rangeData.first, rangeData.second );
	}
	else
	{
		DoubleRatioCompare comp( std::numeric_limits<double>::epsilon() * 2 );
		if( comp( d, m_points.back().first ) )
		{
			return m_points.back().second;
		}
		else
		{
			return std::numeric_limits<double>::quiet_NaN();
		}
	}
}

// x is the difference to the last point. If x is 0 it is thus just coeffs[0]

double CubicSpline::eval( double x, Coefficients const& coeffs ) const
{
	double val = x * coeffs[3] + coeffs[2];
	val *= x;
	val += coeffs[1];
	val *= x;
	val += coeffs[0];
	return val;
}

LinearRegression::LinearRegression ( std::map< double, double > const& points )
{
	std::vector< Point > vpoints( points.begin(), points.end() );
	regress( vpoints );
}

void LinearRegression::regress( std::vector< Point > const& points )
{
	if( points.empty() )
		return;

	size_t size = points.size();

	double sumX = 0.0;
	double sumY = 0.0;
	double sumXY = 0.0;
	double sumXX = 0.0;

	for( Point point : points )
	{
		double x = point.first;
		double y = point.second;

		sumX += x;
		sumY += y;
		sumXY += x*y;
		sumXX += x*x;
	}

	double avgY = sumY / size;
	double avgX = sumX / size;

	m_slope = (size * sumXY - sumX * sumY) / ( size * sumXX - sumX * sumX );
	m_intercept = avgY - (m_slope * avgX);

	double yResSumSq = 0.0;
	double resSumSq = 0.0;
	// calculate the residue (chi squared)
	for( Point point : points )
	{
		double x = point.first;
		double y = point.second;

		// we can now calculate the value we expect and see what our difference is
		double yRes = x * m_slope + m_intercept - y; // expected y - y
		yResSumSq += ( yRes * yRes );

		double res = (y - avgY );
		resSumSq += ( res * res );
	}

	m_rsq = ( resSumSq - yResSumSq ) / resSumSq;
}

}
