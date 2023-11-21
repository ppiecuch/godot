#pragma once

#include "core/map.h"
#include "core/nullable.h"

#include "ddls_fwd.h"
#include "iterators/iterator_from_face_to_inner_edges.h"

class DDLS_AStar : public Reference {
	DDLSMesh mesh;

	real_t radius, radius_squared;
	real_t diameter, diameter_squared;

	Map<DDLSFace, bool> closed_faces;
	Vector<DDLSFace> sorted_opened_faces;
	Map<DDLSFace, Nullable<bool>> opened_faces;
	Map<DDLSFace, DDLSEdge> entry_edges;
	Map<DDLSFace, Point2> entry_points;
	Map<DDLSFace, real_t> score_f;
	Map<DDLSFace, real_t> score_g;
	Map<DDLSFace, real_t> score_h;
	Map<DDLSFace, DDLSFace> predecessor;

	DDLSFace _from_face;
	DDLSFace _to_face;
	DDLSFace _cur_face;

public:
	real_t get_radius() const { return radius; }
	void set_radius(real_t p_radius) {
		radius = p_radius;
		radius_squared = radius * radius;
		diameter = radius * 2;
		diameter_squared = diameter * diameter;
	}

	void set_mesh(DDLSMesh p_mesh);
	void find_path(const Point2 &p_from, const Point2 &p_to, Vector<DDLSFace> &r_result_list_faces, Vector<DDLSEdge> &p_result_list_edges);
	int sorting_faces(DDLSFace a, DDLSFace b); // faces with low distance value are at the end of the array
	bool is_walkable_by_radius(DDLSEdge p_from_edge, DDLSFace p_through_face, DDLSEdge p_to_edge);

	DDLS_AStar();
};
