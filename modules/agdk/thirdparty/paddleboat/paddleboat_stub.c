// Paddleboat stub implementations for linking when actual AGDK libraries are not available.
// Replace with actual AGDK Paddleboat static libraries when available.

#include "include/paddleboat/paddleboat.h"
#include <string.h>

Paddleboat_ErrorCode Paddleboat_init(JNIEnv *env, jobject jcontext) { return PADDLEBOAT_NO_ERROR; }
void Paddleboat_destroy(JNIEnv *env) {}
bool Paddleboat_isInitialized() { return false; }
void Paddleboat_update(JNIEnv *env) {}
void Paddleboat_onStop(JNIEnv *env) {}
void Paddleboat_onStart(JNIEnv *env) {}

void Paddleboat_setControllerStatusCallback(
		Paddleboat_ControllerStatusCallback statusCallback,
		void *userData) {}

Paddleboat_ControllerStatus Paddleboat_getControllerStatus(const int32_t controllerIndex) {
	return PADDLEBOAT_CONTROLLER_INACTIVE;
}

Paddleboat_ErrorCode Paddleboat_getControllerName(
		const int32_t controllerIndex,
		const size_t bufferSize,
		char *controllerName) {
	if (controllerName && bufferSize > 0) {
		controllerName[0] = '\0';
	}
	return PADDLEBOAT_NO_ERROR;
}

Paddleboat_ErrorCode Paddleboat_getControllerInfo(
		const int32_t controllerIndex,
		Paddleboat_Controller_Info *controllerInfo) {
	if (controllerInfo) {
		memset(controllerInfo, 0, sizeof(Paddleboat_Controller_Info));
	}
	return PADDLEBOAT_NO_ERROR;
}

Paddleboat_ErrorCode Paddleboat_getControllerData(
		const int32_t controllerIndex,
		Paddleboat_Controller_Data *controllerData) {
	if (controllerData) {
		memset(controllerData, 0, sizeof(Paddleboat_Controller_Data));
	}
	return PADDLEBOAT_NO_ERROR;
}

int32_t Paddleboat_processInputEvent(const void *event) { return 0; }
uint64_t Paddleboat_getActiveAxisMask() { return 0; }
