#pragma once

#include "core/vector.h"
#include "core/reference.h"
#include "common/gd_core.h"

#include "ddls_fwd.h"

class DDLS_Mesh : public Reference {
	unsigned id;

	real_t width;
	real_t height;
	bool clipping;

	Vector<DDLSVertex> vertices;
	Vector<DDLSEdge> edges;
	Vector<DDLSFace> faces;
	Vector<DDLSConstraintShape> constraint_shapes;
	Vector<DDLSObject> objects;

	// keep references of center vertex and bounding edges when split, useful to restore edges as Delaunay
	DDLSVertex _center_vertex;
	Vector<DDLSEdge> _edges_to_check;

	bool _objects_update_in_progress;

public:
	unsigned get_id() const { return id; }

	real_t get_height() const { return height; }
	real_t get_width() const { return width; }

	bool get_clipping() const { return clipping; }
	void set_clipping(bool value) { clipping = value; }

	Vector<DDLSVertex> get_vertices() const;
	Vector<DDLSEdge> get_edges() const;
	Vector<DDLSFace> get_faces() const;
	Vector<DDLSConstraintShape> get_constraint_shapes() const;

	void build_from_record(const String &rec) {
		const Vector<String> positions = rec.split(";");
		for (int i = 0; i < positions.size(); i+=4) {
			insert_constraint_segment(positions[i].to_float(), positions[i+1].to_float(), positions[i+2].to_float(), positions[i+3].to_float());
		}
	}

	void insert_object(DDLSObject p_object);
	void delete_object(DDLSObject p_object);
	void update_objects();

	// insert a new collection of constrained edges.
	// Coordinates parameter is a list with form [x0, y0, x1, y1, x2, y2, x3, y3, x4, y4, ....]
	// where each 4-uple sequence (xi, yi, xi+1, yi+1) is a constraint segment (with i % 4 == 0)
	// and where each couple sequence (xi, yi) is a point.
	// Segments are not necessary connected.
	// Segments can overlap (then they will be automaticaly subdivided).
	DDLSConstraintShape insert_constraint_shape(Vector<real_t> p_coordinates);
	void delete_constraint_shape(DDLSConstraintShape p_shape);
	DDLSConstraintSegment insert_constraint_segment(real_t x1, real_t  y1, real_t x2, real_t y2);
	void insert_new_constrained_edge(DDLSConstraintSegment p_from_segment, DDLSEdge p_edge_down_up, Vector<DDLSEdge> p_intersected_edges, Vector<DDLSEdge> p_left_bounding_edges, Vector<DDLSEdge> p_right_bounding_edges);
	void delete_constraint_segment(DDLSConstraintSegment p_segment);

	void check() const;

	DDLSVertex insert_vertex(real_t x, real_t y);

	DDLSEdge flip_edge(DDLSEdge edge);

	DDLSVertex split_edge(DDLSEdge p_edge, real_t x, real_t y);
	DDLSVertex split_face(DDLSFace p_face, real_t x, real_t y);

	void restore_as_delaunay();

	bool delete_vertex(DDLSVertex p_vertex);

	/// PRIVATE

	void untriangulate(Vector<DDLSEdge> p_edges_list);
	void triangulate(Vector<DDLSEdge> p_bound, bool p_is_real);

	void debug() const;

	DDLS_Mesh(real_t p_width, real_t p_height);
};
