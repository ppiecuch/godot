#pragma once

#include "data/data.h"

#include "core/variant.h"

class DDLS_GraphNode {
	unsigned id;

	DDLSGraphNode prev;
	DDLSGraphNode next;

	DDLSGraphEdge outgoing_edge;
	Dictionary successor_nodes;

	Variant data;

public:
	unsigned get_id() const { return id; }

	DDLSGraphNode get prev() const { return prev; }
	void set_prev(DDLSGraphNode p_value) { prev = p_value; }
	DDLSGraphNode get_next() const { return next; }
	void set_next(DDLSGraphNode p_value) { next = p_value; }
	DDLSGraphEdge get_outgoing_edge() const { return outgoing_edge; }
	void set_outgoing_edge(DDLSGraphEdge p_value) { outgoing_edge = p_value; }
	Dictionary get_successor_nodes const { return successor_nodes; }
	void set_successor_nodes(Dictionary p_value) { successor_nodes = p_value; }
	Variant get data() const { return data; }
	void set_data(Variant value) { data = p_value; }

	DDLS_GraphNode();
};
