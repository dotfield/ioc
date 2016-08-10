/*
 * input.h
 *
 *  Created on: 17 May 2015
 *      Author: neil
 */
#pragma once

#ifndef UTILITY_INPUT_H_
#define UTILITY_INPUT_H_

#include "api.h"
#include <mutex>
#include <istream>

namespace Utility {

class MTInput // all inlined
{
	std::istream& m_iStream;
	std::mutex m_mutex;
	size_t m_lineNum;

public:
	explicit MTInput( std::istream& iStream )
		: m_iStream( iStream )
		, m_lineNum( 0 )
	{
	}

	// returns a 1-based line number or 0 if we have reached end of stream
	size_t getline( std::string& line )
	{
		std::unique_lock< std::mutex > lock( m_mutex );
		if( std::getline( m_iStream, line ))
		{
			return ++m_lineNum;
		}
		else
		{
			return 0;
		}
	}
};

}

#endif /* UTILITY_INPUT_H_ */
