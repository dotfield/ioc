/*
 * CombinationGenerator.cpp
 *
 *  Created on: 28 Oct 2013
 *      Author: neil
 */

#include <Utility/CombinationGenerator.h>
#include <cassert>
#include <iostream>
// generates all combinations m_nSelections from m_nItems

namespace Utility {

CombinationGenerator::CombinationGenerator( size_t nItems, size_t nSelections, size_t maxCombs )
	: m_nItems( nItems ), m_nSelections( nSelections ), m_maxCombs( maxCombs ),
	  m_indices( nSelections ), m_endFlag( false ), m_isRandom( false ), m_currentCombo( 0 )
{
	assert( m_nSelections <= m_nItems );
	reset();

	// work out if we have too many combinations
	// nItems C nSelections. There is a factorial function but it may go out of range
	double combs = m_nItems;
	for( size_t num = m_nItems - 1; num > (m_nItems - m_nSelections); --num )
	{
		combs *= num;
	}
	for( size_t num = nSelections; num > 1; --num )
	{
		combs /= num;
	}

	if( combs > maxCombs )
		m_isRandom = true;
}

void CombinationGenerator::reset()
{
	for( size_t i = 0; i < m_nSelections; ++i )
	{
		m_indices[ i ] = i;
	}
	m_endFlag = false;
	m_currentCombo = 0;
}

CombinationGenerator::bitset CombinationGenerator::getAsBitset() const
{
	bitset bs( m_nItems );
	for( size_t item : m_indices )
	{
		bs.set( item, true );
	}
	return bs;
}

void CombinationGenerator::next()
{
	++m_currentCombo;
	if( m_isRandom )
	{
		if( m_currentCombo > m_maxCombs )
			m_endFlag = true;

		else
		{
			std::random_shuffle( m_indices.begin(), m_indices.end() );
		}
	}
	else
	{
		// find the right-most position that is not "pos"
		size_t pos = m_nSelections;
		while( pos && (m_indices[pos-1] == m_nItems + pos - m_nSelections -1) )
			--pos;

		if( pos )
		{
			size_t item = ++m_indices[pos-1];
			for( size_t i = pos; i < m_nSelections; ++i )
			{
				m_indices[ i ] = ++item;
			}
		}
		else
		{
			m_endFlag = true;
		}
	}
}

}
