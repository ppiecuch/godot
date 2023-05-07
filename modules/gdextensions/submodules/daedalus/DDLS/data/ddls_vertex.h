#include "core/color.h"
#include "core/vector.h"

#include "data/ddls_constraint_segment.h"

class DDLSEdge;

class DDLSVertex {
	static int COUNTER;
	int id;

	DDLSPoint2D pos;

	bool is_real = true;
	DDLSEdge *edge = nullptr;

	Vector<DDLSConstraintSegment> from_constraint_segments;

	Color color_debug;

public:
	int get_id() const { return id; }
	bool get_is_real() const { return is_real; }

	DDLSPoint2D get_pos() const { return pos; }

	Vector<DDLSConstraintSegment> get_from_constraint_segments() const { return _from_constraint_segments; }
	void set_from_constraint_segments(const Vector<DDLSConstraintSegment> &p_value) { from_constraint_segments = p_value; }
	void set_datas(const DDLSEdge &p_edge, bool p_is_real = true) {
		is_real = p_is_real;
		edge = p_edge;
	}

	void add_from_constraint_segment(const DDLSConstraintSegment &p_segment) {
		if (!_from_constraint_segments.has(p_segment)) {
			_from_constraint_segments.push_back(p_segment);
		}
	}
	void remove_from_constraint_segment(const DDLSConstraintSegment &p_segment) {
		from_constraint_segments.erase(p_segment);
	}

	DDLSEdge get_edge() const { return edge; }
	void set_edge(const DDLSEdge *p_edge) { edge = p_edge; }

	String to_string() const { return "ver_id " + itos(id); }

	DDLSVertex() : id(COUNTER++) { }
}
