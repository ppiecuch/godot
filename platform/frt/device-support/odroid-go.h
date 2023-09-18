#pragma once

/// Support library for the ODROID-GO Advance

// https://github.com/OtherCrashOverride/libgo2

typedef struct {
	float x;
	float y;
} go2_thumb_t;

typedef enum {
	ButtonState_Released = 0,
	ButtonState_Pressed
} go2_button_state_t;

typedef struct {
	go2_button_state_t a;
	go2_button_state_t b;
	go2_button_state_t x;
	go2_button_state_t y;

	go2_button_state_t top_left;
	go2_button_state_t top_right;

	go2_button_state_t f1;
	go2_button_state_t f2;
	go2_button_state_t f3;
	go2_button_state_t f4;
	go2_button_state_t f5;
	go2_button_state_t f6;

} go2_gamepad_buttons_t;

typedef struct {
	go2_button_state_t up;
	go2_button_state_t down;
	go2_button_state_t left;
	go2_button_state_t right;
} go2_dpad_t;

typedef struct {
	go2_thumb_t thumb;
	go2_dpad_t dpad;
	go2_gamepad_buttons_t buttons;
} go2_gamepad_state_t;

typedef struct go2_input go2_input_t;

// v1.1 API
typedef enum {
	Go2InputFeatureFlags_None = (1 << 0),
	Go2InputFeatureFlags_Triggers = (1 << 1),
	Go2InputFeatureFlags_RightAnalog = (1 << 2),
} go2_input_feature_flags_t;

typedef enum {
	Go2InputThumbstick_Left = 0,
	Go2InputThumbstick_Right
} go2_input_thumbstick_t;

typedef enum {
	Go2InputButton_DPadUp = 0,
	Go2InputButton_DPadDown,
	Go2InputButton_DPadLeft,
	Go2InputButton_DPadRight,

	Go2InputButton_A,
	Go2InputButton_B,
	Go2InputButton_X,
	Go2InputButton_Y,

	Go2InputButton_F1,
	Go2InputButton_F2,
	Go2InputButton_F3,
	Go2InputButton_F4,
	Go2InputButton_F5,
	Go2InputButton_F6,

	Go2InputButton_TopLeft,
	Go2InputButton_TopRight,

	Go2InputButton_TriggerLeft,
	Go2InputButton_TriggerRight
} go2_input_button_t;

typedef enum {
	Go2HardwareRevision_Unknown = 0,
	Go2HardwareRevision_V1_0,
	Go2HardwareRevision_V1_1
} go2_hardware_revision_t;

typedef struct go2_input_state go2_input_state_t;

#define GO2_ADC0_PATH "/sys/devices/platform/ff288000.saradc/iio:device0/in_voltage0_raw"
#define GO2_ADC0_VALUE_MAX (1024)

go2_hardware_revision_t go2_hardware_revision_get() {
	go2_hardware_revision_t result = Go2HardwareRevision_Unknown;

	// /sys/devices/platform/ff288000.saradc/iio:device0# cat in_voltage0_raw
	// 675
	// check_range(655, 695, hwrev_adc)

	int fd = open(GO2_ADC0_PATH, O_RDONLY);
	if (fd > 0) {
		char buffer[GO2_ADC0_VALUE_MAX];
		memset(buffer, 0, GO2_ADC0_VALUE_MAX);

		int value;
		ssize_t count = read(fd, buffer, GO2_ADC0_VALUE_MAX);
		if (count > 0) {
			value = atoi(buffer);

			if (check_range(value, 655, 695)) {
				result = Go2HardwareRevision_V1_1;
			} else if (value < 655) {
				result = Go2HardwareRevision_V1_0;
			}
		}
		close(fd);
	}
	return result;
}
