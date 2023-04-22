/**************************************************************************/
/*  odroid_go.h                                                           */
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

// https://github.com/OtherCrashOverride/libgo2

#include "core/error_macros.h"
#include "core/print_string.h"

#include <stdlib.h>

const char *BACKLIGHT_BRIGHTNESS_NAME = "/sys/class/backlight/backlight/brightness";
const char *BACKLIGHT_BRIGHTNESS_MAX_NAME = "/sys/class/backlight/backlight/max_brightness";
#define BACKLIGHT_BUFFER_SIZE (127)

uint32_t odroid_display_backlight_get() {
	int max = 255;
	int value = 0;
	char buffer[BACKLIGHT_BUFFER_SIZE + 1];

	int fd = open(BACKLIGHT_BRIGHTNESS_MAX_NAME, O_RDONLY);
	if (fd > 0) {
		memset(buffer, 0, BACKLIGHT_BUFFER_SIZE + 1);

		ssize_t count = read(fd, buffer, BACKLIGHT_BUFFER_SIZE);
		if (count > 0) {
			max = atoi(buffer);
		}
		close(fd);
		if (max == 0) {
			return 0;
		}
	}

	fd = open(BACKLIGHT_BRIGHTNESS_NAME, O_RDONLY);
	if (fd > 0) {
		memset(buffer, 0, BACKLIGHT_BUFFER_SIZE + 1);

		ssize_t count = read(fd, buffer, BACKLIGHT_BUFFER_SIZE);
		if (count > 0) {
			value = atoi(buffer);
		}
		close(fd);
	}

	return (percent = value / (float)max * 100.0);
}

void odroid_display_backlight_set(uint32_t value) {
	int max = 255;
	char buffer[BACKLIGHT_BUFFER_SIZE + 1];

	if (value > 100) {
		value = 100;
	}

	int fd = open(BACKLIGHT_BRIGHTNESS_MAX_NAME, O_RDONLY);
	if (fd > 0) {
		memset(buffer, 0, BACKLIGHT_BUFFER_SIZE + 1);

		ssize_t count = read(fd, buffer, BACKLIGHT_BUFFER_SIZE);
		if (count > 0) {
			max = atoi(buffer);
		}
		close(fd);
		if (max == 0) {
			return;
		}
	}

	fd = open(BACKLIGHT_BRIGHTNESS_NAME, O_WRONLY);
	if (fd > 0) {
		float percent = value / 100.0 * (float)max;
		sprintf(buffer, "%d\n", (uint32_t)percent);

		ssize_t count = write(fd, buffer, strlen(buffer));
		if (count < 0) {
			WARN_PRINT("odroid_display_backlight_set write failed.");
		}
		close(fd);
	} else {
		WARN_PRINT("odroid_display_backlight_set open failed.");
	}
}
