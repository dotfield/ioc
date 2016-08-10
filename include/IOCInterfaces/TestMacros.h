#pragma once

#ifndef IOC_TEST_MACROS_H_
#define IOC_TEST_MACROS_H_

#include "TestPredicates.h"

/*
	TestMacros
	----------
	As with all testing unit libraries this one provides macros, but I feel there is a major
	difference here.

	In the others I have encountered, you seem to put in macros everywhere - defining the tests, 
	running them, etc.

	In this one the macros are here really only to provide reflection and line-info, plus a few more complex
	multi-line actions. (Reflection means it prints the text as well as evaluating it, thus the
	#expr symbols around these implementations), 

	One can write unit tests without using these, and manually calling the methods in TestIOC::Test::Predicates.h
	You can also use dependency injection fully with your own IOC::Test::predicate objects.
	(Remember however that IOC scripts do not provide flow, they just allow you to create objects written
	in C++ and run functionality implemented in an IOC::Runnable).

*/

#define IOCTEST_TRUE( expr ) \
	step( IOC::Test::predNonZero(#expr, NULL, expr), __LINE__ )

#define IOCTEST_TRUE_DESC( expr, desc ) \
	step( IOC::Test::predNonZero(#expr, desc, expr), __LINE__ )

#define IOCTEST_FALSE( expr ) \
	step( IOC::Test::predZero(#expr, NULL, expr), __LINE__ )

#define IOCTEST_FALSE_DESC( expr ) \
	step( IOC::Test::predZero(#expr, desc, expr), __LINE__ )

#define IOCTEST_REQUIRES( expr ) \
	do{ \
	if( expr ) step( IOC::Test::PredSuccess(#expr), __LINE__ ); \
	else step( IOC::Test::PredThrows( #expr, "REQUIRED" ), __LINE__ ); \
	} while( false )

// Note that if any exception leaks from step, it ends the test case .
// If the type is any other than RequireException it is considered a fatal error for the test case.

// If testing functions that may throw, use these 

// check that it does not throw a std exception
#define IOCTEST_NOTHROW( expr ) \
	do{ \
	try \
	{ \
	    expr; \
		step( IOC::Test::PredSuccess(#expr), __LINE__ ); \
	} \
	catch( std::exception const& err ) \
	{ \
		step( IOC::Test::PredThrows( #expr, err.what() ), __LINE__ ); \
	} \
	} while( false )

// A different exception: you must overload describe() for the exception
// to get a string
#define IOCTEST_NOTHROW_EX( expr, type ) \
	do{ \
	try \
	{ \
	    expr; \
		step( IOC::Test::PredSuccess(#expr), __LINE__ ); \
	} \
	catch( type const& err ) \
	{ \
		step( IOC::Test::PredThrows( #expr, describe(err) ), __LINE__ ); \
	} \
	} while( false )

// These test that something should throw. If they don't, the step fails but doesn't
// throw
#define IOCTEST_THROWS( expr ) \
	do{ \
	try \
	{ \
		expr; \
		step( IOC::Test::PredFail(#expr + std::string("did not throw.") ), __LINE__ ); \
	} \
	catch( std::exception const& err ) \
	{ \
		step( IOC::Test::PredSuccess( #expr + std::string("threw ") + err.what() ), __LINE__ ); \
	} \
	} while( false )

// a different exception: again overload describe()
#define IOCTEST_THROWS_EX( expr, type ) \
	do{ \
	try \
	{ \
		expr; \
		step( IOC::Test::PredFail(#expr + std::string("did not throw.") ), __LINE__ ); \
	} \
	catch( type const& err ) \
	{ \
		step( IOC::Test::PredSuccess( #expr + std::string("threw ") + describe(err) ), __LINE__ ); \
	} \
	} while( false )

// comparison IOC::Test::predicates. These are all one-liners
#define IOCTEST_EQUAL( left, right ) \
	step( IOC::Test::predEqual( left, right, #left, #right ), __LINE__ )

#define IOCTEST_NOT_EQUAL( left, right ) \
	step( IOC::Test::predNotEqual( left, right, #left, #right ), __LINE__ )

#define IOCTEST_LESS( left, right ) \
	step( IOC::Test::predLess( left, right, #left, #right ), __LINE__ )

#define IOCTEST_LESS_EQUAL( left, right ) \
	step( IOC::Test::predLessEqual( left, right, #left, #right ), __LINE__ )

#define IOCTEST_GREATER( left, right ) \
	step( IOC::Test::predGreater( left, right, #left, #right ), __LINE__ )

#define IOCTEST_GREATER_EQUAL( left, right ) \
	step( IOC::Test::predGreaterEqual( left, right, #left, #right ), __LINE__ )

#define IOCTEST_APPROX_DIFF( left, right, tol ) \
	step( IOC::Test::predDoubleDiff( left, right, #left, #right, tol ), __LINE__ )

#define IOCTEST_APPROX_RATIO( left, right, tol ) \
	step( IOC::Test::predDoubleRatio( left, right, #left, #right, tol ), __LINE__ )

// this macros logs information
#define IOCTEST_INFO( text ) \
	info( text, __LINE__ )

#define IOCTEST_INFO_STREAM( streamed ) \
	do { \
		std::ostringstream oss; \
		oss << streamed; \
		info( oss.str(), __LINE__ ); \
	} while( false )

// this also does but indicates that a test has passed so counts as a passed predicate
#define IOCTEST_PASS( text ) \
	step( IOC::Test::PredSuccess( text ), __LINE__ )

#define IOCTEST_PASS_STREAM( streamed ) \
	do { \
		std::ostringstream oss; \
		oss << streamed; \
		step( IOC::Test::PredSuccess( oss.str() ), __LINE__ ); \
	} while( false );

// This is a failure but not fatal so it will go on testing
#define IOCTEST_FAIL( text ) \
	step( IOC::Test::PredFail( text ), __LINE__ )

#define IOCTEST_FAIL_STREAM( streamed ) \
	do { \
		std::ostringstream oss; \
		oss << streamed; \
		step( IOC::Test::PredFail( oss.str() ), __LINE__ ); \
	} while( false );

#endif
