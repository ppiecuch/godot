#pragma once

#include "ddls_fwd.h"

#include "core/image.h"
#include "core/vector.h"

namespace DDLSPotrace {
	Vector<Vector<real_t>> build_shapes(Ref<Image> p_bmp_data, Ref<Image> p_debug_bmp = nullptr, Shape debug_shape = nullptr);
	Vector<real_t> build_shape(Ref<Image> bmpData, int fromPixelRow, int fromPixelCol, Dictionary dictPixelsDone, Ref<Image> debugBmp = nullptr, Shape debugShape = nullptr);
	DDLSGraph build_graph(Vector<real_t> shape);
	Vector<real_t> build_polygon(DDLSGraph graph, Shape debugShape = nullptr);
} // DDLSPotrace
