#ifndef __TRILIB__
#define __TRILIB__

// trilib.h: header file for loading triangles from an Alias triangle file
#define MAXTRIANGLES 2048

typedef struct {
	vec3_t verts[3];
} triangle_t;

void LoadTriangleList (char *filename, triangle_t **pptri, int *numtriangles);

#endif // __TRILIB__
