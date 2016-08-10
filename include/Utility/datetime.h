/*
 * datetime.h
 *
 *  Created on: 11 Nov 2013
 *      Author: neil
 */

#ifndef DATETIME_H_
#define DATETIME_H_

#include "api.h"
#include <boost/date_time.hpp>

namespace Utility
{

// Usage:
// Create one of these for each date/time format you want to print
// Then use it to print multiple date/times efficiently.
// If we have to read in date-times we will write a separate class
// You must also link against boost_system if you use this

class DateTimeFormatter
{
public:
    typedef boost::posix_time::ptime time_type;
private:
    typedef boost::date_time::time_facet<time_type, char> facet_type;
    facet_type * m_facet;
    std::locale m_locale;

public:
    explicit DateTimeFormatter( const char * formatStr ) :
        m_facet( new facet_type( formatStr ) ),
        m_locale( std::locale::classic(), m_facet )
    {
    }

    std::ostream & printTime( std::ostream& os, time_type const& time ) const
    {
        os.imbue( m_locale );
        os << time;
        return os;
    }

    std::string printTime( time_type const& time ) const
    {
        std::ostringstream oss;
        printTime( oss, time );
        return oss.str();
    }
};

// TODO: Firstly this should work. Secondly ideally move the implementation into a library
UTILITY_API std::ostream & timestampMS( std::ostream & os );

}

#endif /* DATETIME_H_ */
