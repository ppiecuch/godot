#include "core/color.h"

class DDLSEdge;

class DDLSFace {
	static int COUNTER;

	int id;
	bool is_real = true;
	DDLSEdge *edge = nullptr;

	Color color_debug;

public:
	int get_id() const { return id; }
	bool get_is_real() const { return is_real; }

	void set_datas(const DDLSEdge *p_edge, bool p_is_real = true) {
		is_real = p_is_real;
		edge = p_edge;
	}

	DDLSEdge *get_edge() const { return edge; }

	DDLSFace() : id(COUNTER++) { }
};
