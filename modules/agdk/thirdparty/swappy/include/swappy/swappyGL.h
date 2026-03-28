// AGDK Swappy stub header for compilation without vendored libraries.
// Replace with actual AGDK Swappy headers when available.

#ifndef SWAPPY_GL_H
#define SWAPPY_GL_H

#include <EGL/egl.h>
#include <jni.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void SwappyGL_init(JNIEnv *env, jobject jactivity);
void SwappyGL_destroy();
bool SwappyGL_swap(EGLDisplay display, EGLSurface surface);
void SwappyGL_setSwapIntervalNS(uint64_t swap_ns);
void SwappyGL_setAutoSwapInterval(bool enabled);
void SwappyGL_setAutoPipelineMode(bool enabled);
bool SwappyGL_isEnabled();
void SwappyGL_setWindow(struct ANativeWindow *window);
void SwappyGL_setFenceTimeoutNS(uint64_t fence_timeout_ns);
void SwappyGL_setUseAffinity(bool tf);
void SwappyGL_setMaxAutoSwapIntervalNS(uint64_t max_swap_ns);

#define SWAPPY_SWAP_60FPS (16666667ULL)
#define SWAPPY_SWAP_30FPS (33333333ULL)
#define SWAPPY_SWAP_20FPS (50000000ULL)

#ifdef __cplusplus
}
#endif

#endif // SWAPPY_GL_H
