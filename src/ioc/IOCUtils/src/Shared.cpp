// Normally we pass std::string and vector by value, but we use shared_ptr when we really want to share
// the string or vector between two objects, i.e. one will write and the other will read.

// To do this you still need a builder so we implement them here
// (We might have put these into the IOC static library, similarly with the others in this library).

#include <IOC/BuilderNParams.h>

// a quick way to make 0-param builders through a typedef
namespace {

	// Note that these are builders to the type, e.g. std::string, which will produce 
	// a shared_ptr to the type as builders do.
	template< typename SharedType >
	struct Shared
	{
		typedef IOC::Builder0Params< SharedType, SharedType > builder_type;
		typedef IOC::BuilderFactoryImpl< builder_type > BuilderFactory;
	};
}

extern "C" {
	IOC_API Shared<std::string>::BuilderFactory g_SharedString;
	IOC_API Shared<std::wstring>::BuilderFactory g_SharedWString;
	IOC_API Shared<std::vector<char> >::BuilderFactory g_SharedVecChar;
	IOC_API Shared<std::vector<unsigned char> >::BuilderFactory g_SharedVecUChar;
	IOC_API Shared<std::vector<wchar_t> >::BuilderFactory g_SharedVecWChar;
	IOC_API Shared<std::vector<double> >::BuilderFactory g_SharedVecDbl;
	IOC_API Shared<std::vector<int> >::BuilderFactory g_SharedVecInt;
	IOC_API Shared<std::vector<size_t> >::BuilderFactory g_SharedVecUInt;
	IOC_API Shared<std::vector<std::string> >::BuilderFactory g_SharedVecStr;
	IOC_API Shared<std::vector<std::wstring> >::BuilderFactory g_SharedVecWStr;
	

}
