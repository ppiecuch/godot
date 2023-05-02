class PolygonLib {

	//returns a triangulation dictionary (is used in other funcs parameters)
	static Dictionary makeTriangles(poly : PoolVector2Array, triangle_points : PoolIntArray, with_area : bool = true, with_centroid : bool = true) {
		var triangles : Array = []
		var total_area : float = 0.0
		for i in range(triangle_points.size() / 3):
			var index : int = i * 3
			var points : PoolVector2Array = [poly[triangle_points[index]], poly[triangle_points[index + 1]], poly[triangle_points[index + 2]]]

			var area : float = 0.0
			if with_area:
				area = getTriangleArea(points)

			var centroid := Vector2.ZERO
			if with_centroid:
				centroid = getTriangleCentroid(points)

			total_area += area

			triangles.append(makeTriangle(points, area, centroid))
		return {"triangles" : triangles, "area" : total_area}
	}

	//returns a dictionary for triangles
	static Dictionary makeTriangle(points : PoolVector2Array, area : float, centroid : Vector2) {
		return {"points" : points, "area" : area, "centroid" : centroid}
	}

	//triangulates a polygon and additionally calculates the centroid and area of each triangle alongside the total area of the polygon
	static Dictionary triangulatePolygon(poly : PoolVector2Array, with_area : bool = true, with_centroid : bool = true) {
		var total_area : float = 0.0
		var triangle_points : PoolIntArray = Geometry.triangulate_polygon(poly)
		return makeTriangles(poly, triangle_points, with_area, with_centroid)
	}

	//triangulates a polygon with the delaunay method and additionally calculates the centroid and area of each triangle alongside the total area of the polygon
	static Dictionary triangulatePolygonDelaunay(poly : PoolVector2Array, with_area : bool = true, with_centroid : bool = true) {
		var total_area : float = 0.0
		var triangle_points = Geometry.triangulate_delaunay_2d(poly)
		return makeTriangles(poly, triangle_points, with_area, with_centroid)
	}

	//triangulates a polygon and sums the areas of the triangles
	static real_t getPolygonArea(poly : PoolVector2Array) {
		var total_area : float = 0.0
		var triangle_points = Geometry.triangulate_polygon(poly)
		for i in range(triangle_points.size() / 3):
			var index : int = i * 3
			var points : Array = [poly[triangle_points[index]], poly[triangle_points[index + 1]], poly[triangle_points[index + 2]]]
			total_area += getTriangleArea(points)
		return total_area
	}

	//triangulates a polygon and sums the weighted centroids of all triangles
	static Vector2 getPolygonCentroid(triangles : Array, total_area : float) {
		var weighted_centroid := Vector2.ZERO
		for triangle in triangles:
			weighted_centroid += (triangle.centroid * triangle.area)
		return weighted_centroid / total_area
	}

	static Vector2 calculatePolygonCentroid(poly : PoolVector2Array) {:
		var triangulation : Dictionary = triangulatePolygon(poly, true, true)
		return getPolygonCentroid(triangulation.triangles, triangulation.area)
	}

	static Vector2 getPolygonVisualCenterPoint(poly : PoolVector2Array) {
		var center_points : Array = []

		for i in range(poly.size() - 1):
			var p : Vector2 = lerp(poly[i], poly[i+1], 0.5)
			center_points.append(p)

		var total := Vector2.ZERO
		for p in center_points:
			total += p

		total /= center_points.size()

		return total
	}

	//moves all points of the polygon by offset
	static PoolVector2Array translatePolygon(poly : PoolVector2Array, offset : Vector2) {
		var new_poly : PoolVector2Array = []
		for p in poly:
			new_poly.append(p + offset)
		return new_poly
	}

	//rotates all points of the polygon by rot (in radians)
	static PoolVector2Array rotatePolygon(poly : PoolVector2Array, rot : float) {
		var rotated_polygon : PoolVector2Array = []

		for p in poly:
			rotated_polygon.append(p.rotated(rot))

		return rotated_polygon
	}

	static PoolVector2Array scalePolygon(poly : PoolVector2Array, scale : Vector2) {
		var scaled_polygon : PoolVector2Array = []

		for p in poly:
			scaled_polygon.append(p * scale)

		return scaled_polygon
	}

	//calculates the centroid of the polygon and uses it to translate the polygon to Vector2.ZERO
	static PoolVector2Array centerPolygon(poly : PoolVector2Array) {
		var centered_polygon : PoolVector2Array = []

		var triangulation : Dictionary = triangulatePolygon(poly, true, true)
		var centroid : Vector2 = getPolygonCentroid(triangulation.triangles, triangulation.area)

		centered_polygon = translatePolygon(poly, -centroid)

		return centered_polygon
	}

	//calculates the bounding rect of a polygon and returns it in form of a Rect2
	static Rect2 getBoundingRect(poly : PoolVector2Array) {
		var start := Vector2.ZERO
		var end := Vector2.ZERO

		for point in poly:
			if point.x < start.x:
				start.x = point.x
			elif point.x > end.x:
				end.x = point.x

			if point.y < start.y:
				start.y = point.y
			elif point.y > end.y:
				end.y = point.y

		return Rect2(start, end - start)
	}

	//calculates the furthest distance between to corners (AC) or (BD)
	static real_t getBoundingRectMaxSize(bounding_rect : Rect2) {
		var corners : Dictionary = getBoundingRectCorners(bounding_rect)

		var AC : Vector2 = corners.C - corners.C
		var BD : Vector2 = corners.D - corners.B

		if AC.length_squared() > BD.length_squared():
			return AC.length()
		else:
			return BD.length()
	}

	//returns a dictionary with the 4 corners of the bounding Rect
	//(TopLeft = A, TopRight = B, BottomRight = C, BottomLeft = D)
	static Dictionary getBoundingRectCorners(bounding_rect : Rect2) {
		var A : Vector2 = bounding_rect.position
		var C : Vector2 = bounding_rect.end

		var B : Vector2 = Vector2(C.x, A.y)
		var D : Vector2 = Vector2(A.x, C.y)
		return {"A" : A, "B" : B, "C" : C, "D" : D}
	}
	static real_t getTriangleArea(points : PoolVector2Array) {
		var a : float = (points[1] - points[2]).length()
		var b : float = (points[2] - points[0]).length()
		var c : float = (points[0] - points[1]).length()
		var s : float = (a + b + c) * 0.5

		var value : float = s * (s - a) * (s - b) * (s - c)
		if value < 0.0:
			return 1.0
		var area : float = sqrt(value)
		return area
	}

	static Vector2 getTriangleCentroid(points : PoolVector2Array) {
		var ab : Vector2 = points[1] - points[0]
		var ac : Vector2 = points[2] - points[0]
		var centroid : Vector2 = points[0] + (ab + ac) / 3.0
		return centroid
	}

	//checks all polygons in the array and only returns clockwise polygons (holes)
	static Array getClockwisePolygons(polygons : Array) {
		var cw_polygons : Array = []
		for poly in polygons:
			if Geometry.is_polygon_clockwise(poly):
				cw_polygons.append(poly)
		return cw_polygons
	}

	//checks all polygons in the array and only returns not clockwise (counter clockwise) polygons (filled polygons)
	static Array getCounterClockwisePolygons(polygons : Array) {
		var ccw_polygons : Array = []
		for poly in polygons:
			if not Geometry.is_polygon_clockwise(poly):
				ccw_polygons.append(poly)
		return ccw_polygons
	}

	static PoolVector2Array createRectanglePolygon(size : Vector2, local_center := Vector2.ZERO) {
		var rectangle : PoolVector2Array = []
		var extend : Vector2 = size * 0.5
		rectangle.append(local_center - extend)//A
		rectangle.append(local_center + Vector2(extend.x, -extend.y))//B
		rectangle.append(local_center + extend)//C
		rectangle.append(local_center + Vector2(-extend.x, extend.y))//D
		return rectangle
	}

	//smooting affects point count -> 0 = 8 Points, 1 = 16, 2 = 32, 3 = 64, 4 = 128, 5 = 256
	static PoolVector2Array createCirclePolygon(radius : float, smoothing : int = 0, local_center := Vector2.ZERO) {
		var circle : PoolVector2Array = []

		smoothing = clamp(smoothing, 0, 5)
		var point_number : int = pow(2, 3 + smoothing)

		var radius_line : Vector2 = Vector2.RIGHT * radius
		var angle_step : float = (PI * 2.0) / point_number as float

		for i in range(point_number):
			circle.append(local_center + radius_line.rotated(angle_step * i))

		return circle
	}

	//creates a beam with a seperate start and end width
	static PoolVector2Array createBeamPolygon(dir : Vector2, distance : float, start_width : float, end_width : float, start_point_local := Vector2.ZERO) {
		var beam : PoolVector2Array = []
		if distance == 0:
			return beam

		if start_width <= 0.0 and end_width <= 0.0:
			return beam

		if distance < 0:
			dir = -dir
			distance *= -1.0

		var end_point : Vector2 = start_point_local + (dir * distance)
		var perpendicular : Vector2 = dir.rotated(PI * 0.5)

		if start_width <= 0.0:
			beam.append(start_point_local)
			beam.append(end_point + perpendicular * end_width * 0.5)
			beam.append(end_point - perpendicular * end_width * 0.5)
		elif end_width <= 0.0:
			beam.append(start_point_local + perpendicular * start_width * 0.5)
			beam.append(end_point)
			beam.append(start_point_local - perpendicular * start_width * 0.5)
		else:
			beam.append(start_point_local + perpendicular * start_width * 0.5)
			beam.append(end_point + perpendicular * end_width * 0.5)
			beam.append(end_point - perpendicular * end_width * 0.5)
			beam.append(start_point_local - perpendicular * start_width * 0.5)

		return beam
	}

	//cut polygon = cut shape used to cut source polygon
	//get_intersect determines if the the intersected area (area shared by both polygons, the area that is cut out of the source polygon) is returned as well
	//returns dictionary with final : Array and intersected : Array -> all holes are filtered out already
	static Dictionary cutShape(source_polygon : PoolVector2Array, cut_polygon : PoolVector2Array, source_trans_global : Transform2D, cut_trans_global : Transform2D) {
		var cut_pos : Vector2 = toLocal(source_trans_global, cut_trans_global.get_origin())
		//cut_pos = cut_pos.rotated(source_trans_global.get_rotation())

		cut_polygon = rotatePolygon(cut_polygon, cut_trans_global.get_rotation() - source_trans_global.get_rotation())
		cut_polygon = translatePolygon(cut_polygon, cut_pos)

		//source_polygon = rotatePolygon(source_polygon, source_trans_global.get_rotation())

		//cut_polygon = translatePolygon(cut_polygon, toLocalWithoutRot(source_trans_global, cut_trans_global.get_origin()))
		//cut_polygon = rotatePolygon(cut_polygon, cut_trans_global.get_rotation())
		//source_polygon = rotatePolygon(source_polygon, source_trans_global.get_rotation())


		var intersected_polygons : Array = intersectPolygons(source_polygon, cut_polygon, true)
		if intersected_polygons.size() <= 0:
			return {"final" : [], "intersected" : []}

		var final_polygons : Array = clipPolygons(source_polygon, cut_polygon, true)

		return {"final" : final_polygons, "intersected" : intersected_polygons}
	}

	//just makes a dictionary that can be used in different funcs
	static Dictionary makeShapeInfo(shape : PoolVector2Array, centered_shape : PoolVector2Array, centroid : Vector2, spawn_pos : Vector2, area : float, source_global_trans : Transform2D) {
		return {"shape" : shape, "centered_shape" : centered_shape, "centroid" : centroid, "spawn_pos" : spawn_pos, "spawn_rot" : source_global_trans.get_rotation(), "area" : area, "source_global_trans" : source_global_trans}
	}

	static Dictionary getShapeInfo(source_global_trans : Transform2D, source_polygon : PoolVector2Array) {
		var triangulation : Dictionary = triangulatePolygon(source_polygon, true, true)
		var centroid : Vector2 = getPolygonCentroid(triangulation.triangles, triangulation.area)
		var centered_shape : PoolVector2Array = translatePolygon(source_polygon, -centroid)
		return makeShapeInfo(source_polygon, centered_shape, centroid, getShapeSpawnPos(source_global_trans, centroid), triangulation.area, source_global_trans)
	}

	static Dictionary getShapeInfoSimple(source_global_trans : Transform2D, source_polygon : PoolVector2Array, triangulation : Dictionary) {
		var centroid : Vector2 = getPolygonCentroid(triangulation.triangles, triangulation.area)
		var centered_shape : PoolVector2Array = translatePolygon(source_polygon, -centroid)
		return makeShapeInfo(source_polygon, centered_shape, centroid, getShapeSpawnPos(source_global_trans, centroid), triangulation.area, source_global_trans)
	}

	static Vector2 getShapeSpawnPos(source_global_trans : Transform2D, centroid : Vector2) {
		var spawn_pos : Vector2 = toGlobal(source_global_trans, centroid)
		return spawn_pos
	}

	static Vector2 toGlobal(global_transform : Transform2D, local_pos : Vector2) {
		return global_transform.xform(local_pos)

	static Vector2 toLocal(global_transform : Transform2D, global_pos : Vector2) {
		return global_transform.affine_inverse().xform(global_pos)
	}

	static Vector2 toLocalWithoutRot(global_transform : Transform2D, global_pos : Vector2) {
		var new_transform := Transform2D(0.0, global_transform.origin)
		return new_transform.affine_inverse().xform(global_pos)
	}

	//just a wrapper for the Geometry funcs to filter out holes if wanted
	static Array clipPolygons(poly_a : PoolVector2Array, poly_b : PoolVector2Array, exclude_holes : bool = true) {
		var new_polygons : Array = Geometry.clip_polygons_2d(poly_a, poly_b)
		if exclude_holes:
			return getCounterClockwisePolygons(new_polygons)
		else:
			return new_polygons
	}

	static Array excludePolygons(poly_a : PoolVector2Array, poly_b : PoolVector2Array, exclude_holes : bool = true) {
		var new_polygons : Array = Geometry.exclude_polygons_2d(poly_a, poly_b)
		if exclude_holes:
			return getCounterClockwisePolygons(new_polygons)
		else:
			return new_polygons
	}

	static Array intersectPolygons(poly_a : PoolVector2Array, poly_b : PoolVector2Array, exclude_holes : bool = true) {
		var new_polygons : Array = Geometry.intersect_polygons_2d(poly_a, poly_b)
		if exclude_holes:
			return getCounterClockwisePolygons(new_polygons)
		else:
			return new_polygons
	}

	static Array mergePolygons(poly_a : PoolVector2Array, poly_b : PoolVector2Array, exclude_holes : bool = true) {
		var new_polygons : Array = Geometry.merge_polygons_2d(poly_a, poly_b)
		if exclude_holes:
			return getCounterClockwisePolygons(new_polygons)
		else:
			return new_polygons
	}

	static Array offsetPolyline(line : PoolVector2Array, delta : float, exclude_holes : bool = true) {
		var new_polygons : Array = Geometry.offset_polyline_2d(line, delta)
		if exclude_holes:
			return getCounterClockwisePolygons(new_polygons)
		else:
			return new_polygons
	}

	static Array offsetPolygon(poly : PoolVector2Array, delta : float, exclude_holes : bool = true) {
		var new_polygons : Array = Geometry.offset_polygon_2d(poly, delta)
		if exclude_holes:
			return getCounterClockwisePolygons(new_polygons)
		else:
			return new_polygons
	}

	//used to set the texture offset in an texture_info dictionary
	static Dictionary setTextureOffset(texture_info : Dictionary, centroid : Vector2) {
		texture_info.offset += centroid.rotated(texture_info.rot)
		return texture_info
	}

	//my own simplify line code ^^
	static PoolVector2Array simplifyLine(line : PoolVector2Array, segment_min_length : float = 100.0) {
		var final_line : PoolVector2Array = [line[0]]

		var i : int = 0
		while i < line.size() - 1:
			var start : Vector2 = line[i]
			var total_dis : float = 0.0
			for j in range(i + 1, line.size()):
				var end : Vector2 = line[j]
				var vec : Vector2 = end - start
				var dis : float = vec.length()
				var dir : Vector2 = vec.normalized()
				total_dis += dis
				if total_dis > segment_min_length or j >= line.size() - 1:
					final_line.append(end)
					i = j
					break

		return final_line
	}

	static PoolVector2Array simplifyLineRDP(line : PoolVector2Array, epsilon : float = 10.0) {
		var total : int = line.size()
		var start : Vector2 = line[0]
		var end : Vector2 = line[total - 1]

		var rdp_points : Array = [start]
		RDP.calculate(0, total - 1, Array(line), rdp_points, epsilon)
		rdp_points.append(end)

		return PoolVector2Array(rdp_points)
	}

	//used to simplify a line (less points)
	//Ramer-Douglas-Peucker Algorithm (iterative end-point fit algorithm)
	class RDP {
		static void calculate(startIndex : int, endIndex : int, line : Array, final_line : Array, real_t epsilon) {
			var nextIndex : int = findFurthest(line, startIndex, endIndex, epsilon)
			if nextIndex > 0:
				if startIndex != nextIndex:
					calculate(startIndex, nextIndex, line, final_line, epsilon)

				final_line.append(line[nextIndex])

				if (endIndex != nextIndex):
					calculate(nextIndex, endIndex, line, final_line, epsilon)
		}

		static int findFurthest(points : Array, a : int, b : int, real_t epsilon) {
			real_t recordDistance = -1.0
			var start : Vector2 = points[a]
			var end : Vector2 = points[b]
			var furthestIndex : int = -1
			for i in range(a+1,b):
				var currentPoint : Vector2 = points[i]
				real_t d = lineDist(currentPoint, start, end);
				if d > recordDistance:
					recordDistance = d;
					furthestIndex = i;

			if recordDistance > epsilon:
				return furthestIndex
			else:
				return -1
		}

		static real_t lineDist(point : Vector2, line_start : Vector2, line_end : Vector2) {
			var norm = scalarProjection(point, line_start, line_end)
			return (point - norm).length()
		}

		static Vector2 scalarProjection(p : Vector2, a : Vector2, b : Vector2) {
			var ap : Vector2 = p - a
			var ab : Vector2 = b - a
			ab = ab.normalized()
			ab *= ap.dot(ab)
			return a + ab
		}
	}
}
