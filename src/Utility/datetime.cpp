/*
 * datetime.cpp
 *
 *  Created on: 25 Nov 2013
 *      Author: neil
 */

#include <Utility/datetime.h>

namespace Utility {

std::ostream & timestampMS( std::ostream & os )
{
    static Utility::DateTimeFormatter dtf( "%H:%M:%s" );
    boost::posix_time::ptime curTime = boost::date_time::microsec_clock<boost::posix_time::ptime>::local_time();
    dtf.printTime( os, curTime );
    return os;
}

}


