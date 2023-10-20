#ifndef LIBMAP_ENTITY_GEOMETRY_H
#define LIBMAP_ENTITY_GEOMETRY_H

#include "libmap/vector.h"

#include <stdlib.h>

typedef struct LMVertexUV {
	double u;
	double v;
} LMVertexUV;

typedef struct LMVertexTangent {
	double x;
	double y;
	double z;
	double w;
} LMVertexTangent;

typedef struct LMFaceVertex {
	vec3 vertex;
	vec3 normal;
	LMVertexUV uv;
	LMVertexTangent tangent;
} LMFaceVertex;

typedef struct LMFaceGeometry {
	int vertex_count = 0;
	LMFaceVertex *vertices = nullptr;
	int index_count = 0;
	int *indices = nullptr;
} LMFaceGeometry;

typedef struct LMBrushGeometry {
	LMFaceGeometry *faces = nullptr;
} LMBrushGeometry;

typedef struct LMEntityGeometry {
	LMBrushGeometry *brushes = nullptr;
} LMEntityGeometry;

#endif // LIBMAP_ENTITY_GEOMETRY_H
