/**************************************************************************/
/*  linux_hw.h                                                            */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

/// Some linux-specific hardware accelerations

#ifndef LINUX_HW_H
#define LINUX_HW_H

#include <stdbool.h>
#include <stddef.h>

// Raspberry Pi VideoCore

#include <linux/ioctl.h>

#define DEVICE_FILE_NAME "/dev/vcio"
#define MAJOR_NUM 100
#define VCIO_IOC_MAGIC MAJOR_NUM
#define IOCTL_MBOX_PROPERTY _IOWR(VCIO_IOC_MAGIC, 0, char *)

// IPU hardware scaler

class LinuxHw : public Object {
	GDCLASS(LinuxHw, Object)

	static LinuxHw *instance = nullptr;

protected:
	LinuxHw();

public:
	LinuxHw *get_singleton();

	enum IPU_FILTER_TYPE {
		IPU_FILTER_BICUBIC = 0,
		IPU_FILTER_BILINEAR,
		IPU_FILTER_NEAREST,
		IPU_FILTER_LAST
	};

	/* Enables/disables downscaling when using the IPU hardware scaler */
	bool ipu_set_downscaling_enable(bool enable);
};

#endif // LINUX_HW_H
