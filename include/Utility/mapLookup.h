#pragma once
#ifndef MAP_LOOKUP_H_
#define MAP_LOOKUP_H_

// This template does not belong here and goes in a very generic algorithm area

#include <map>
namespace Utility {

// function looks up in the map. Returns whether found or not
// If found it populates value, otherwise does not modify it.

// Coll should be std::map or std::unordered_map but not multimap

template< typename K, typename V, typename Coll >
bool mapLookup( Coll const& map, const K& key, V& value )
{
	typename Coll::const_iterator iter = map.find(key);
	if( iter == map.end() )
	{
		return false;
	}
	else
	{
		value = iter->second;
		return true;
	}
}

// extracts data from the map if found. If not found returns the
// supplied default or the type default if none is supplied.

template< typename K, typename Coll, typename V=typename Coll::mapped_type >
V mapGet( Coll const & map, K const& key, V const& defV = V() )
{
	V val = defV;
	mapLookup( map, key, val );
	return val;
}

}

#endif
