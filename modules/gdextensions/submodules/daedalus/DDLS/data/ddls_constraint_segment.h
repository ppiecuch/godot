#include "core/ustring.h"
#include "core/vector.h"

class DDLSConstraintShape;

class DDLSConstraintSegment {
	static int COUNTER;

	int id;
	Vector<DDLSEdge> edges;
	DDLSConstraintShape *from_shape = nullptr;

public:
	int get_id() const { return id; }

	DDLSConstraintShape *get_from_shape() const { return from_shape; }
	void set_from_shape(const DDLSConstraintShape *p_value) { from_shape = p_value; }

	Vector<DDLSEdge> get_edges() const { return edges; }

	void add_edge(const DDLSEdge &p_edge) {
		if (!edges.has(p_edge) &&  !edges.has(p_edge.opposite_edge)) {
			edges.push_back(p_edge);
		}
	}

	void remove_edge(const DDLSEdge &p_edge) {
		int index = edges.find(p_edge);
		if (index == -1) {
			index = edges.find(p_edge.opposite_edge);
		}
		if (index != -1) {
			edges.remove(index);
		}
	}

	String to_string() const { return "seg_id " + itos(id); }

	DDLSConstraintSegment() : id(COUNTER++) { }
};
