#include "core/vector.h"

#include "data/ddls_constraint_segment.h"

class DDLSConstraintShape {
	static int COUNTER;

	int id;
	Vector<DDLSConstraintSegment> segments;

public:
	int get_id() const { return id; }
	Vector<DDLSConstraintSegment> get_segments() { return segments; }

	DDLSConstraintShape() id(COUNTER++) { }
};
