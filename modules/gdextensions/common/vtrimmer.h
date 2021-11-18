#ifndef vtrimmer_h
#define vtrimmer_h

#include "core/int_types.h"
#include "core/image.h"

// vertex_trimmer <imagefile> <threshold 0-255> <vertex_count 3-8> [options]
//
// Option | Type   | Default | Description
// -------+--------+---------+------------------------------------------------------------
//  -ax   | int    |    1    | Atlas tile count in x
//  -ay   | int    |    1    | Atlas tile count in y
//  -sx   | real_t |   1.0   | Scale factor of final x-coordinates
//  -sy   | real_t |   1.0   | Scale factor of final y-coordinates
//  -bx   | real_t |   0.0   | Bias factor of final x-coordinate
//  -by   | real_t |   0.0   | Bias factor of final y-coordinate
//  -h    | int    |   50    | Maximum convex hull size to reduce to before optimization
//  -sp   | int    |   16    | Sub-pixel division when generating the convex hull
//  -d    | int    |    0    | Number of times to initially dilate the image
//  -i    | int[n] |  NULL   | Index buffer. Used to optimize vertex ordering if provided.
// -------+--------+---------+------------------------------------------------------------

extern struct vertex_trimmer_opt_t {
	unsigned vertex_count;
	unsigned treshold;
	struct { real_t x,y; } scale;
	struct { real_t x,y; } bias;
	unsigned atlas_x;
	unsigned atlas_y;
	unsigned max_hull_size;
	int sub_pixel;
	unsigned dilate_count;
	unsigned indices[(8 - 2) * 3];
	bool use_index_buffer;
} vertex_trimmer_default_opt;

bool vertex_trimmer(Ref<Image> &image, vertex_trimmer_opt_t *opt);

#endif // vtrimmer_h
