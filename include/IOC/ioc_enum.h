#pragma once
#ifndef IOC_ENUM_H_
#define IOC_ENUM_H_

namespace IOC
{
	enum ExpressionType 
	  { 
		EString, EBool, EInt, EReal, // these 4 are literal types
		EVoid, // function type with no parameters
		EVariable, // this refers to something defined elsewhere
		EList, EMap, EConcat, ELibrary, EClass, EPair, EObject, ECurrentDir, // these 6 are expressions
		EError // parsing error
	  };
}

#endif
