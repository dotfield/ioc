/*
 * OutputLogger.cpp
 *
 *  Created on: 30 Dec 2013
 *      Author: neil
 */

#include <Utility/OutputLogger.h>
#include <Utility/datetime.h>

namespace Utility {

OutputLogger::OutputLogger( OutputPtr output, std::bitset< 8 > subjects )
	 : m_output( output ),
	   m_subjects( subjects )
{
}

void OutputLogger::subscribe()
{
	for( int subject = 0; subject < 8; ++subject )
	{
		if( m_subjects.test( subject ) )
		{
			subscribeSubject( subject );
		}
	}
};

const char * const subjects[] = { "Debug", "Info", "Warning", "Error", "subj4,", "subj5", "subj6", "subj7" };

void OutputLogger::logMessage( int subject, std::string const& message )
{
	// the subject here is passed for info, which can be put into the log.
	// we do not need to check if we are subscribed to log it, unless we are we wouldn't have got it.

	// In our case we are not going to (at this stage) put it into the log

	timestampMS( m_output->os() ) << ": LOG-" << subjects[subject] << ": " << message << '\n';
	m_output->flush();
}

}

