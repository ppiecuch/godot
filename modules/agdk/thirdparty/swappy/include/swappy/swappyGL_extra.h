// AGDK Swappy extra stub header for compilation without vendored libraries.
// Replace with actual AGDK Swappy headers when available.

#ifndef SWAPPY_GL_EXTRA_H
#define SWAPPY_GL_EXTRA_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SwappyStats {
	uint64_t totalFrames;
} SwappyStats;

void SwappyGL_getStats(SwappyStats *stats);
void SwappyGL_clearStats();

#ifdef __cplusplus
}
#endif

#endif // SWAPPY_GL_EXTRA_H
