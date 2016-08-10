#pragma once

#ifndef IOC_DL_OBJECT_H_
#define IOC_DL_OBJECT_H_

#include "iocfwd.h"
#include "ioc_api.h"

namespace IOC {
// this seemingly has no purpose but actually it does have one
// It must be the base class for any class type whose objects we dynamically load
// with GetProcAddress.

class IOC_API DLObject
{
public:
	virtual ~DLObject() = 0;
};

}

#endif
