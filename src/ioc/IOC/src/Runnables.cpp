#include "stdafx.h"
#include <IOC/Runnable.h>
#include <IOC/BuilderNParams.h>
#include <IOC/LibraryStaticImpl.h>

#include <boost/thread.hpp>

namespace IOC
{
	Runnable::~Runnable()
	{
	}

	template class BuilderT<Runnable>;

	class SequentialRunnableList : public Runnable
	{
	private:
		std::vector< RunnablePtr > m_runnables;

	public:
		SequentialRunnableList( std::vector< RunnablePtr > const& runnables )
			: m_runnables( runnables )
		{
		}

		// a runnable should return as follows:
		// 0 = success
		// 1 = fail but continue
		// any other value: fail and stop
		int doRun()
		{
			int res = 0;
			for( std::vector<RunnablePtr>::const_iterator iter= m_runnables.begin(),
						end = m_runnables.end();
					iter != end && !( res & ~1 );
					++iter )
			{
				res |= (*iter)->run();
			}

			return res;
		}
	};
	
	// this is how simple it is to create a Builder for your objects...
	typedef Builder1Param< SequentialRunnableList, Runnable, std::vector<Runnable> > SequentialRunnableListBuilder;

	class ParallelRunnableList : public Runnable
	{
	private:
		std::vector< RunnablePtr > m_runnables;

	public:
		ParallelRunnableList( std::vector< RunnablePtr > const& runnables )
			: m_runnables( runnables )
		{
		}

		// we run them all at once and there is no "abort". Implementations will need
		// to check for some status to decide whether to continue running
		// Note that all LOADING is done in single-threaded mode but RUNNING can take
		// place in a multi-threaded environment.

		int doRun()
		{
			boost::thread_group group;
			for( std::vector<RunnablePtr>::const_iterator iter= m_runnables.begin(),
						end = m_runnables.end();
					iter != end; ++iter )
			{
				group.create_thread( [ iter ]{ (*iter)->run(); } );
			}
			group.join_all();
			int res = 0;
			for( std::vector<RunnablePtr>::const_iterator iter= m_runnables.begin(),
						end = m_runnables.end();
						iter != end; ++iter )
			{
				res |= (*iter)->lastStatus();
			}

			return res;
		}
	};

	typedef Builder1Param< ParallelRunnableList, Runnable, std::vector<Runnable> > ParallelRunnableListBuilder;

	void initIOCObjLibrary()
	{
		// this is how simple it is to create the symbol for your builder although if it is to be
		// dynamically loaded, either create it with dllexport status or put name in def file
		// and do not declare it within a namespace or a function, of course, but in a compilation
		// unit somewhere. You'll have to qualify IOC::BuilderFactoryImpl of course 

		static BuilderFactoryImpl< SequentialRunnableListBuilder > sequentialRunnableListFactory;
		static BuilderFactoryImpl< ParallelRunnableListBuilder > parallelRunnableListFactory;
		static bool isInit = false;

		// should be done whilst in single threaded state so no need for boost::once or similar
		// This section should not throw
		if( !isInit )
		{
			LibraryTable & table = libraryTableInstance();
			LibraryStaticImpl * lib = new LibraryStaticImpl( "IOC", "IOC.dll" );
			LibraryPtr libPtr( lib );
			lib->addSymbol( "SequentialRunnableList", &sequentialRunnableListFactory );
			lib->addSymbol( "ParallelRunnableList", &parallelRunnableListFactory );

			table.addStaticLibrary( "IOC", libPtr );

			isInit = true;
		}
	}
}




			
