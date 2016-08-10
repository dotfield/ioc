#include "stdafx.h"

#include <IOCInterfaces/Test.h>
// No need to include TestPredicates.h. Predicate itself is in Test.h

namespace IOC { namespace Test {



Predicate::~Predicate()
{
}

Reporter::~Reporter()
{
}

Case::Case( str_cref caseId ) : m_caseId( caseId ), m_errorCount( 0 )
{
}

Case::~Case()
{
}

int Case::run( str_cref suiteId, ReporterPtr reporter )
{
	m_reporter = reporter;
	reporter->startCase( m_caseId, description(), suiteId );

	m_errorCount = 0;
//	StepInfo::Status stat = StepInfo::EPass;
	
	try
	{
		doRun();
	}
	catch( RequireException const& )
	{
		// nothing to do
	}
	catch( std::exception const& err ) 
	{
		// Another exception that may or may not have been caught within step().
		// If not (e.g. it is in the doRun() function but not within a controlled predicate)
		// we can now log the exception itself but don't know its line number.
		// If it was caught within a step the line number will have been logged.
		// Either way it is considered a fatal error for this test case, which not only
		// ends execution of the test case but of the whole suite.

		// If you don't want this, you should put IOC_NOTHROW() for a std::exception
		// derivative or IOC_NOTHROW_EX for another exception type around the
		// statement that might throw in which case it will be only a RequireException.
		
		reporter->abortCase( m_caseId, err.what() );
		return -1;
	}
	catch( ... ) // wouldn't happen from anything within a Step which catches these
		// and returns them as std::exception
	{
		reporter->abortCase( m_caseId, "Unknown error" );
		return -1;
	}

	reporter->endCase( m_caseId );
	return m_errorCount;
}

void Case::step( Predicate const & pred, int line )
{
	try
	{
		bool success = pred.eval();
		StepInfo info( line, pred.desc(), success ? StepInfo::EPass : StepInfo::EFail );
		m_reporter->reportStep( m_caseId, info );
	}
	catch( RequireException const& err )
	{
		StepInfo info( line, pred.desc() + " - Failed with error " + err.what(), StepInfo::EFail );
		m_reporter->reportStep( m_caseId, info );
		throw;
	}
	// any other exception is fatal and gets thrown.
	catch( std::exception const& err )
	{
		StepInfo info( line, pred.desc() + " - threw unexpected exception " + err.what(), 
			StepInfo::EFatal );
		m_reporter->reportStep( m_caseId, info );
		throw;
	}
	catch( ... )
	{
		StepInfo info( line, pred.desc() + " - failed with fatal unknown error ", StepInfo::EFatal );
		m_reporter->reportStep( m_caseId, info );
		std::ostringstream oss;
		oss << "Error at line " << line;
		throw std::runtime_error( oss.str() );
	}
}

void Case::info( str_cref text, int line )
{
	m_reporter->reportStep( m_caseId, StepInfo( line, text, StepInfo::ELog) );
}

std::string Case::description() const
{
	return std::string();
}


} }
