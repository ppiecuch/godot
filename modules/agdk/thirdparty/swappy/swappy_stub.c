// Swappy stub implementations for linking when actual AGDK libraries are not available.
// Replace with actual AGDK Swappy static libraries when available.

#include "include/swappy/swappyGL.h"
#include "include/swappy/swappyGL_extra.h"

void SwappyGL_init(JNIEnv *env, jobject jactivity) {}
void SwappyGL_destroy() {}
bool SwappyGL_swap(EGLDisplay display, EGLSurface surface) { return false; }
void SwappyGL_setSwapIntervalNS(uint64_t swap_ns) {}
void SwappyGL_setAutoSwapInterval(bool enabled) {}
void SwappyGL_setAutoPipelineMode(bool enabled) {}
bool SwappyGL_isEnabled() { return false; }
void SwappyGL_setWindow(struct ANativeWindow *window) {}
void SwappyGL_setFenceTimeoutNS(uint64_t fence_timeout_ns) {}
void SwappyGL_setUseAffinity(bool tf) {}
void SwappyGL_setMaxAutoSwapIntervalNS(uint64_t max_swap_ns) {}
void SwappyGL_getStats(SwappyStats *stats) {
	if (stats) {
		stats->totalFrames = 0;
	}
}
void SwappyGL_clearStats() {}
