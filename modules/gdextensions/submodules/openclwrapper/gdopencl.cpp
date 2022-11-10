/*************************************************************************/
/*  gdopencl.cpp                                                         */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "gdopencl.h"

void OpenCLDevice::test() {
	const uint N = 1024u; // size of vectors
	CL::Memory<float> A(device, N); // allocate memory on both host and device
	CL::Memory<float> B(device, N);
	CL::Memory<float> C(device, N);

	CL::Kernel add_kernel(device, N, "add_kernel", A, B, C); // kernel that runs on the device

	for (uint n = 0u; n < N; n++) {
		A[n] = 3.0f; // initialize memory
		B[n] = 2.0f;
		C[n] = 1.0f;
	}

	cl::print_info("Value before kernel execution: C[0] = " + ::to_string(C[0]));

	A.write_to_device(); // copy data from host memory to device memory
	B.write_to_device();
	add_kernel.run(); // run add_kernel on the device
	C.read_from_device(); // copy data from device memory to host memory

	cl::print_info("Value after kernel execution: C[0] = " + ::to_string(C[0]));
}

OpenCLDevice::OpenCLDevice() {
	device = select_device_with_most_flops(); // compile OpenCL C code for the fastest available device
}
