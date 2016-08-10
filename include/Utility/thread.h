/*
 * thread.h
 *
 *  Created on: 24 Feb 2014
 *      Author: neil
 */

#ifndef UTILITY_THREAD_H_
#define UTILITY_THREAD_H_

#include <condition_variable>
#include <mutex>
#include <thread>

namespace Utility {

class Synchronise
{
	std::condition_variable condvar;
	std::mutex mutex;
	bool condition = false;

public:

	void wait()
	{
		std::unique_lock< std::mutex > lock( mutex );
		if( !condition )
			condvar.wait( lock );

		condition = false;
	}

	void signal()
	{
		std::unique_lock< std::mutex > lock( mutex );
		condition = true;
		condvar.notify_one();
	}
};

}


#endif /* THREAD_H_ */
