
#include "core/os/file_access.h"
#include "core/variant.h"

#include <ctype.h>
#include <stdio.h>
#include <sys/utsname.h>

/* Specifies all possible image filtering
 * methods when using the IPU hardware scaler
 * > Note: We do not allow 'fine tuning' of the
 *   bicubic sharpness factor, since anything
 *   other than the default value looks terrible... */
enum dingux_ipu_filter_type {
	DINGUX_IPU_FILTER_BICUBIC = 0,
	DINGUX_IPU_FILTER_BILINEAR,
	DINGUX_IPU_FILTER_NEAREST,
	DINGUX_IPU_FILTER_LAST
};

/* Enables/disables downscaling when using the IPU hardware scaler */
static bool dingux_ipu_set_downscaling_enable(bool enable);

/* Sets the video scaling mode when using the
 * IPU hardware scaler
 * - keep_aspect: When 'true', aspect ratio correction
 *   (1:1 PAR) is applied. When 'false', image is
 *   stretched to full screen dimensions
 * - integer_scale: When 'true', enables integer
 *   scaling. This implicitly sets keep_aspect to
 *   'true' (since integer scaling is by definition
 *   1:1 PAR)
 * Note: OpenDingux stock firmware allows keep_aspect
 * and integer_scale to be set independently, hence
 * separate boolean values. OpenDingux beta properly
 * groups the settings into a single scaling type
 * parameter. When supporting both firmwares, it would
 * be cleaner to refactor this function to accept one
 * enum rather than 2 booleans - but this would break
 * users' existing configs, so we maintain the old
 * format... */
static bool dingux_ipu_set_scaling_mode(bool keep_aspect, bool integer_scale);

/* Sets the image filtering method when
 * using the IPU hardware scaler */
static bool dingux_ipu_set_filter_type(enum dingux_ipu_filter_type filter_type);

#ifdef OPENDINGUX_BETA
/* Specifies all video refresh rates supported by OpenDingux Beta */
enum dingux_refresh_rate {
	DINGUX_REFRESH_RATE_60HZ = 0,
	DINGUX_REFRESH_RATE_50HZ,
	DINGUX_REFRESH_RATE_LAST
};

/* Sets the refresh rate of the integral LCD panel.
 * If specified value is invalid, will set refresh
 * rate to 60 Hz.
 * Returns a floating point representation of the
 * resultant hardware refresh rate. In the event
 * that a refresh rate cannot be set (i.e. hardware
 * error), returns 0.0 */
static float dingux_set_video_refresh_rate(enum dingux_refresh_rate refresh_rate);

/* Gets the currently set refresh rate of the
 * integral LCD panel.
 * Returns false if hardware is in an undefined
 * state. */
static bool dingux_get_video_refresh_rate(enum dingux_refresh_rate *refresh_rate);
#endif // OPENDINGUX_BETA

/* Resets the IPU hardware scaler to the default configuration */
static bool dingux_ipu_reset(void);

/* Fetches internal battery level */
static int dingux_get_battery_level(void);

/* Fetches the path of the base directory */
static void dingux_get_base_path(char *path, size_t len);

#define DINGUX_ALLOW_DOWNSCALING_FILE "/sys/devices/platform/jz-lcd.0/allow_downscaling"
#define DINGUX_KEEP_ASPECT_RATIO_FILE "/sys/devices/platform/jz-lcd.0/keep_aspect_ratio"
#define DINGUX_INTEGER_SCALING_FILE "/sys/devices/platform/jz-lcd.0/integer_scaling"
#define DINGUX_SHARPNESS_UPSCALING_FILE "/sys/devices/platform/jz-lcd.0/sharpness_upscaling"
#define DINGUX_SHARPNESS_DOWNSCALING_FILE "/sys/devices/platform/jz-lcd.0/sharpness_downscaling"
#define DINGUX_BATTERY_CAPACITY_FILE "/sys/class/power_supply/battery/capacity"

/* Base path defines */
#define DINGUX_HOME_ENVAR "HOME"
#define DINGUX_MEDIA_PATH "/media"
#define DINGUX_DEFAULT_SD_PATH "/media/mmcblk0p1"
#define DINGUX_DATA_PATH "/media/data"

/* OpenDingux Beta defines */
#define DINGUX_BATTERY_VOLTAGE_MIN "/sys/class/power_supply/jz-battery/voltage_min_design"
#define DINGUX_BATTERY_VOLTAGE_MAX "/sys/class/power_supply/jz-battery/voltage_max_design"
#define DINGUX_BATTERY_VOLTAGE_NOW "/sys/class/power_supply/jz-battery/voltage_now"
#define DINGUX_SCALING_MODE_ENVAR "VIDEO_KMSDRM_SCALING_MODE"
#define DINGUX_SCALING_SHARPNESS_ENVAR "VIDEO_KMSDRM_SCALING_SHARPNESS"
#define DINGUX_VIDEO_REFRESHRATE_ENVAR "VIDEO_REFRESHRATE"

static bool _write_file(const char *path, const char *data, size_t data_size) {
	FileAccessRef fa(FileAccess::open(path, FileAccess::READ_WRITE));
	if (fa) {
		fa->store_buffer((const uint8_t *)data, data_size);
		return true;
	}
	return false;
}

static bool _read_file(const char *path, Vector<uint8_t> &data) {
	FileAccessRef fa(FileAccess::open(path, FileAccess::READ));
	if (fa) {
		Error err;
		data = fa->get_file_as_array(path, &err);
		if (err != OK) {
			return false;
		}
		return true;
	}
	return false;
}

static PoolIntArray _kernel_version() {
	struct utsname buffer;
	if (uname(&buffer) != 0) {
		perror("uname");
		return PoolIntArray();
	}
	PoolIntArray ver;
	char *p = buffer.release;
	while (*p) {
		if (isdigit(*p)) {
			ver.push_back(strtol(p, &p, 10));
		} else {
			p++;
		}
	}
	return ver;
}

static bool _is_dingux_beta() {
	PoolIntArray ver = _kernel_version();
	return true;
}

static bool dingux_ipu_set_downscaling_enable(bool enable) {
#ifdef OPENDINGUX_BETA
	return true;
#else
	const char *path = DINGUX_ALLOW_DOWNSCALING_FILE;
	const char *enable_str = enable ? "1" : "0";
	/* Check whether file exists */
	if (!FileAccess::exists(path)) {
		return false;
	}
	/* Write enable state to file */
	return _write_file(path, enable_str, 1);
#endif // DINGUX_BETA
}

static bool dingux_ipu_set_scaling_mode(bool keep_aspect, bool integer_scale) {
#ifdef OPENDINGUX_BETA
	const char *scaling_str = "0";
	/* integer_scale takes priority */
	if (integer_scale) {
		scaling_str = "2";
	} else if (keep_aspect) {
		scaling_str = "1";
	}
	return (setenv(DINGUX_SCALING_MODE_ENVAR, scaling_str, 1) == 0);
#else
	const char *keep_aspect_path = DINGUX_KEEP_ASPECT_RATIO_FILE;
	const char *keep_aspect_str = keep_aspect ? "1" : "0";
	bool keep_aspect_success = false;

	const char *integer_scale_path = DINGUX_INTEGER_SCALING_FILE;
	const char *integer_scale_str = integer_scale ? "1" : "0";
	bool integer_scale_success = false;

	/* Set keep_aspect */
	if (FileAccess::exists(keep_aspect_path)) {
		keep_aspect_success = _write_file(keep_aspect_path, keep_aspect_str, 1);
	}
	/* Set integer_scale */
	if (FileAccess::exists(integer_scale_path)) {
		integer_scale_success = _write_file(integer_scale_path, integer_scale_str, 1);
	}
	return (keep_aspect_success && integer_scale_success);
#endif // DINGUX_BETA
}

static bool dingux_ipu_set_aspect_ratio_enable(bool enable) {
	const char *path = DINGUX_KEEP_ASPECT_RATIO_FILE;
	const char *enable_str = enable ? "1" : "0";
	/* Check whether file exists */
	if (!FileAccess::exists(path)) {
		return false;
	}
	/* Write enable state to file */
	return _write_file(path, enable_str, 1);
}

static bool dingux_ipu_set_integer_scaling_enable(bool enable) {
	const char *path = DINGUX_INTEGER_SCALING_FILE;
	const char *enable_str = enable ? "1" : "0";
	/* Check whether file exists */
	if (!FileAccess::exists(path)) {
		return false;
	}
	/* Write enable state to file */
	return _write_file(path, enable_str, 1);
}

static bool dingux_ipu_set_filter_type(enum dingux_ipu_filter_type filter_type) {
	/* Sharpness settings range is [0,32]
	 * - 0:      nearest-neighbour
	 * - 1:      bilinear
	 * - 2...32: bicubic (translating to a sharpness
	 *                    factor of -0.25..-4.0 internally)
	 * Default bicubic sharpness factor is
	 * (-0.125 * 8) = -1.0 */
#ifndef OPENDINGUX_BETA
	const char *upscaling_path = DINGUX_SHARPNESS_UPSCALING_FILE;
	const char *downscaling_path = DINGUX_SHARPNESS_DOWNSCALING_FILE;
	bool upscaling_success = false;
	bool downscaling_success = false;
#endif
	const char *sharpness_str = "8";

	/* Check filter type */
	switch (filter_type) {
		case DINGUX_IPU_FILTER_BILINEAR:
			sharpness_str = "1";
			break;
		case DINGUX_IPU_FILTER_NEAREST:
			sharpness_str = "0";
			break;
		default:
			/* sharpness_str is already set to 8 by default */
			break;
	}
#ifdef OPENDINGUX_BETA
	return (setenv(DINGUX_SCALING_SHARPNESS_ENVAR, sharpness_str, 1) == 0);
#else
	/* Set upscaling sharpness */
	if (FileAccess::exists(upscaling_path)) {
		upscaling_success = _write_file(upscaling_path, sharpness_str, 1);
	} else {
		upscaling_success = false;
	}
	/* Set downscaling sharpness */
	if (FileAccess::exists(downscaling_path)) {
		downscaling_success = _write_file(downscaling_path, sharpness_str, 1);
	} else {
		downscaling_success = false;
	}
	return (upscaling_success && downscaling_success);
#endif // OPENDINGUX_BETA
}

static int _read_battery_sys_file(const char *path) {
	Vector<uint8_t> file_buf;
	int sys_file_value = 0;
	/* Check whether file exists */
	if (!FileAccess::exists(path)) {
		goto error;
	}
	/* Read file */
	if (!_read_file(path, file_buf) || file_buf.empty()) {
		goto error;
	}
	/* Convert to integer */
	sys_file_value = atoi((const char *)file_buf.ptr());
	return sys_file_value;
error:
	return -1;
}

static int dingux_get_battery_level() {
#ifdef OPENDINGUX_BETA
	/* Taken from https://github.com/OpenDingux/gmenu2x/blob/master/src/battery.cpp
	 * No 'capacity' file in sysfs - Do a dumb approximation of the capacity
	 * using the current voltage reported and the min/max voltages of the
	 * battery */
	int voltage_min = 0;
	int voltage_max = 0;
	int voltage_now = 0;

	voltage_min = dingux_read_battery_sys_file(DINGUX_BATTERY_VOLTAGE_MIN);
	if (voltage_min < 0) {
		return -1;
	}
	voltage_max = dingux_read_battery_sys_file(DINGUX_BATTERY_VOLTAGE_MAX);
	if (voltage_max < 0) {
		return -1;
	}
	voltage_now = dingux_read_battery_sys_file(DINGUX_BATTERY_VOLTAGE_NOW);
	if (voltage_now < 0) {
		return -1;
	}
	if ((voltage_max <= voltage_min) || (voltage_now < voltage_min)) {
		return -1;
	}
	return (int)(((voltage_now - voltage_min) * 100) / (voltage_max - voltage_min));
#else
	return _read_battery_sys_file(DINGUX_BATTERY_CAPACITY_FILE);
#endif
}

// Godot access

static LinuxHw *LinuxHw::instance = nullptr;
