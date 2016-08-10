#pragma once
#ifndef IOC_INTERFACES_API_H
#define IOC_INTERFACES_API_H

#if defined _WIN32 || defined _WIN64

#ifndef IOC_INTERFACES_API
#define IOC_INTERFACES_API __declspec( dllimport )

#ifdef _DEBUG

#pragma comment( lib, "IOCInterfacesD.lib" )

#else

#pragma comment( lib, "IOCInterfaces.lib" )

#endif
#endif

#elif __GNUC__ >= 4
#define IOC_INTERFACES_API __attribute__ ((visibility ("default")))
#endif


#endif

