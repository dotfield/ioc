#pragma once

#ifndef IOC_OBJECT_LOADER_H
#define IOC_OBJECT_LOADER_H

#include "../iocfwd.h"
#include <vector>
#include <map>

namespace IOC {


// The ObjectLoader is responsible for storing all the loaded libraries, objects, builders
// and for storing the map of aliases to their underlying expressions.

class IOC_API ObjectLoader
{
public:
	virtual ~ObjectLoader();

	// any vector or map parameters are returned as an alias, and must then be
	// resolved with lookupVector or lookupMap.
	
	// get the expression from a name. Throws if not found
	virtual expr_cref lookup( str_cref name ) const = 0;

	// This is always returned as a vector of strings. If they are objects, you then
	// need to resolve the objects. If you want numbers they must be lexically cast.

protected:
	// these may either lexically cast or return a variable that has the value

	// toString does not simply return the string that is input.
	// Input is either a quoted string in which case it removes the quotes, or it is
	// an alias to a constant in which case it returns the constant.
	// It will also resolve embedded values

	virtual double toDouble( expr_cref value ) const = 0;
	virtual int toInt( expr_cref value ) const = 0;
	virtual size_t toUInt( expr_cref value ) const = 0;
	virtual bool toBool( expr_cref value ) const = 0;
	virtual std::string toString( expr_cref value ) const = 0;
	
	// We do not properly support UNICODE but we support apps that want their UTF-8 strings in this format
	virtual std::wstring toWString( expr_cref value ) const = 0;

public:
	// to convert a vector we have to go through this...
	virtual void toVector( expr_cref value, std::vector< RecursiveExpressionPtr > & res ) const = 0;
	// after which w convert each expression returned

	// to convert to a map we have to go through this...
	virtual void toMap( expr_cref value, 
		std::vector< std::pair< RecursiveExpressionPtr, RecursiveExpressionPtr > > & res ) const = 0;
	// and then convert each key-value pair.

	// this is different in implementation from toString as it gets the last "variable" in the
	// chain that must otherwise be undefined. Note that variable names cannot use :: in their
	// names. Use . as a namespace separator if you want one... This is the main reason I allowed
	// the use of . in the names

	virtual std::string toEnum( expr_cref value ) const = 0;

	// gets the "underlying" expression for this one if this is a Variable (or a Concat)
	virtual expr_cref underlying( expr_cref start, std::string * lastvar = NULL, bool throwIfNotFound=true ) const = 0;
	// Note: these functions are all called "convert" on purpose.. It enables our meta-programming
	// in LitParamBinder
	void convert( expr_cref value, double& d ) const
	{
		d = toDouble( value );
	}

	void convert( expr_cref value, int& i ) const
	{
		i = toInt( value );
	}

	void convert( expr_cref value, size_t & ui ) const
	{
		ui = toUInt( value );
	}

	void convert( expr_cref value, bool & ui ) const
	{
		ui = toBool( value );
	}

	void convert( expr_cref value, std::string & str ) const
	{
		str = toString( value );
	}

	void convert( expr_cref value, std::wstring & wstr ) const
	{
		wstr = toWString( value );
	}

	// get the builder for this object
	virtual BuilderPtr getBuilder( expr_cref value ) const = 0;
	
};

}

#endif

