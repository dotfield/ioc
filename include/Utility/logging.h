/*
 * logging.h
 *
 *  Created on: 30 Dec 2013
 *      Author: neil
 */

#ifndef LOGGING_H_
#define LOGGING_H_

#include "api.h"
#include <memory>
#include <vector>
#include "Message.h"
// This implements "shout" logging as I call it.
// What that means essentially is that when you want to log something you just "shout out" your
// message and it gets picked up by any implementation that wants to log it.

// The implementation might use log4cpp or just file it or whatever.
// The library of code creating the log message does not need to have any dependency on what happens to the message.
// Normally though we give each message a "subject" to which loggers subscribe, and before logging, the sender
// can check if anything is subscribed.

// Perfect implementation is threadsafe by sending the log messages to a queue, and logging them is done by one or more
// logger thread.

// Using this system does require presence of a singleton (a global) that manages the loggers and the messages.
// This singleton is created at load time and loggers subscribe to it at load time too. This happens within a single threaded
// environment. New subjects are only created by loggers.

namespace Utility {

// Refine this, but we want to be able to use it soon enough to debug our current libraries...

// this does not belong here either because it's an implementation detail. User interface is all free-functions


class UTILITY_API Logger : public std::enable_shared_from_this< Logger > // enables us to shared_ptr ourselves when we subscribe
{
public:
	static const int EDebug = 0;
	static const int EInfo = 1;
	static const int EWarn = 2;
	static const int EError = 3;

	Logger();

	virtual ~Logger();

	virtual void logMessage( int subject, std::string const& message ) = 0;

	virtual void subscribe() = 0;

protected:
	void subscribeSubject( int subject );
};


typedef std::shared_ptr< Logger > LoggerPtr;

class UTILITY_API LoggerSubscriber
{
public:
	explicit LoggerSubscriber( std::vector< LoggerPtr > const& loggers );
};

// free functions

UTILITY_API bool logHasSubscribers( int subject );
UTILITY_API void logMessage( int subject, std::string const& message );

class UTILITY_API LogMessage
{
	int m_subject;

public:
	explicit LogMessage( int subject ) : m_subject( subject )
	{}

	int subject() const { return m_subject; }
};

inline Message const& operator<<( Message const& msg, LogMessage const& logMsg )
{
	logMessage( logMsg.subject(), msg.str() );
	return msg;
}
/*
template< int SUBJECT > Message const& LogMessage( Message const& msg )
{
	logMessage( SUBJECT, msg.str() );
	return msg;
}
*/


}

inline const char * getType( Utility::Logger const * )
{
	return "Logger";
}

inline const char * getType( Utility::LoggerSubscriber const * )
{
	return "LoggerSubscriber";
}

#define DEBUGLOG_TEXT( txt ) \
	if( !Utility::logHasSubscribers( Utility::Logger::EDebug ) ) ; \
	else Utility::logMessage( Utility::Logger::EDebug, txt )

#define DEBUGLOG_STREAM( strmtxt ) \
	if( !Utility::logHasSubscribers( Utility::Logger::EDebug ) ) ; \
	else Utility::Message() << strmtxt << Utility::LogMessage(Utility::Logger::EDebug)

#define LOG_TEXT( subject, txt ) \
		if( !Utility::logHasSubscribers( subject ) ) ; \
		else Utility::logMessage( subject, txt )

#define LOG_STREAM( subject, strmtxt ) \
	if( !Utility::logHasSubscribers( subject ) ) ; \
	else Utility::Message() << strmtxt << Utility::LogMessage(subject)

#endif /* LOGGING_H_ */
