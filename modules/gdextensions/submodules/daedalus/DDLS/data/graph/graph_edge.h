#pragma once

#include "data/data.h"

#include "core/variant.h"

class DDLS_GraphEdge{
	unsigned id;

	DDLSGraphEdge prev;
	DDLSGraphEdge next;

	DDLSGraphEdge rot_prev_edge;
	DDLSGraphEdge rot_next_edge;
	DDLSGraphEdge opposite_edge;
	DDLSGraphNode source_node;
	DDLSGraphNode destination_node;

	Variant data;

public:
	unsigned get_id() const { return id; }

	DDLSGraphEdge get_prev() const;
	void set_prev(DDLSGraphEdge p_value);
	DDLSGraphEdge get_next() const;
	void set_next(DDLSGraphEdge p_value);
	DDLSGraphEdge get_rot_prev_edge() const;
	void set_rot_prev_edge(DDLSGraphEdge p_value);
	DDLSGraphEdge get_rot_next_edge() const;
	void set_rot_next_edge(DDLSGraphEdge p_value);
	DDLSGraphEdge get_opposite_edge() const;
	void set_opposite_edge(DDLSGraphEdge p_value);
	DDLSGraphNode get_source_node const;
	void set_source_node(DDLSGraphNode p_value);
	DDLSGraphNode get_destination_node() const;
	void set_destination_node(DDLSGraphNode p_value);
	Variant get_data();
	void set_data(Variant p_value);

	DDLS_GraphEdge();
};
