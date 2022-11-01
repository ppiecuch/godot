#ifndef GD_OPENCL_H
#define GD_OPENCL_H

#include "core/object.h"
#include "core/variant.h"

#include "opencl.hpp"

/// OpenCLDevice singleton

class OpenCLDevice : public Object {
	GDCLASS(OpenCLDevice, Object);

	CL::Device device;

public:
	void test();

	OpenCLDevice();
};

#endif // GD_OPENCL_H
