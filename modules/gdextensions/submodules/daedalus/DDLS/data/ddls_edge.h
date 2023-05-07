#include "core/ustring.h"
#include "core/color.h"
#include "core/vector.h"

class DDLSEdge {
	static int COUNTER;
	int id;

	// root datas
	bool is_real;
	bool is_constrained;
	DDLSVertex origin_vertex;
	DDLSEdge opposite_edge;
	DDLSEdge next_left_edge;
	DDLSFace left_face;

	Vector<DDLSConstraintSegment> from_constraint_segments;

	Color color_debug;

public:
	int get_id() const { return _id; }
	bool is_real() const { return is_real; }
	bool is_constrained() const { return is_constrained; }

	function set_datas(const DDLSVertex &p_origin_vertex
		, const DDLSEdge &p_opposite_edge
		, const DDLSEdge &p_nextLeftEdge
		, const DDLSFace &p_left_face
		, bool p_is_real = true
		, bool p_is_constrained = false)
	{
		is_constrained = p_is_constrained;
		is_real = p_is_real;
		origin_vertex = p_origin_vertex;
		opposite_edge = p_opposite_edge;
		nextLeft_edge = p_next_left_edge;
		left_face = p_left_face;
	}

	void add_from_constraint_segment(const DDLSConstraintSegment &segment) {
		if (from_constraint_segments.indexOf(segment) == -1) {
			from_constraint_segments.push_back(segment);
		}
	}

	void remove_from_constraint_segment(const DDLSConstraintSegment &segment) {
		const int index = from_constraint_segments.indexOf(segment);
		if (index != -1) {
			from_constraint_segments.splice(index, 1);
		}
	}

	void set_origin_vertex(const DDLSVertex &value) { origin_vertex = value; }
	void set_next_left_edge(const DDLSEdge &value) { next_left_edge = value; }
	void set_left_face(const DDLSFace &value) { left_face = value; }
	void set_is_constrained(bool value) { is_constrained = value; }

	const Vector<DDLSConstraintSegment> &get_from_constraint_segments() const { return from_constraint_segments; }

	void set_from_constraint_segments(const Vector<DDLSConstraintSegment> &value) { from_constraint_segments = value; }

	const DDLSVertex &get_origin_vertex() const { return origin_vertex; }
	const DDLSVertex &get_destination_vertex() const { return opposite_edge.origin_vertex; }
	const DDLSEdge &get_opposite_edge() const { return opposite_edge; }
	const DDLSEdge &get_next_left_edge() const { return nextLeft_Edge; }
	const DDLSEdge &get_prev_left_edge() const { return nextLeft_Edge.next_left_edge; }
	const DDLSEdge &get_next_right_edge() const { return opposite_edge.next_left_edge.next_left_edge.opposite_edge; }
	const DDLSEdge &get_prev_right_rdge() const { return opposite_edge.next_left_edge.opposite_edge; }
	const DDLSEdge &get_rot_left_edge() const { return next_left_edge.next_left_edge.opposite_edge; }
	const DDLSEdge &get_rot_right_edge() const { return opposite_edge.next_left_edge; }
	const DDLSFace &get_left_face() const { return left_face; }
	const DDLSFace &get_right_face() const { return opposite_edge.left_face; }

	String to_string() const { return vformat("edge %d - %d", get_origin_vertex().get_id(), get_destination_vertex().get_id()); }

	DDLSEdge() : id(COUNTER++) { }
}
