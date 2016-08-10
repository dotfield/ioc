#ifndef COMBINATION_GENERATOR_H
#define COMBINATION_GENERATOR_H

#include <vector>
#include <boost/dynamic_bitset.hpp>
#include "api.h"

namespace Utility {

class UTILITY_API CombinationGenerator
{
	size_t m_nItems;
	size_t m_nSelections;
	size_t m_maxCombs;

	std::vector< size_t > m_indices;
	bool m_endFlag; // set when there are no more combinations

	bool m_isRandom;
	size_t m_currentCombo;

public:
	typedef boost::dynamic_bitset<unsigned int> bitset;

	CombinationGenerator( size_t nItems, size_t nSelections, size_t maxCombs );

	std::vector< size_t > const& get() const
	{
		return m_indices;
	}

	// This version returns a bitset for which are set and not set
	bitset getAsBitset() const;

	void next();
	void reset();
	bool isEnd() const
	{
		return m_endFlag;
	}
};


// createSplit
// flat number of items and a bitset. For each element of
// flat, if its corresponding bit is set it goes into left
// otherwise it goes into right.
// The precondition is that the size of left must be at least
// the number of bits set, and the size of right must be at
// least the number of bits not set. If the output iterators are
// insert-iterators the precondition does not need to be met, of course.

template< typename OutIter, typename Seq >
void createSplit
    (
		OutIter left,
		OutIter right,
		Seq const& flat,
		CombinationGenerator::bitset const& bits
	)
{
	size_t i = 0;
	for( auto const& val : flat )
	{
		if( bits.test( i ) )
		{
			*left = val;
			++left;
		}
		else
		{
			*right = val;
			++right;
		}
		++i;
	}
}

}

#endif

