#include "stdafx.h"

#include <IOCInterfaces/Test.h>
#include <IOCInterfaces/Output.h>
#include <IOC/Runnable.h>
#include <IOC/BuilderNParams.h>
#include <Utility/datetime.h>
#include <Utility/logging.h>

namespace IOC { namespace Test {

	
// Suite is a concrete class, and derives from IOC::Runnable. Therefore it doesn't need to
// be included in a header.

class Suite : public Runnable
{
public:
	Suite( str_cref suiteId, str_cref desc, ReporterPtr reporter, std::vector< CasePtr > const& cases ) :
		m_suiteId( suiteId),
		m_desc( desc ),
		m_reporter( reporter ),
		m_cases( cases )
	{
	}

	// we don't need to do anything with the loggers, it just becomes part of the chain.
	// Note it is the first parameters so it gets created first
	Suite( std::shared_ptr<Utility::LoggerSubscriber> const& loggerSubscriber,
			str_cref suiteId, str_cref desc, ReporterPtr reporter, std::vector< CasePtr > const& cases )
	:   m_suiteId( suiteId),
		m_desc( desc ),
		m_reporter( reporter ),
		m_cases( cases )
	{
	}

protected:
	int doRun();

private:
	std::string m_suiteId;
	std::string m_desc;
	ReporterPtr m_reporter;
	std::vector< CasePtr > m_cases;

};

int Suite::doRun()
{
	m_reporter->startSuite( m_suiteId, m_desc );
	int totalErrorCount = 0;
	// manual loop
	for( std::vector<CasePtr>::const_iterator iter = m_cases.begin(), iterEnd = m_cases.end();
		iter != iterEnd; ++iter )
	{
		CasePtr casePtr = *iter;
		int errorCount = casePtr->run( m_suiteId, m_reporter );
		if( errorCount < 0 )
		{
			totalErrorCount = -1;
			break; // no more test cases can run
		}
		else
		{
			totalErrorCount += errorCount;
		}
	}
	m_reporter->endSuite( m_suiteId );
	return totalErrorCount;
}

struct CaseStat
{
	std::string suite;
	size_t numSteps;
	int lastLine;
	int result;
	
	CaseStat() : numSteps(0), lastLine(-1), result( 0 )
	{
	}
};

struct SuiteStat
{
	size_t numCases;
	int result;

	SuiteStat() : numCases( 0 ), result ( 0 )
	{
	}
};

class BasicReporter : public Reporter
{
private:
	Utility::OutputPtr m_output;
	bool m_verbose;

public:
	BasicReporter( Utility::OutputPtr output, bool verbose ) : m_output( output ), m_verbose( verbose )
	{
	}

	virtual void startSuite( str_cref suiteId, str_cref suiteDesc );
	virtual void startCase( str_cref caseId, str_cref caseDesc, str_cref suiteId );
	virtual void reportStep( str_cref caseId, StepInfo const& info ); 
			
			// abortCase should be called when you experience an unexpected exception. 
			// A failing step that throws RequiresException is not considered an abort, albeit
			// no more steps are processed.
	virtual void abortCase( str_cref caseId, str_cref errInfo );

	virtual void endCase( str_cref caseId );
	virtual void endSuite( str_cref suiteId );

private:
	std::map< std::string, CaseStat > m_caseStats;
	std::map< std::string, SuiteStat > m_suiteStats;

};

// in seconds. We don't bother with milliseconds
std::string timestamp()
{
	static Utility::DateTimeFormatter dtf( "%Y-%b-%d %H:%M:%S" );

	boost::posix_time::ptime curTime = boost::date_time::second_clock<boost::posix_time::ptime>::local_time();
	return dtf.printTime( curTime );
}

void BasicReporter::startSuite( str_cref suiteId, str_cref suiteDesc )
{
	// just log it
	m_output->os() << timestamp() << " - start test suite " << suiteId << '\n';
	if( !suiteDesc.empty() )
	{
		m_output->os() << "\t - " << suiteDesc << '\n';
	}

	m_output->flush();
	// no need to do anything to the stat for it
}

void BasicReporter::startCase( str_cref caseId, str_cref caseDesc, str_cref suiteId )
{
	m_output->os() << timestamp() << " - start test case " << caseId << " in test suite " << suiteId << '\n';
	if( !caseDesc.empty() )
	{
		m_output->os() << "\t - " << caseDesc << '\n';
	}

	// add a stat for this case
	CaseStat & caseStat= m_caseStats[ caseId ];
	caseStat.suite = suiteId; // this enables us to mark the status of the suite in which it belongs

	SuiteStat & suiteStat = m_suiteStats[ suiteId ];
	++suiteStat.numCases;
	m_output->flush();
}

// order is EPass, ELog, EFail, EFatal. Log is marked as INFO

const char * const statusStr[] = { "PASS", "INFO", "FAIL", "FATAL" };

void BasicReporter::reportStep( str_cref caseId, StepInfo const& info )
{
	// in non-verbose mode we do not log steps that have a status of EPass
	// Instead we just gather the stats of how many steps were run
	CaseStat & caseStat = m_caseStats[ caseId ];

	if( m_verbose || (info.status != StepInfo::EPass) )
	{
		m_output->os() << statusStr[ info.status ] << " line " << info.line << " - " << info.desc << '\n';
	}
	
	// You don't get fatal status through this function, only through abortCase
	// therefore we'd only mark an "EFail"
	
	// the step doesn't count for the stats if it's just a log
	if( info.status != StepInfo::ELog )
		caseStat.numSteps ++;

	// even in that case we still take its line. The purpose is that if we got a fatal
	// error we know the last line that was logged.
	caseStat.lastLine = info.line;

	if( info.status == StepInfo::EFail )
	{
		++caseStat.result;
	}

	m_output->flush();
}

// abortCase should be called when you experience an unexpected exception. 
// A failing step that throws RequiresException is not considered an abort, albeit
// no more steps are processed. A case is always ended by a call to either endCase or abortCase
// (unless the test suite totally crashes)

void BasicReporter::abortCase( str_cref caseId, str_cref errInfo )
{
	// get the stat for this case
	CaseStat & caseStat = m_caseStats[ caseId ];

	m_output->os() << timestamp() << " - abort test case " << caseId << 
		" after line " << caseStat.lastLine << " in suite " << caseStat.suite
		<< "\n\t reason: " << errInfo << '\n';

	// get the suiteStat for this case
	SuiteStat & suiteStat = m_suiteStats[ caseStat.suite ];
	suiteStat.result = -1; // indicate that the suite failed. 
		// When we get endSuite called shortly we will be able to report that
	m_output->flush();
}

void BasicReporter::endCase( str_cref caseId )
{
	// get the case stat for this
	CaseStat & caseStat = m_caseStats[ caseId ];

	m_output->os() << timestamp() << " completed test case " << caseId << " in suite " << caseStat.suite
		<< " with " << caseStat.numSteps << (caseStat.numSteps != 1 ? " checks" : " check");

	// it will never be -1 as that goes through abortCase not endCase
	switch ( caseStat.result )
	{
	case 0:
		m_output->os() << " - Passed\n";
		break;

	case 1: // may as well have a case for 1 so we don't write 1 errors
		m_output->os() << " - failed with 1 error\n";
		break;

	default:
		m_output->os() << " - failed with " << caseStat.result << " errors\n";
		break;
	}

	if( caseStat.result != 0 )
	{
		SuiteStat & suiteStat = m_suiteStats[ caseStat.suite ];
		if( suiteStat.result != -1 )
		{
			++suiteStat.result;
		}
	}
	m_output->flush();
}

void BasicReporter::endSuite( str_cref suiteId )
{
	SuiteStat & suiteStat = m_suiteStats[ suiteId ];

	m_output->os() << timestamp() << " - completed test suite " << suiteId <<
		" with " << suiteStat.numCases << " test cases";

	switch( suiteStat.result )
	{
	case 0:
		m_output->os() << " - All tests passed\n";
		break;

	case 1:
		m_output->os() << " - 1 test failed\n";
		break;

	case -1:
		m_output->os() << " - aborted due to aborting test case\n";
		break;

	default:
		m_output->os() << " - " << suiteStat.result << " tests failed\n";
	}

	m_output->flush();
}

// make builders for them

typedef Builder4Params< Suite, Runnable, std::string, std::string, Reporter, std::vector<Case> > SuiteBuilder;
typedef Builder5Params< Suite, Runnable, Utility::LoggerSubscriber, std::string, std::string, Reporter, std::vector<Case> > SuiteWithLoggingBuilder;
typedef Builder2Params< BasicReporter, Reporter, Utility::Output, bool > BasicReporterBuilder;

} }

using IOC::BuilderFactoryImpl;

extern "C" {

	IOC_API BuilderFactoryImpl< IOC::Test::SuiteBuilder > g_testSuite;
	IOC_API BuilderFactoryImpl< IOC::Test::SuiteWithLoggingBuilder > g_testSuiteWithLogging;
	IOC_API BuilderFactoryImpl< IOC::Test::BasicReporterBuilder > g_testBasicReporter;

}
