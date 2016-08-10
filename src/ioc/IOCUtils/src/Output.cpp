#include "stdafx.h"

#include <IOC/BuilderNParams.h> 
#include <IOCInterfaces/Output.h>

#include <Utility/OutputLogger.h>

#include <iostream>
#include <fstream>
#include <sstream>

// It is not within the scope of this library to implement an Output that writes to log4cxx or similar,
// but you certainly can have one. Implement it somewhere where log4cxx is within scope.

// our builders
namespace {

using namespace IOC;
using Utility::Output;
using Utility::WOutput;

typedef Builder2Params< Utility::BasicFileOutput<char>, Output, std::string, bool > FileOutputBuilder;
typedef Builder0Params< Utility::ConsoleOutput, Output > ConsoleOutputBuilder;
typedef Builder0Params< Utility::ConsoleError, Output > ConsoleErrorBuilder;
typedef Builder1Param< Utility::BasicSharedStringOutput<char>, Output, spns::shared_ptr<std::string> > SharedStringOutputBuilder;
typedef Builder2Params< Utility::BasicFileOutput<wchar_t>, WOutput, std::string, bool > WFileOutputBuilder;
typedef Builder0Params< Utility::WConsoleOutput, WOutput > WConsoleOutputBuilder;
typedef Builder0Params< Utility::WConsoleError, WOutput > WConsoleErrorBuilder;
typedef Builder1Param< Utility::BasicSharedStringOutput<wchar_t>, WOutput, spns::shared_ptr<std::wstring> > SharedWStringOutputBuilder;

typedef Builder2Params< Utility::OutputLogger, Utility::Logger, Output, size_t > OutputLoggerBuilder;
typedef Builder1Param< Utility::LoggerSubscriber, Utility::LoggerSubscriber, std::vector<Utility::Logger> > LoggerSubscriberBuilder;

typedef Builder1Param< Utility::MTOutput, Utility::MTOutput, Output > MTOutputBuilder;
typedef Builder1Param< Utility::MTWOutput, Utility::MTWOutput, WOutput > MTWOutputBuilder;

// No builders for CallbackOutput, not supported in IOC
// Alternatives are:
// 1. Create your own callback derived from String (or WString) Output which does whatever in flush() and make a builder for it.
// 2. If the callback isn't available at load time, create some kind of factory for it

}

using IOC::BuilderFactoryImpl;

extern "C" 
{
  IOC_API BuilderFactoryImpl< FileOutputBuilder > g_FileOutput;
  IOC_API BuilderFactoryImpl< ConsoleOutputBuilder > g_ConsoleOutput;
  IOC_API BuilderFactoryImpl< ConsoleErrorBuilder > g_ConsoleError;
  IOC_API BuilderFactoryImpl< SharedStringOutputBuilder > g_SharedStringOutput;
  IOC_API BuilderFactoryImpl< WFileOutputBuilder > g_WFileOutput;
  IOC_API BuilderFactoryImpl< WConsoleOutputBuilder > g_WConsoleOutput;
  IOC_API BuilderFactoryImpl< WConsoleErrorBuilder > g_WConsoleError;
  IOC_API BuilderFactoryImpl< MTOutputBuilder > g_MTOutput;
  IOC_API BuilderFactoryImpl< MTWOutputBuilder > g_MTWOutput;
  IOC_API BuilderFactoryImpl< SharedWStringOutputBuilder > g_SharedWStringOutput;
  IOC_API BuilderFactoryImpl< OutputLoggerBuilder > g_OutputLogger;
  IOC_API BuilderFactoryImpl< LoggerSubscriberBuilder > g_LoggerSubscriber;
}
