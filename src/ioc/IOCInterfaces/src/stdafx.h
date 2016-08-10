#pragma once
#ifndef STDAFX_H
#define STDAFX_H

#if defined _WIN32 || defined _WIN64
#define IOC_INTERFACES_API __declspec( dllexport )
#endif

#include <string>
#include <sstream>
#include <stdexcept>
#include <memory>
#include <boost/shared_ptr.hpp>

#endif
