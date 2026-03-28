// AGDK Paddleboat stub header for compilation without vendored libraries.
// Replace with actual AGDK Paddleboat headers when available.

#ifndef PADDLEBOAT_H
#define PADDLEBOAT_H

#include <jni.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PADDLEBOAT_MAX_CONTROLLERS 8

typedef int32_t Paddleboat_ErrorCode;

#define PADDLEBOAT_NO_ERROR 0

typedef enum Paddleboat_ControllerStatus {
	PADDLEBOAT_CONTROLLER_INACTIVE = 0,
	PADDLEBOAT_CONTROLLER_ACTIVE = 1,
	PADDLEBOAT_CONTROLLER_JUST_CONNECTED = 2,
	PADDLEBOAT_CONTROLLER_JUST_DISCONNECTED = 3,
} Paddleboat_ControllerStatus;

typedef enum Paddleboat_ControllerButtonLayout {
	PADDLEBOAT_CONTROLLER_LAYOUT_STANDARD = 0,
	PADDLEBOAT_CONTROLLER_LAYOUT_SHAPES = 1,
	PADDLEBOAT_CONTROLLER_LAYOUT_REVERSE = 2,
	PADDLEBOAT_CONTROLLER_LAYOUT_ARCADE_STICK = 3,
	PADDLEBOAT_CONTROLLER_LAYOUT_MASK = 3,
} Paddleboat_ControllerButtonLayout;

#define PADDLEBOAT_CONTROLLER_FLAG_TOUCHPAD 0x10
#define PADDLEBOAT_CONTROLLER_FLAG_VIRTUAL_MOUSE 0x20

typedef struct Paddleboat_Controller_Thumbstick_Precision {
	float stickFlatX;
	float stickFlatY;
	float stickFuzzX;
	float stickFuzzY;
} Paddleboat_Controller_Thumbstick_Precision;

typedef struct Paddleboat_Controller_Info {
	uint32_t controllerFlags;
	int32_t controllerNumber;
	int32_t vendorId;
	int32_t productId;
	int32_t deviceId;
	Paddleboat_Controller_Thumbstick_Precision leftStickPrecision;
	Paddleboat_Controller_Thumbstick_Precision rightStickPrecision;
} Paddleboat_Controller_Info;

typedef struct Paddleboat_Controller_Thumbstick {
	float stickX;
	float stickY;
} Paddleboat_Controller_Thumbstick;

typedef struct Paddleboat_Controller_Pointer {
	float pointerX;
	float pointerY;
} Paddleboat_Controller_Pointer;

typedef struct Paddleboat_Controller_Data {
	uint64_t timestamp;
	uint32_t buttonsDown;
	Paddleboat_Controller_Thumbstick leftStick;
	Paddleboat_Controller_Thumbstick rightStick;
	float triggerL1;
	float triggerL2;
	float triggerR1;
	float triggerR2;
	Paddleboat_Controller_Pointer virtualPointer;
} Paddleboat_Controller_Data;

typedef void (*Paddleboat_ControllerStatusCallback)(
		const int32_t controllerIndex,
		const Paddleboat_ControllerStatus controllerStatus,
		void *userData);

Paddleboat_ErrorCode Paddleboat_init(JNIEnv *env, jobject jcontext);
void Paddleboat_destroy(JNIEnv *env);
bool Paddleboat_isInitialized();
void Paddleboat_update(JNIEnv *env);
void Paddleboat_onStop(JNIEnv *env);
void Paddleboat_onStart(JNIEnv *env);

void Paddleboat_setControllerStatusCallback(
		Paddleboat_ControllerStatusCallback statusCallback,
		void *userData);

Paddleboat_ControllerStatus Paddleboat_getControllerStatus(const int32_t controllerIndex);

Paddleboat_ErrorCode Paddleboat_getControllerName(
		const int32_t controllerIndex,
		const size_t bufferSize,
		char *controllerName);

Paddleboat_ErrorCode Paddleboat_getControllerInfo(
		const int32_t controllerIndex,
		Paddleboat_Controller_Info *controllerInfo);

Paddleboat_ErrorCode Paddleboat_getControllerData(
		const int32_t controllerIndex,
		Paddleboat_Controller_Data *controllerData);

int32_t Paddleboat_processInputEvent(const void *event);
uint64_t Paddleboat_getActiveAxisMask();

#ifdef __cplusplus
}
#endif

#endif // PADDLEBOAT_H
