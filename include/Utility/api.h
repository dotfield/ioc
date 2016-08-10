/*
 * api.h
 *
 *  Created on: 27 Nov 2013
 *      Author: neil
 */

#ifndef UTILITY_API_H_
#define UTILITY_API_H_

#if defined _WIN32 || defined _WIN64

#ifndef UTILITY_API
#define UTILITY_API __declspec( dllimport )

#ifdef _DEBUG

#pragma comment( lib, "UtilityD.lib" )

#else

#pragma comment( lib, "Utility.lib" )

#endif
#endif

#elif __GNUC__ >= 4
#define UTILITY_API __attribute__ ((visibility ("default")))
#endif


#endif /* API_H_ */
