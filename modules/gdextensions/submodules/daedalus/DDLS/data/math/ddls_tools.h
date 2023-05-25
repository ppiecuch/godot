#include "core/vector.h"
#include "core/math/vector2.h"

namespace DDLSTools {
	void extract_mesh_from_bitmap(Ref<Image> bmp_data, Vector<Point2> &vertices, Vector<int> &triangles);
	// Simplify polyline (Ramer-Douglas-Peucker) from array of coords defining the polyline.
	// Epsilon is a perpendicular distance threshold (typically in the range [1..2]).
	Vector<real_t> simplify(Vector<real_t> p_coords, real_t p_epsilon = 1);
} // namespace DDLSTools
