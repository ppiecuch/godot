#ifndef __L3DSLIB__
#define __L3DSLIB__

struct triangle_t;

// l3dslib.h: header file for loading triangles from a 3DS triangle file
void Load3DSTriangleList (char *filename, triangle_t **pptri, int *numtriangles);

#endif // __L3DSLIB__
