#pragma once

#ifndef IOC_STDAFX_H
#define IOC_STDAFX_H
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <set>

#include <memory>

#if defined _WIN32 || defined _WIN64
#define IOC_API __declspec(dllexport)
#endif

#endif
