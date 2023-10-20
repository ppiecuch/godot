#ifndef LIBMAP_BRUSH_H
#define LIBMAP_BRUSH_H

#include "libmap/vector.h"

#include <stdlib.h>

struct LMFace;

struct LMBrush {
	int face_count = 0;
	LMFace *faces = nullptr;
	vec3 center;
};

#endif // LIBMAP_BRUSH_H
