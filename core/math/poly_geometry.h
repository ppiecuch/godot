#ifndef POLY_GEOMETRY_H
#define POLY_GEOMETRY_H

#include "core/variant.h"

enum LineDrawMode {

	LINE_JOIN_MITTER,
	LINE_JOIN_BEVEL,
	LINE_JOIN_ROUND,
	LINE_CAP_SQUARE,
	LINE_CAP_PROJECT,
	LINE_CAP_ROUND,
};

/// full polygon stroke with line with 'p_width'
PoolVector2Array strokify(const PoolVector2Array &p_contour, real_t p_width, LineDrawMode p_line_cap, LineDrawMode p_line_join, bool p_loop, bool p_antialiased = false, bool p_allow_overlap = false);

/// extend contour with line joints only
PoolVector2Array strokify(const PoolVector2Array &p_contour, LineDrawMode p_line_join, bool p_loop, bool p_allow_overlap = false);

#endif // POLY_GEOMETRY_H
