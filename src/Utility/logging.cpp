/*
 * logging.cpp
 *
 *  Created on: 30 Dec 2013
 *      Author: neil
 */

#include <Utility/logging.h>
#include <Utility/mapLookup.h>
#include <map>
#include <vector>
#include <iostream>
#include <thread>
#include <mutex>

namespace {

class LogManager
{
private:
	std::map< int, std::vector<Utility::LoggerPtr> > m_loggers; // one logger can subscribe to more than one subject
	mutable std::mutex m_mutex;
public:

	void subscribe( int subject, Utility::LoggerPtr logger )
	{
		// if your logger subscribes twice it will end up logging everything twice
		// there is no real way to check the uniqueness of loggers
		std::cout << "Subscribing a logger with subject " << subject << std::endl;

		m_loggers[subject].push_back( logger );
	}

	bool hasSubscribers( int subject ) const
	{
		return m_loggers.count( subject ) > 0;
	}

	void log( int subject, std::string const& message ) const
	{
		// TODO: Push to a queue. Have thread listening to the queue that does below
		// One queue will suffice. The thread that pulls will "flush" it in one go.
		// The mutex will only be locked to manage the queue and will be unlocked whilst
		// message are actually being output.

		// We will probably start the thread at the point that the first subscriber is added,
		// and terminate it in our destructor by sending a condition_variable::signal_all()
		// a terminating condition to be checked, and then waiting for all these threads.

		std::lock_guard < std::mutex > mlock( m_mutex );
		std::vector< Utility::LoggerPtr > loggers = Utility::mapGet( m_loggers, subject );
		for( auto logger : loggers )
		{
			logger->logMessage( subject, message );
		}
	}
};

LogManager theLogManager;

}

namespace Utility {

Logger::Logger()
{
}

Logger::~Logger()
{
}

void Logger::subscribeSubject( int subject )
{
	theLogManager.subscribe( subject, shared_from_this() );
}

LoggerSubscriber::LoggerSubscriber( std::vector< LoggerPtr > const& loggers )
{
	std::cout << "LoggerSubscriber constructor with " << loggers.size() << " loggers " << std::endl;
	for( auto logger : loggers )
	{
		logger->subscribe();
	}
}

bool logHasSubscribers( int subject )
{
	return theLogManager.hasSubscribers( subject );
}

void logMessage( int subject, std::string const& message )
{
	theLogManager.log( subject, message );
}

}

