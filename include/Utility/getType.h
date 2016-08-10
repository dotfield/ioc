/*
 * getType.h
 *
 *  Created on: 3 Feb 2014
 *      Author: neil
 */

#ifndef UTILITY_GETTYPE_H_
#define UTILITY_GETTYPE_H_

// keep getType in the open namespace for basic-types
// For types that are in namespaces, you can put getType into the same
// namespace as that type, and the Koenig lookup rule will automatically
// kick in.

#include <typeinfo>

namespace Utility {
// you can overload this if the typenames produced here look ugly
template<typename T>
std::string getType( const T* t )
{
	return typeid(T).name();
}

inline std::string getType( const int * )
{
	return "Integer";
}

inline std::string getType( const double * )
{
	return "Real";
}

inline std::string getType( const std::string * )
{
	return "String";
}

}

#endif /* GETTYPE_H_ */
