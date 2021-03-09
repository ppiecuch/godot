#include "vaser.h"

#ifdef VASER_DEBUG
# define DEBUG printf
#else
# define DEBUG ;//
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <vector>

namespace VASEr
{
	namespace VASErin { // VASEr internal namespace

		const real_t vaser_min_alw = 0.0000001; //smallest value not regarded as zero
		const Color default_color = {0, 0, 0, 1};
		const real_t default_weight = 1;
		#include "inc/point.h"
		#include "inc/color.h"
		class vertex_array_holder;
		#include "inc/backend.h"
		#include "inc/vertex_array_holder.h"
		#include "inl/agg_curve4.cpp.inl"
	}

	#include "inl/opengl.cpp.inl"
	#include "inl/polyline.cpp.inl"
	#include "inl/gradient.cpp.inl"
	#include "inl/curve.cpp.inl"
} //namespace VASEr

#undef DEBUG
