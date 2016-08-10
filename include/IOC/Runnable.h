#pragma once

#ifndef IOC_RUNNABLE_H_
#define IOC_RUNNABLE_H_

#include "ioc_api.h"
#include "Builder.h"

namespace IOC 
{
	class IOC_API Runnable
	{
	private:
		int m_lastStatus;
	public:
		virtual ~Runnable();

		int run()
		{
			return m_lastStatus = doRun();
		}

		int lastStatus() const
		{
			return m_lastStatus;
		}

	protected:
		virtual int doRun() = 0;
	};

	extern template class IOC_API BuilderT<Runnable>;
}

#endif
