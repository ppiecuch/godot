
/// Some linux-specific hardware accelerations

#ifndef LINUX_HW_H
#define LINUX_HW_H

#include <stddef.h>
#include <stdbool.h>

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
