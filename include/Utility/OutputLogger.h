/*
 * OutputLogger.h
 *
 *  Created on: 30 Dec 2013
 *      Author: neil
 */

#ifndef OUTPUTLOGGER_H_
#define OUTPUTLOGGER_H_

// OutputLogger implements Logger (logging.h) by writing its messages to the Output.
#include "api.h"
#include "Output.h"
#include "logging.h"
#include <bitset>

namespace Utility
{

class UTILITY_API OutputLogger : public Logger
{
private:
	OutputPtr m_output;
	std::bitset< 8 > m_subjects;

public:
	OutputLogger( OutputPtr output, std::bitset< 8 > subjects );

	void subscribe();
	void logMessage( int subject, std::string const& message );
};

}

#endif /* OUTPUTLOGGER_H_ */
