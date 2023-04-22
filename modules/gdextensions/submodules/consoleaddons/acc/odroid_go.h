// https://github.com/OtherCrashOverride/libgo2

#include "core/error_macros.h"
#include "core/print_string.h"

#include <stdlib.h>

const char* BACKLIGHT_BRIGHTNESS_NAME = "/sys/class/backlight/backlight/brightness";
const char* BACKLIGHT_BRIGHTNESS_MAX_NAME = "/sys/class/backlight/backlight/max_brightness";
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
		if (max == 0) return 0;
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

	if (value > 100) value = 100;

	int fd = open(BACKLIGHT_BRIGHTNESS_MAX_NAME, O_RDONLY);
	if (fd > 0) {
		memset(buffer, 0, BACKLIGHT_BUFFER_SIZE + 1);

		ssize_t count = read(fd, buffer, BACKLIGHT_BUFFER_SIZE);
		if (count > 0) {
			max = atoi(buffer);
		}
		close(fd);
		if (max == 0) return;
	}

	fd = open(BACKLIGHT_BRIGHTNESS_NAME, O_WRONLY);
	if (fd > 0) {
		float percent = value / 100.0 * (float)max;
		sprintf(buffer, "%d\n", (uint32_t)percent);

		//printf("backlight=%d, max=%d\n", (uint32_t)percent, max);

		ssize_t count = write(fd, buffer, strlen(buffer));
		if (count < 0) {
			printf("odroid_display_backlight_set write failed.\n");
		}
		close(fd);
	} else {
		printf("odroid_display_backlight_set open failed.\n");
	}
}
