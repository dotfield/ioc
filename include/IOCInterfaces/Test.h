#pragma once

#ifndef IOC_TEST_H_
#define IOC_TEST_H_

#include "./api.h"
#include "../IOC/iocfwd.h" // for spns and str_cref. But should those be elsewhere?
#include <stdexcept>
#include <atomic>
// Whilst this header may look bulky, in this case you're only going to include it
// when you are writing test cases or a test reporter.

// Option 2: this could become its own project
#if defined _WIN32 || defined __WIN64
#pragma warning ( disable : 4251 )
#endif

namespace IOC 
{
	namespace Test
	{
		// A step to be processed by the reporter.
		// EPass means that a test passed, ELog means we didn't test anything but want to log something
		// (e.g. info about where we are in the test). Although there is a PredSuccess that does this,
		// we might not be logging info for passed tests, but we want to log ones specifically marked for ELog.
		// EFail means a test step failed and EFatal means we got a fatal error.

		struct StepInfo
		{
			enum Status { EPass, ELog, EFail, EFatal };

			int line;
			std::string desc;
			Status status;

			StepInfo() : line(0), status( ELog )
			{
			}

			StepInfo( int l, str_cref d, Status s ) :
				line( l ), desc( d ) , status( s )
			{
			}
		};


		// the Reporter interface just reports StepInfo.
		// We can have different Reporter objects attached to different test suites.
		// You can thus configure how your test results are handled.

		// Predicates are normally created within a test case with macros.
		// However one can create a test case that uses dependency injection to
		// obtain a list of Predicates and evaluate them all by stepping through
		// them in turn.

		class IOC_INTERFACES_API Predicate
		{
		public:
			virtual ~Predicate();

			virtual std::string desc() const = 0;

			// eval should return true or false or throw a std::exception derivative.
			// If it throws there will be extra info to be logged in the error message.
			virtual bool eval() const = 0;
		};

		// A RequireException is a lighter form of exception which is thrown when
		// this particular assertion must pass to continue the 
		class RequireException : public std::runtime_error
		{
		public:
			explicit RequireException( str_cref msg )
				: std::runtime_error( msg )
			{
			}
		};

	
		// A Reporter is an abstract interface for a class that processes the test results
		class IOC_INTERFACES_API Reporter 
		{
		public:
			virtual ~Reporter();

			// we log to the reporter when we start or end of a suite or a case
			// logging a step of a case, You can make a general log statement
			// within a case by reporting as a step with status ELog.

			virtual void startSuite( str_cref suiteId, str_cref suiteDesc ) = 0;
			virtual void startCase( str_cref caseId, str_cref caseDesc, str_cref suiteId ) = 0;
			virtual void reportStep( str_cref caseId, StepInfo const& info ) = 0; 
			
			// abortCase should be called when you experience an unexpected exception. 
			// A failing step that throws RequiresException is not considered an abort, albeit
			// no more steps are processed.
			virtual void abortCase( str_cref caseId, str_cref errInfo ) = 0;

			virtual void endCase( str_cref caseId ) = 0;
			virtual void endSuite( str_cref suiteId ) = 0;
		};

		// A Case is an abstract interface with some implementation. Developers will derive
		// from this class and put all runnable code test into doRun() or member functions called
		// from that function. 
		typedef spns::shared_ptr< Reporter > ReporterPtr;

		class IOC_INTERFACES_API Case
		{
		public:
			explicit Case( str_cref caseId );

			virtual ~Case();
			
			// this method is virtual but not pure virtual. The default returns empty string
			virtual std::string description() const;
		
			// called from Suite. Passes the id of the suite it is being run from
			// and the reporter. A caseId should be unique within its reporter,
			// but can be shared among different reporters. 

			// A case maintains state so the same case object
			// should not run in multiple threads simultaneously. Instead you should create
			// a matching object for each thread. (They may have the same id if they are going
			// to use different reporters, otherwise they should have different thread ids and
			// you should ensure the reporter is threadsafe)
			int run( str_cref suiteId, ReporterPtr reporter );
			
			typedef std::function< void( Predicate const&, int ) > step_function;
			typedef std::function< void( str_cref, int ) > info_function;

		protected:
			// should throw if a require fails or on a fatal
			virtual void doRun() = 0;

			// you may call this from within your test func, call __LINE__ for the 
			// the line number. You can also use one of the macros, which are provided
			// in a separate file.

			void step( Predicate const& pred, int line );

			// you can pass this as a parameter to external functions that should call this
			// parameter "step" and then
			step_function make_step()
			{
				return [this]( Predicate const& pred, int line )
					{
						this->step( pred, line );
					};
			}
			// use this to log some info.
			void info( str_cref text, int line );

			info_function make_info()
			{
				return [this]( str_cref text, int line )
				{
					this->info( text, line );
				};
			}

		private:
			std::string m_caseId;
			ReporterPtr m_reporter;
			std::atomic_int m_errorCount;
		};


		typedef spns::shared_ptr< Case > CasePtr;
	}

	inline const char * getName( Test::Case const* )
	{
		return "Test::Case";
	}

	// Just in case you want to write your own
	inline const char * getName( Test::Predicate const* )
	{
		return "Test::Predicate";
	}

	inline const char * getName( Test::Reporter const* )
	{
		return "Test::Reporter";
	}
}

#endif
